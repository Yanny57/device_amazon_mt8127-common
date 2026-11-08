/*
 * Stand-in for the libsensor calls the MTK camera blobs make (sensorlistener and
 * libshim_libcamera's SensorManager singleton). libsensor is a system library outside
 * the VNDK, so a GSI's vendor namespace can't load it. Report no sensors instead:
 * the camera's sensor listener then has nothing to enable.
 */

#include <stdint.h>
#include <sys/types.h>

// Only ever handed back to the blobs as an opaque SensorManager&
static uint64_t sDummySensorManager[64];

extern "C" {

// SensorManager& SensorManager::getInstanceForPackage(const String16&)
void* _ZN7android13SensorManager21getInstanceForPackageERKNS_8String16E(const void*) {
    return sDummySensorManager;
}

// sp<SensorEventQueue> SensorManager::createEventQueue(String8, int): returned through
// the result pointer; a null sp
void _ZN7android13SensorManager16createEventQueueENS_7String8Ei(void** result, void*, void*,
                                                                 int) {
    *result = nullptr;
}

// Sensor const* SensorManager::getDefaultSensor(int)
const void* _ZN7android13SensorManager16getDefaultSensorEi(void*, int) {
    return nullptr;
}

// SensorEventQueue: only reachable with a queue, which is never handed out
ssize_t _ZN7android16SensorEventQueue4readEP12ASensorEventj(void*, void*, size_t) {
    return 0;
}
int _ZNK7android16SensorEventQueue12enableSensorEPKNS_6SensorE(const void*, const void*) {
    return -19; // NO_INIT
}
int _ZNK7android16SensorEventQueue13disableSensorEPKNS_6SensorE(const void*, const void*) {
    return -19;
}
int _ZNK7android16SensorEventQueue12setEventRateEPKNS_6SensorEx(const void*, const void*,
                                                                 int64_t) {
    return -19;
}
int _ZNK7android16SensorEventQueue5getFdEv(const void*) {
    return -1;
}

}
