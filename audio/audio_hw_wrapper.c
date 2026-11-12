/*
 * Q-layout front for the Nougat MTK primary audio HAL (audio.primary.mt8127_vendor.so).
 *
 * Android P inserted get_microphones into struct audio_hw_device after
 * close_input_stream, so every hook from dump on is one slot off in the blob:
 * Q's get_master_mute lands on the blob's create_audio_patch and
 * set_audio_port_config reads past its struct. This module opens the blob and
 * hands out a device with the current layout that forwards to the blob's hooks.
 * Output streams are passed through as-is (the blob's struct only lacks fields
 * the 2.0 HIDL impl never calls for non-MMAP streams). Input streams are
 * wrapped: the blob's legacy audio_hw_hal predates get_capture_position, so
 * that slot holds its C++ AudioStreamIn pointer, and the HIDL ReadThread would
 * jump into the heap on the first read.
 *
 * Android P also added "flags" to struct audio_port_config, in front of ext.
 * The blob reads ext 4 bytes early (device type = module handle, mix handle =
 * module handle, ...), so it rejects every audio patch; a failed input patch
 * makes AudioFlinger fail startInput (-ENODATA) and no app can record. Port
 * configs are converted to the Nougat layout on the way in and out.
 */

#define LOG_TAG "audio_hw_wrapper"

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <log/log.h>

#define VENDOR_LIB "/vendor/lib/hw/audio.primary.mt8127_vendor.so"

/* struct audio_port_config as of Nougat: no flags before ext */
struct legacy_audio_port_config {
    audio_port_handle_t id;
    audio_port_role_t role;
    audio_port_type_t type;
    unsigned int config_mask;
    unsigned int sample_rate;
    audio_channel_mask_t channel_mask;
    audio_format_t format;
    struct audio_gain_config gain;
    union {
        struct audio_port_config_device_ext device;
        struct audio_port_config_mix_ext mix;
        struct audio_port_config_session_ext session;
    } ext;
};

/* struct audio_port as of Nougat: embeds the Nougat port config */
struct legacy_audio_port {
    audio_port_handle_t id;
    audio_port_role_t role;
    audio_port_type_t type;
    char name[AUDIO_PORT_MAX_NAME_LEN];
    unsigned int num_sample_rates;
    unsigned int sample_rates[AUDIO_PORT_MAX_SAMPLING_RATES];
    unsigned int num_channel_masks;
    audio_channel_mask_t channel_masks[AUDIO_PORT_MAX_CHANNEL_MASKS];
    unsigned int num_formats;
    audio_format_t formats[AUDIO_PORT_MAX_FORMATS];
    unsigned int num_gains;
    struct audio_gain gains[AUDIO_PORT_MAX_GAINS];
    struct legacy_audio_port_config active_config;
    union {
        struct audio_port_device_ext device;
        struct audio_port_mix_ext mix;
        struct audio_port_session_ext session;
    } ext;
};

#define CFG_HEAD offsetof(struct legacy_audio_port_config, ext)
#define PORT_HEAD offsetof(struct legacy_audio_port, active_config)

_Static_assert(CFG_HEAD == offsetof(struct audio_port_config, flags),
               "port config fields before flags must match");
_Static_assert(sizeof(((struct audio_port_config *)0)->ext) ==
                       sizeof(((struct legacy_audio_port_config *)0)->ext),
               "port config ext must match");
_Static_assert(PORT_HEAD == offsetof(struct audio_port, active_config),
               "port fields before active_config must match");
_Static_assert(sizeof(((struct audio_port *)0)->ext) ==
                       sizeof(((struct legacy_audio_port *)0)->ext),
               "port ext must match");

static void cfg_to_legacy(const struct audio_port_config *src,
                          struct legacy_audio_port_config *dst) {
    memcpy(dst, src, CFG_HEAD);
    memcpy(&dst->ext, &src->ext, sizeof(dst->ext));
}

static void cfg_from_legacy(const struct legacy_audio_port_config *src,
                            struct audio_port_config *dst) {
    memcpy(dst, src, CFG_HEAD);
    memset(&dst->flags, 0, sizeof(dst->flags));
    memcpy(&dst->ext, &src->ext, sizeof(dst->ext));
}

/* struct audio_hw_device as of Nougat: no get_microphones */
struct legacy_audio_hw_device {
    struct hw_device_t common;
    uint32_t (*get_supported_devices)(const struct legacy_audio_hw_device *dev);
    int (*init_check)(const struct legacy_audio_hw_device *dev);
    int (*set_voice_volume)(struct legacy_audio_hw_device *dev, float volume);
    int (*set_master_volume)(struct legacy_audio_hw_device *dev, float volume);
    int (*get_master_volume)(struct legacy_audio_hw_device *dev, float *volume);
    int (*set_mode)(struct legacy_audio_hw_device *dev, audio_mode_t mode);
    int (*set_mic_mute)(struct legacy_audio_hw_device *dev, bool state);
    int (*get_mic_mute)(const struct legacy_audio_hw_device *dev, bool *state);
    int (*set_parameters)(struct legacy_audio_hw_device *dev, const char *kv_pairs);
    char *(*get_parameters)(const struct legacy_audio_hw_device *dev, const char *keys);
    size_t (*get_input_buffer_size)(const struct legacy_audio_hw_device *dev,
                                    const struct audio_config *config);
    int (*open_output_stream)(struct legacy_audio_hw_device *dev, audio_io_handle_t handle,
                              audio_devices_t devices, audio_output_flags_t flags,
                              struct audio_config *config, struct audio_stream_out **stream_out,
                              const char *address);
    void (*close_output_stream)(struct legacy_audio_hw_device *dev,
                                struct audio_stream_out *stream_out);
    int (*open_input_stream)(struct legacy_audio_hw_device *dev, audio_io_handle_t handle,
                             audio_devices_t devices, struct audio_config *config,
                             struct audio_stream_in **stream_in, audio_input_flags_t flags,
                             const char *address, audio_source_t source);
    void (*close_input_stream)(struct legacy_audio_hw_device *dev,
                               struct audio_stream_in *stream_in);
    int (*dump)(const struct legacy_audio_hw_device *dev, int fd);
    int (*set_master_mute)(struct legacy_audio_hw_device *dev, bool mute);
    int (*get_master_mute)(struct legacy_audio_hw_device *dev, bool *mute);
    int (*create_audio_patch)(struct legacy_audio_hw_device *dev, unsigned int num_sources,
                              const struct legacy_audio_port_config *sources,
                              unsigned int num_sinks,
                              const struct legacy_audio_port_config *sinks,
                              audio_patch_handle_t *handle);
    int (*release_audio_patch)(struct legacy_audio_hw_device *dev,
                               audio_patch_handle_t handle);
    int (*get_audio_port)(struct legacy_audio_hw_device *dev, struct legacy_audio_port *port);
    int (*set_audio_port_config)(struct legacy_audio_hw_device *dev,
                                 const struct legacy_audio_port_config *config);
};

struct wrapper_device {
    struct audio_hw_device dev;
    struct legacy_audio_hw_device *legacy;
};

static void *vendor_handle;
static struct hw_module_t *vendor_module;

#define L(d) (((struct wrapper_device *)(d))->legacy)

/* Forward hook h if the blob set it, else fail like an unimplemented hook */
#define FWD(h, ...) (L(dev)->h ? L(dev)->h(L(dev), ##__VA_ARGS__) : -ENOSYS)

static uint32_t get_supported_devices(const struct audio_hw_device *dev) {
    return L(dev)->get_supported_devices ? L(dev)->get_supported_devices(L(dev)) : 0;
}
static int init_check(const struct audio_hw_device *dev) {
    return FWD(init_check);
}
static int set_voice_volume(struct audio_hw_device *dev, float volume) {
    return FWD(set_voice_volume, volume);
}
static int set_master_volume(struct audio_hw_device *dev, float volume) {
    return FWD(set_master_volume, volume);
}
static int get_master_volume(struct audio_hw_device *dev, float *volume) {
    return FWD(get_master_volume, volume);
}
static int set_mode(struct audio_hw_device *dev, audio_mode_t mode) {
    return FWD(set_mode, mode);
}
static int set_mic_mute(struct audio_hw_device *dev, bool state) {
    return FWD(set_mic_mute, state);
}
static int get_mic_mute(const struct audio_hw_device *dev, bool *state) {
    return FWD(get_mic_mute, state);
}
static int set_parameters(struct audio_hw_device *dev, const char *kv_pairs) {
    return FWD(set_parameters, kv_pairs);
}
static char *get_parameters(const struct audio_hw_device *dev, const char *keys) {
    return L(dev)->get_parameters ? L(dev)->get_parameters(L(dev), keys) : strdup("");
}
static size_t get_input_buffer_size(const struct audio_hw_device *dev,
                                    const struct audio_config *config) {
    return L(dev)->get_input_buffer_size ? L(dev)->get_input_buffer_size(L(dev), config) : 0;
}
static int open_output_stream(struct audio_hw_device *dev, audio_io_handle_t handle,
                              audio_devices_t devices, audio_output_flags_t flags,
                              struct audio_config *config, struct audio_stream_out **stream_out,
                              const char *address) {
    return FWD(open_output_stream, handle, devices, flags, config, stream_out, address);
}
static void close_output_stream(struct audio_hw_device *dev,
                                struct audio_stream_out *stream_out) {
    if (L(dev)->close_output_stream) L(dev)->close_output_stream(L(dev), stream_out);
}
/*
 * Input stream wrapper. Only the hooks up to get_input_frames_lost exist in the
 * blob's struct (same offsets as today); everything after it is left NULL.
 */
struct wrapper_stream_in {
    struct audio_stream_in stream;
    struct audio_stream_in *legacy;
};

#define LIN(s) (((struct wrapper_stream_in *)(s))->legacy)
#define LCOMMON(s) (&LIN(s)->common)

static uint32_t in_get_sample_rate(const struct audio_stream *s) {
    return LCOMMON(s)->get_sample_rate(LCOMMON(s));
}
static int in_set_sample_rate(struct audio_stream *s, uint32_t rate) {
    return LCOMMON(s)->set_sample_rate(LCOMMON(s), rate);
}
static size_t in_get_buffer_size(const struct audio_stream *s) {
    return LCOMMON(s)->get_buffer_size(LCOMMON(s));
}
static audio_channel_mask_t in_get_channels(const struct audio_stream *s) {
    return LCOMMON(s)->get_channels(LCOMMON(s));
}
static audio_format_t in_get_format(const struct audio_stream *s) {
    return LCOMMON(s)->get_format(LCOMMON(s));
}
static int in_set_format(struct audio_stream *s, audio_format_t format) {
    return LCOMMON(s)->set_format(LCOMMON(s), format);
}
static int in_standby(struct audio_stream *s) {
    return LCOMMON(s)->standby(LCOMMON(s));
}
static int in_dump(const struct audio_stream *s, int fd) {
    return LCOMMON(s)->dump(LCOMMON(s), fd);
}
static audio_devices_t in_get_device(const struct audio_stream *s) {
    return LCOMMON(s)->get_device(LCOMMON(s));
}
static int in_set_device(struct audio_stream *s, audio_devices_t device) {
    return LCOMMON(s)->set_device(LCOMMON(s), device);
}
static int in_set_parameters(struct audio_stream *s, const char *kv_pairs) {
    return LCOMMON(s)->set_parameters(LCOMMON(s), kv_pairs);
}
static char *in_get_parameters(const struct audio_stream *s, const char *keys) {
    return LCOMMON(s)->get_parameters(LCOMMON(s), keys);
}
static int in_add_audio_effect(const struct audio_stream *s, effect_handle_t effect) {
    return LCOMMON(s)->add_audio_effect(LCOMMON(s), effect);
}
static int in_remove_audio_effect(const struct audio_stream *s, effect_handle_t effect) {
    return LCOMMON(s)->remove_audio_effect(LCOMMON(s), effect);
}
static int in_set_gain(struct audio_stream_in *s, float gain) {
    return LIN(s)->set_gain(LIN(s), gain);
}
static ssize_t in_read(struct audio_stream_in *s, void *buffer, size_t bytes) {
    return LIN(s)->read(LIN(s), buffer, bytes);
}
static uint32_t in_get_input_frames_lost(struct audio_stream_in *s) {
    return LIN(s)->get_input_frames_lost(LIN(s));
}

static int open_input_stream(struct audio_hw_device *dev, audio_io_handle_t handle,
                             audio_devices_t devices, struct audio_config *config,
                             struct audio_stream_in **stream_in, audio_input_flags_t flags,
                             const char *address, audio_source_t source) {
    struct audio_stream_in *legacy = NULL;
    struct wrapper_stream_in *w;
    int ret;

    ret = FWD(open_input_stream, handle, devices, config, &legacy, flags, address, source);
    if (ret || !legacy) {
        *stream_in = NULL;
        return ret ? ret : -EINVAL;
    }
    w = calloc(1, sizeof(*w));
    if (!w) {
        L(dev)->close_input_stream(L(dev), legacy);
        *stream_in = NULL;
        return -ENOMEM;
    }
    w->legacy = legacy;
    w->stream.common.get_sample_rate = in_get_sample_rate;
    w->stream.common.set_sample_rate = in_set_sample_rate;
    w->stream.common.get_buffer_size = in_get_buffer_size;
    w->stream.common.get_channels = in_get_channels;
    w->stream.common.get_format = in_get_format;
    w->stream.common.set_format = in_set_format;
    w->stream.common.standby = in_standby;
    w->stream.common.dump = in_dump;
    w->stream.common.get_device = in_get_device;
    w->stream.common.set_device = in_set_device;
    w->stream.common.set_parameters = in_set_parameters;
    w->stream.common.get_parameters = in_get_parameters;
    w->stream.common.add_audio_effect = in_add_audio_effect;
    w->stream.common.remove_audio_effect = in_remove_audio_effect;
    w->stream.set_gain = in_set_gain;
    w->stream.read = in_read;
    w->stream.get_input_frames_lost = in_get_input_frames_lost;
    /* get_capture_position and all later hooks stay NULL (not supported) */
    *stream_in = &w->stream;
    return 0;
}
static void close_input_stream(struct audio_hw_device *dev, struct audio_stream_in *stream_in) {
    if (!stream_in) return;
    if (L(dev)->close_input_stream) L(dev)->close_input_stream(L(dev), LIN(stream_in));
    free(stream_in);
}
static int dump(const struct audio_hw_device *dev, int fd) {
    return FWD(dump, fd);
}
static int set_master_mute(struct audio_hw_device *dev, bool mute) {
    return FWD(set_master_mute, mute);
}
static int get_master_mute(struct audio_hw_device *dev, bool *mute) {
    return FWD(get_master_mute, mute);
}
static int create_audio_patch(struct audio_hw_device *dev, unsigned int num_sources,
                              const struct audio_port_config *sources, unsigned int num_sinks,
                              const struct audio_port_config *sinks,
                              audio_patch_handle_t *handle) {
    struct legacy_audio_port_config lsources[AUDIO_PATCH_PORTS_MAX];
    struct legacy_audio_port_config lsinks[AUDIO_PATCH_PORTS_MAX];
    unsigned int i;

    if (num_sources > AUDIO_PATCH_PORTS_MAX || num_sinks > AUDIO_PATCH_PORTS_MAX)
        return -EINVAL;
    for (i = 0; i < num_sources; i++) cfg_to_legacy(&sources[i], &lsources[i]);
    for (i = 0; i < num_sinks; i++) cfg_to_legacy(&sinks[i], &lsinks[i]);
    return FWD(create_audio_patch, num_sources, lsources, num_sinks, lsinks, handle);
}
static int release_audio_patch(struct audio_hw_device *dev, audio_patch_handle_t handle) {
    return FWD(release_audio_patch, handle);
}
static int get_audio_port(struct audio_hw_device *dev, struct audio_port *port) {
    struct legacy_audio_port lport;
    int ret;

    memcpy(&lport, port, PORT_HEAD);
    cfg_to_legacy(&port->active_config, &lport.active_config);
    memcpy(&lport.ext, &port->ext, sizeof(lport.ext));
    ret = FWD(get_audio_port, &lport);
    if (ret == 0) {
        memcpy(port, &lport, PORT_HEAD);
        cfg_from_legacy(&lport.active_config, &port->active_config);
        memcpy(&port->ext, &lport.ext, sizeof(port->ext));
    }
    return ret;
}
static int set_audio_port_config(struct audio_hw_device *dev,
                                 const struct audio_port_config *config) {
    struct legacy_audio_port_config lconfig;

    cfg_to_legacy(config, &lconfig);
    return FWD(set_audio_port_config, &lconfig);
}

static int wrapper_close(hw_device_t *device) {
    struct wrapper_device *w = (struct wrapper_device *)device;
    int ret = w->legacy->common.close(&w->legacy->common);
    free(w);
    return ret;
}

static int wrapper_open(const hw_module_t *module, const char *name, hw_device_t **device) {
    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0) return -EINVAL;

    if (!vendor_handle) {
        vendor_handle = dlopen(VENDOR_LIB, RTLD_NOW);
        if (!vendor_handle) {
            ALOGE("dlopen %s: %s", VENDOR_LIB, dlerror());
            return -EINVAL;
        }
        vendor_module = (struct hw_module_t *)dlsym(vendor_handle, HAL_MODULE_INFO_SYM_AS_STR);
        if (!vendor_module) {
            ALOGE("no %s in %s", HAL_MODULE_INFO_SYM_AS_STR, VENDOR_LIB);
            dlclose(vendor_handle);
            vendor_handle = NULL;
            return -EINVAL;
        }
    }

    struct legacy_audio_hw_device *legacy = NULL;
    int ret = vendor_module->methods->open(vendor_module, name, (hw_device_t **)&legacy);
    if (ret || !legacy) {
        ALOGE("vendor open failed: %d", ret);
        return ret ? ret : -EINVAL;
    }

    struct wrapper_device *w = calloc(1, sizeof(*w));
    if (!w) {
        legacy->common.close(&legacy->common);
        return -ENOMEM;
    }
    w->legacy = legacy;
    w->dev.common = legacy->common;
    w->dev.common.module = (struct hw_module_t *)module;
    w->dev.common.close = wrapper_close;
    w->dev.get_supported_devices = get_supported_devices;
    w->dev.init_check = init_check;
    w->dev.set_voice_volume = set_voice_volume;
    w->dev.set_master_volume = set_master_volume;
    w->dev.get_master_volume = get_master_volume;
    w->dev.set_mode = set_mode;
    w->dev.set_mic_mute = set_mic_mute;
    w->dev.get_mic_mute = get_mic_mute;
    w->dev.set_parameters = set_parameters;
    w->dev.get_parameters = get_parameters;
    w->dev.get_input_buffer_size = get_input_buffer_size;
    w->dev.open_output_stream = open_output_stream;
    w->dev.close_output_stream = close_output_stream;
    w->dev.open_input_stream = open_input_stream;
    w->dev.close_input_stream = close_input_stream;
    w->dev.get_microphones = NULL;
    w->dev.dump = dump;
    w->dev.set_master_mute = set_master_mute;
    w->dev.get_master_mute = get_master_mute;
    w->dev.create_audio_patch = create_audio_patch;
    w->dev.release_audio_patch = release_audio_patch;
    w->dev.get_audio_port = get_audio_port;
    w->dev.set_audio_port_config = set_audio_port_config;

    *device = &w->dev.common;
    return 0;
}

static struct hw_module_methods_t wrapper_methods = {
    .open = wrapper_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "MT8127 audio HAL wrapper",
        .author = "The LineageOS Project",
        .methods = &wrapper_methods,
    },
};
