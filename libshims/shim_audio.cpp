/*
 * AudioSystem::getDeviceConnectionState for the MTK audio blob, which used to get it
 * through libmedia (Nougat) -> libaudioclient. libaudioclient is outside the VNDK, so a
 * GSI's vendor namespace can't load it: forward to it where the process can (Lineage's
 * VNDK-lite namespace) and report the device as unavailable otherwise.
 */

#include <dlfcn.h>

extern "C" int _ZN7android11AudioSystem24getDeviceConnectionStateEjPKc(unsigned int device,
                                                                    const char* address) {
    typedef int (*fn_t)(unsigned int, const char*);
    static fn_t real = [] {
        void* lib = dlopen("libaudioclient.so", RTLD_NOW);
        return lib ? reinterpret_cast<fn_t>(dlsym(lib,
                "_ZN7android11AudioSystem24getDeviceConnectionStateEjPKc")) : nullptr;
    }();
    return real ? real(device, address) : 0; // AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE
}
