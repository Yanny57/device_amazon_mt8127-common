/*
 * Q-layout front for the Nougat MTK primary audio HAL (audio.primary.mt8127_vendor.so).
 *
 * Android P inserted get_microphones into struct audio_hw_device after
 * close_input_stream, so every hook from dump on is one slot off in the blob:
 * Q's get_master_mute lands on the blob's create_audio_patch and
 * set_audio_port_config reads past its struct. This module opens the blob and
 * hands out a device with the current layout that forwards to the blob's hooks.
 * Streams are passed through as-is (the blob's stream structs only lack fields
 * the 2.0 HIDL impl never calls for non-MMAP streams).
 */

#define LOG_TAG "audio_hw_wrapper"

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <log/log.h>

#define VENDOR_LIB "/vendor/lib/hw/audio.primary.mt8127_vendor.so"

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
                              const struct audio_port_config *sources, unsigned int num_sinks,
                              const struct audio_port_config *sinks,
                              audio_patch_handle_t *handle);
    int (*release_audio_patch)(struct legacy_audio_hw_device *dev,
                               audio_patch_handle_t handle);
    int (*get_audio_port)(struct legacy_audio_hw_device *dev, struct audio_port *port);
    int (*set_audio_port_config)(struct legacy_audio_hw_device *dev,
                                 const struct audio_port_config *config);
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
static int open_input_stream(struct audio_hw_device *dev, audio_io_handle_t handle,
                             audio_devices_t devices, struct audio_config *config,
                             struct audio_stream_in **stream_in, audio_input_flags_t flags,
                             const char *address, audio_source_t source) {
    return FWD(open_input_stream, handle, devices, config, stream_in, flags, address, source);
}
static void close_input_stream(struct audio_hw_device *dev, struct audio_stream_in *stream_in) {
    if (L(dev)->close_input_stream) L(dev)->close_input_stream(L(dev), stream_in);
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
    return FWD(create_audio_patch, num_sources, sources, num_sinks, sinks, handle);
}
static int release_audio_patch(struct audio_hw_device *dev, audio_patch_handle_t handle) {
    return FWD(release_audio_patch, handle);
}
static int get_audio_port(struct audio_hw_device *dev, struct audio_port *port) {
    return FWD(get_audio_port, port);
}
static int set_audio_port_config(struct audio_hw_device *dev,
                                 const struct audio_port_config *config) {
    return FWD(set_audio_port_config, config);
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
