/*
 * ford_repart: switch the Amazon Fire 7 (2015, "ford") eMMC between Amazon's
 * stock GPT layout and the treble layout LineageOS 18.1 needs.
 *
 *   stock   p11 system 1200 MiB, p12 cache 250 MiB, p13 userdata (rest)
 *   treble  p11 system 2048 MiB, p14 vendor 256 MiB, p12 cache 128 MiB,
 *           p13 userdata (rest)
 *
 * Partitions p1-p10 (preloader data, LK, boot, recovery, TEE, ...) never move;
 * the tool refuses to touch a disk whose p1-p10 differ from ford's. Userdata
 * always runs to the end of the disk, so 8 GB and 16 GB units both work.
 * Only the GPT sectors are written: the primary table (LBA 1-33) and the
 * backup table (last 33 sectors). The protective MBR and disk GUID are kept.
 *
 * Changing the layout destroys cache and userdata (internal storage); the
 * TWRP zips format them afterwards.
 *
 * Usage: ford_repart [--device PATH] [--backup DIR] [--dry-run] status|treble|stock
 *
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/fs.h>
#endif

#define SECTOR 512u
#define GPT_ENTRIES 128u
#define GPT_ENTRY_SIZE 128u
#define GPT_ENTRY_SECTORS (GPT_ENTRIES * GPT_ENTRY_SIZE / SECTOR) /* 32 */
#define MIB (1024u * 1024u / SECTOR)                             /* sectors */

struct gpt_header {
    char signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable;
    uint64_t last_usable;
    uint8_t disk_guid[16];
    uint64_t entries_lba;
    uint32_t num_entries;
    uint32_t entry_size;
    uint32_t entries_crc;
} __attribute__((packed));

struct gpt_entry {
    uint8_t type_guid[16];
    uint8_t unique_guid[16];
    uint64_t first_lba;
    uint64_t last_lba;
    uint64_t attributes;
    uint16_t name[36];
} __attribute__((packed));

struct fixed_part {
    const char *name;
    uint64_t first, last;
};

/* ford's p1-p10, identical in both layouts */
static const struct fixed_part kFixed[10] = {
    {"KB", 2048, 4095},          {"DKB", 4096, 6143},         {"EXPDB", 6144, 41727},
    {"UBOOT", 41728, 43775},     {"boot", 43776, 76543},      {"recovery", 76544, 109311},
    {"MISC", 109312, 110335},    {"LOGO", 110336, 117503},    {"TEE1", 117504, 127743},
    {"TEE2", 127744, 137983},
};

#define SYSTEM_START 137984ull
#define STOCK_SYSTEM (1200ull * MIB)
#define STOCK_CACHE (250ull * MIB)
#define TREBLE_SYSTEM (2048ull * MIB)
#define TREBLE_VENDOR (256ull * MIB)
#define TREBLE_CACHE (128ull * MIB)

/* Linux filesystem data */
static const uint8_t kLinuxDataGuid[16] = {0xaf, 0x3d, 0xc6, 0x0f, 0x83, 0x84, 0x72, 0x47,
                                           0x8e, 0x79, 0x3d, 0x69, 0xd8, 0x47, 0x7d, 0xe4};

enum layout { LAYOUT_UNKNOWN, LAYOUT_STOCK, LAYOUT_TREBLE };
static const char *layout_name(enum layout l) {
    return l == LAYOUT_STOCK ? "stock" : l == LAYOUT_TREBLE ? "treble" : "unknown";
}

struct disk {
    int fd;
    const char *path;
    uint64_t sectors;
    uint8_t mbr[SECTOR];
    struct gpt_header hdr; /* primary */
    struct gpt_entry ent[GPT_ENTRIES];
};

static uint32_t crc32_buf(const void *buf, size_t len) {
    static uint32_t table[256];
    if (!table[1]) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
    }
    const uint8_t *p = buf;
    uint32_t c = 0xffffffffu;
    while (len--) c = table[(c ^ *p++) & 0xff] ^ (c >> 8);
    return c ^ 0xffffffffu;
}

static int read_at(struct disk *d, uint64_t lba, void *buf, size_t len) {
    ssize_t n = pread(d->fd, buf, len, (off_t)(lba * SECTOR));
    if (n != (ssize_t)len) {
        fprintf(stderr, "E: read at LBA %" PRIu64 " failed: %s\n", lba,
                n < 0 ? strerror(errno) : "short read");
        return -1;
    }
    return 0;
}

static int write_at(struct disk *d, uint64_t lba, const void *buf, size_t len) {
    ssize_t n = pwrite(d->fd, buf, len, (off_t)(lba * SECTOR));
    if (n != (ssize_t)len) {
        fprintf(stderr, "E: write at LBA %" PRIu64 " failed: %s\n", lba,
                n < 0 ? strerror(errno) : "short write");
        return -1;
    }
    return 0;
}

static void entry_name(const struct gpt_entry *e, char *out, size_t outlen) {
    size_t i;
    for (i = 0; i + 1 < outlen && i < 36 && e->name[i]; i++)
        out[i] = e->name[i] < 0x80 ? (char)e->name[i] : '?';
    out[i] = '\0';
}

static void set_entry_name(struct gpt_entry *e, const char *name) {
    memset(e->name, 0, sizeof(e->name));
    for (size_t i = 0; name[i] && i < 36; i++) e->name[i] = (uint8_t)name[i];
}

static int entry_empty(const struct gpt_entry *e) {
    static const uint8_t zero[16];
    return !memcmp(e->type_guid, zero, 16);
}

static int header_valid(const struct gpt_header *h, uint64_t lba) {
    struct gpt_header tmp = *h;
    if (memcmp(h->signature, "EFI PART", 8) || h->header_size != 92 || h->current_lba != lba)
        return 0;
    tmp.header_crc = 0;
    return crc32_buf(&tmp, 92) == h->header_crc;
}

static int load(struct disk *d) {
    struct stat st;
    if (fstat(d->fd, &st)) return -1;
#ifdef BLKGETSIZE64
    if (S_ISBLK(st.st_mode)) {
        uint64_t bytes;
        if (ioctl(d->fd, BLKGETSIZE64, &bytes)) {
            perror("E: BLKGETSIZE64");
            return -1;
        }
        d->sectors = bytes / SECTOR;
    } else
#endif
        d->sectors = (uint64_t)st.st_size / SECTOR;

    if (read_at(d, 0, d->mbr, SECTOR)) return -1;
    uint8_t sec[SECTOR];
    if (read_at(d, 1, sec, SECTOR)) return -1;
    memcpy(&d->hdr, sec, sizeof(d->hdr));
    if (!header_valid(&d->hdr, 1)) {
        fprintf(stderr, "E: primary GPT header missing or corrupt\n");
        return -1;
    }
    if (d->hdr.num_entries != GPT_ENTRIES || d->hdr.entry_size != GPT_ENTRY_SIZE ||
        d->hdr.entries_lba != 2) {
        fprintf(stderr, "E: unexpected GPT entry array (%u x %u at LBA %" PRIu64 ")\n",
                d->hdr.num_entries, d->hdr.entry_size, d->hdr.entries_lba);
        return -1;
    }
    if (read_at(d, 2, d->ent, sizeof(d->ent))) return -1;
    if (crc32_buf(d->ent, sizeof(d->ent)) != d->hdr.entries_crc) {
        fprintf(stderr, "E: primary GPT entry CRC mismatch\n");
        return -1;
    }
    if (d->hdr.backup_lba != d->sectors - 1) {
        fprintf(stderr, "E: backup GPT at LBA %" PRIu64 " but the disk has %" PRIu64
                        " sectors; refusing\n", d->hdr.backup_lba, d->sectors);
        return -1;
    }
    return 0;
}

/* Check p1-p10 and classify p11-p14 */
static enum layout classify(const struct disk *d, int verbose) {
    char name[40];
    for (int i = 0; i < 10; i++) {
        entry_name(&d->ent[i], name, sizeof(name));
        if (strcmp(name, kFixed[i].name) || d->ent[i].first_lba != kFixed[i].first ||
            d->ent[i].last_lba != kFixed[i].last) {
            if (verbose)
                fprintf(stderr, "E: p%d is '%s' %" PRIu64 "-%" PRIu64 ", expected ford's '%s'\n",
                        i + 1, name, d->ent[i].first_lba, d->ent[i].last_lba, kFixed[i].name);
            return LAYOUT_UNKNOWN;
        }
    }
    const struct gpt_entry *sys = &d->ent[10], *cache = &d->ent[11], *data = &d->ent[12],
                           *vnd = &d->ent[13];
    char n1[40], n2[40], n3[40], n4[40];
    entry_name(sys, n1, sizeof(n1));
    entry_name(cache, n2, sizeof(n2));
    entry_name(data, n3, sizeof(n3));
    entry_name(vnd, n4, sizeof(n4));
    if (strcmp(n1, "system") || strcmp(n2, "cache") || strcmp(n3, "userdata") ||
        sys->first_lba != SYSTEM_START || data->last_lba != d->hdr.last_usable) {
        if (verbose) fprintf(stderr, "E: p11-p13 are not system/cache/userdata as expected\n");
        return LAYOUT_UNKNOWN;
    }
    for (unsigned i = 14; i < GPT_ENTRIES; i++)
        if (!entry_empty(&d->ent[i])) {
            if (verbose) fprintf(stderr, "E: unexpected partition p%u\n", i + 1);
            return LAYOUT_UNKNOWN;
        }

    uint64_t s = SYSTEM_START;
    if (entry_empty(vnd) && sys->last_lba == s + STOCK_SYSTEM - 1 &&
        cache->first_lba == s + STOCK_SYSTEM &&
        cache->last_lba == s + STOCK_SYSTEM + STOCK_CACHE - 1 &&
        data->first_lba == s + STOCK_SYSTEM + STOCK_CACHE)
        return LAYOUT_STOCK;
    s += TREBLE_SYSTEM;
    if (!strcmp(n4, "vendor") && sys->last_lba == s - 1 && vnd->first_lba == s &&
        vnd->last_lba == s + TREBLE_VENDOR - 1 && cache->first_lba == s + TREBLE_VENDOR &&
        cache->last_lba == s + TREBLE_VENDOR + TREBLE_CACHE - 1 &&
        data->first_lba == s + TREBLE_VENDOR + TREBLE_CACHE)
        return LAYOUT_TREBLE;
    if (verbose) fprintf(stderr, "E: p11-p14 match neither the stock nor the treble layout\n");
    return LAYOUT_UNKNOWN;
}

static void print_table(const struct disk *d) {
    char name[40];
    printf("disk: %" PRIu64 " sectors (%.2f GiB)\n", d->sectors,
           (double)d->sectors * SECTOR / (1024.0 * 1024 * 1024));
    for (unsigned i = 10; i < 14; i++) {
        if (entry_empty(&d->ent[i])) continue;
        entry_name(&d->ent[i], name, sizeof(name));
        printf("  p%-2u %-9s %9" PRIu64 " - %9" PRIu64 "  %6" PRIu64 " MiB\n", i + 1, name,
               d->ent[i].first_lba, d->ent[i].last_lba,
               (d->ent[i].last_lba - d->ent[i].first_lba + 1) / MIB);
    }
}

static void random_guid(uint8_t g[16]) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0 || read(fd, g, 16) != 16) {
        /* fall back to something unique enough for a partition GUID */
        uint64_t t = (uint64_t)time(NULL) ^ ((uint64_t)getpid() << 32);
        memcpy(g, &t, 8);
        memcpy(g + 8, &t, 8);
    }
    if (fd >= 0) close(fd);
    g[7] = (g[7] & 0x0f) | 0x40; /* version 4 (mixed-endian field) */
    g[8] = (g[8] & 0x3f) | 0x80; /* RFC 4122 variant */
}

static void build_layout(struct disk *d, enum layout target) {
    struct gpt_entry *sys = &d->ent[10], *cache = &d->ent[11], *data = &d->ent[12],
                     *vnd = &d->ent[13];
    uint64_t s = SYSTEM_START;
    if (target == LAYOUT_STOCK) {
        sys->last_lba = s + STOCK_SYSTEM - 1;
        cache->first_lba = s + STOCK_SYSTEM;
        cache->last_lba = s + STOCK_SYSTEM + STOCK_CACHE - 1;
        data->first_lba = s + STOCK_SYSTEM + STOCK_CACHE;
        memset(vnd, 0, sizeof(*vnd));
    } else {
        sys->last_lba = s + TREBLE_SYSTEM - 1;
        if (entry_empty(vnd)) {
            memcpy(vnd->type_guid, kLinuxDataGuid, 16);
            random_guid(vnd->unique_guid);
            vnd->attributes = 0;
            set_entry_name(vnd, "vendor");
        }
        vnd->first_lba = s + TREBLE_SYSTEM;
        vnd->last_lba = s + TREBLE_SYSTEM + TREBLE_VENDOR - 1;
        cache->first_lba = s + TREBLE_SYSTEM + TREBLE_VENDOR;
        cache->last_lba = s + TREBLE_SYSTEM + TREBLE_VENDOR + TREBLE_CACHE - 1;
        data->first_lba = cache->last_lba + 1;
    }
    data->last_lba = d->hdr.last_usable;
}

static int save_backup(struct disk *d, const char *dir) {
    char path[512];
    uint8_t *buf = malloc(34 * SECTOR);
    int ret = -1;
    if (!buf) return -1;
    snprintf(path, sizeof(path), "%s/ford-gpt-%s-%" PRIu64 ".bin", dir,
             layout_name(classify(d, 0)), d->sectors);
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "W: cannot write backup %s: %s\n", path, strerror(errno));
        free(buf);
        return -1;
    }
    /* primary (LBA 0-33) followed by backup (last 33 sectors) */
    if (!read_at(d, 0, buf, 34 * SECTOR) && fwrite(buf, SECTOR, 34, f) == 34 &&
        !read_at(d, d->sectors - 33, buf, 33 * SECTOR) && fwrite(buf, SECTOR, 33, f) == 33)
        ret = 0;
    if (fclose(f)) ret = -1;
    free(buf);
    if (!ret) printf("saved current GPT to %s\n", path);
    return ret;
}

static int write_gpt(struct disk *d) {
    uint64_t last = d->sectors - 1;
    uint32_t ecrc = crc32_buf(d->ent, sizeof(d->ent));
    uint8_t sec[SECTOR];

    struct gpt_header prim = d->hdr;
    prim.entries_crc = ecrc;
    prim.header_crc = 0;
    prim.header_crc = crc32_buf(&prim, 92);

    struct gpt_header back = prim;
    back.current_lba = last;
    back.backup_lba = 1;
    back.entries_lba = last - GPT_ENTRY_SECTORS;
    back.header_crc = 0;
    back.header_crc = crc32_buf(&back, 92);

    /* backup first: if the primary write fails the old primary still boots */
    if (write_at(d, back.entries_lba, d->ent, sizeof(d->ent))) return -1;
    memset(sec, 0, sizeof(sec));
    memcpy(sec, &back, sizeof(back));
    if (write_at(d, last, sec, SECTOR)) return -1;
    if (fsync(d->fd)) perror("W: fsync");

    if (write_at(d, 2, d->ent, sizeof(d->ent))) return -1;
    memset(sec, 0, sizeof(sec));
    memcpy(sec, &prim, sizeof(prim));
    if (write_at(d, 1, sec, SECTOR)) return -1;
    if (fsync(d->fd)) {
        perror("E: fsync");
        return -1;
    }
    return 0;
}

static int verify_backup(struct disk *d) {
    uint8_t sec[SECTOR];
    struct gpt_header h;
    struct gpt_entry *ent = malloc(sizeof(d->ent));
    int ok = 0;
    if (!ent) return 0;
    if (!read_at(d, d->sectors - 1, sec, SECTOR)) {
        memcpy(&h, sec, sizeof(h));
        ok = header_valid(&h, d->sectors - 1) && h.entries_lba == d->sectors - 33 &&
             !read_at(d, h.entries_lba, ent, sizeof(d->ent)) &&
             crc32_buf(ent, sizeof(d->ent)) == h.entries_crc &&
             !memcmp(ent, d->ent, sizeof(d->ent));
    }
    free(ent);
    return ok;
}

static void usage(void) {
    fprintf(stderr,
            "usage: ford_repart [--device PATH] [--backup DIR] [--dry-run] status|treble|stock\n");
}

int main(int argc, char **argv) {
    const char *path = "/dev/block/mmcblk0", *backup_dir = NULL, *cmd = NULL;
    int dry_run = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--device") && i + 1 < argc) path = argv[++i];
        else if (!strcmp(argv[i], "--backup") && i + 1 < argc) backup_dir = argv[++i];
        else if (!strcmp(argv[i], "--dry-run")) dry_run = 1;
        else if (!cmd) cmd = argv[i];
        else {
            usage();
            return 2;
        }
    }
    enum layout target;
    if (!cmd) {
        usage();
        return 2;
    } else if (!strcmp(cmd, "status")) {
        target = LAYOUT_UNKNOWN;
        dry_run = 1;
    } else if (!strcmp(cmd, "treble")) {
        target = LAYOUT_TREBLE;
    } else if (!strcmp(cmd, "stock")) {
        target = LAYOUT_STOCK;
    } else {
        usage();
        return 2;
    }

    struct disk d = {.path = path};
    d.fd = open(path, dry_run ? O_RDONLY : O_RDWR);
    if (d.fd < 0) {
        fprintf(stderr, "E: open %s: %s\n", path, strerror(errno));
        return 2;
    }
    if (load(&d)) return 2;
    enum layout cur = classify(&d, 1);
    print_table(&d);
    printf("layout: %s\n", layout_name(cur));
    if (cur == LAYOUT_UNKNOWN) {
        fprintf(stderr, "E: not a known ford layout, nothing changed\n");
        return 1;
    }
    if (target == LAYOUT_UNKNOWN) return 0;
    if (cur == target) {
        printf("already %s, nothing to do\n", layout_name(target));
        return 0;
    }

    if (!dry_run && backup_dir) save_backup(&d, backup_dir);
    build_layout(&d, target);
    if (classify(&d, 1) != target) { /* sanity check of the new table */
        fprintf(stderr, "E: internal error building the %s layout\n", layout_name(target));
        return 2;
    }
    printf("new %s layout:\n", layout_name(target));
    print_table(&d);
    if (dry_run) {
        printf("dry run, nothing written\n");
        return 0;
    }
    if (write_gpt(&d)) {
        fprintf(stderr, "E: writing the GPT failed\n");
        return 2;
    }

    /* read it back from the disk */
    struct disk v = {.fd = d.fd, .path = path};
    if (load(&v) || classify(&v, 1) != target || !verify_backup(&v)) {
        fprintf(stderr, "E: verification of the new GPT failed\n");
        return 2;
    }
    printf("GPT written and verified: %s\n", layout_name(target));

#ifdef BLKRRPART
    struct stat st;
    if (!fstat(d.fd, &st) && S_ISBLK(st.st_mode)) {
        sync();
        if (ioctl(d.fd, BLKRRPART) == 0)
            printf("kernel re-read the partition table\n");
        else
            printf("reboot required: kernel could not re-read the table (%s)\n", strerror(errno));
    }
#endif
    close(d.fd);
    return 0;
}
