#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <utils/Log.h>
#include <hardware/power.h>
#include <hardware/hardware.h>


#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
#include <string>
#include <sensor/SensorEventQueue.h>
#include <sensor/SensorManager.h>
#include <utils/String16.h>
#include <binder/IServiceManager.h>

using namespace android;

extern "C" void _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
    void* thiz, uint32_t inWidth, uint32_t inHeight, int inFormat, uint32_t inUsage,
    std::string requestorName);

extern "C" void _ZN7android13GraphicBufferC1Ejjij(
    void* thiz, uint32_t inWidth, uint32_t inHeight, int inFormat, uint32_t inUsage) {
  std::string requestorName = "<Unknown>";
  _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
      thiz, inWidth, inHeight, inFormat, inUsage, requestorName);
}


// GraphicBufferMapper::lock(handle, int usage, ...) from Nougat
extern "C" status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handleiRKNS_4RectEPPv(
	GraphicBufferMapper* thiz, buffer_handle_t handle, int32_t usage, const Rect& bounds,
	void** vaddr) {
	return thiz->lock(handle, static_cast<uint32_t>(usage), bounds, vaddr);
}

// GraphicBuffer::lock(usage, vaddr) from Nougat; Q added the bytes-per-pixel/stride outputs
extern "C" status_t _ZN7android13GraphicBuffer4lockEjPPv(
	GraphicBuffer* thiz, uint32_t inUsage, void** vaddr) {
	return thiz->lock(inUsage, vaddr);
}

// Nougat camera blobs use the Singleton<SensorManager> that Oreo removed (SensorManager is
// now per package and larger). Pre-fill the inlined singleton with a real Oreo instance so
// the blobs never construct a Nougat-sized one themselves.
extern "C" pthread_mutex_t _ZN7android9SingletonINS_13SensorManagerEE5sLockE = PTHREAD_MUTEX_INITIALIZER;
extern "C" SensorManager* _ZN7android9SingletonINS_13SensorManagerEE9sInstanceE = NULL;

__attribute__((constructor)) static void shim_sensor_manager_init() {
    // This shim is also loaded by media.codec (MTK OMX), which can't reach sensorservice:
    // only fill the singleton where the service is up, instead of blocking on it.
    if (defaultServiceManager()->checkService(String16("sensorservice")) == NULL) return;
    _ZN7android9SingletonINS_13SensorManagerEE9sInstanceE =
            &SensorManager::getInstanceForPackage(String16("camera"));
}

extern "C" void _ZN7android13SensorManagerC1Ev(void* /* sensorMgr */) {
    LOG_ALWAYS_FATAL("SensorManager() is not supported, the singleton must be pre-filled");
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C" sp<SensorEventQueue> _ZN7android13SensorManager16createEventQueueEv(SensorManager* sensorMgr) {
    return sensorMgr->createEventQueue();
}
#pragma clang diagnostic pop
