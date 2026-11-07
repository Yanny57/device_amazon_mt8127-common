#include <stdint.h>
#include <unistd.h>
#include <gui/BufferQueue.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/SurfaceControl.h>
#include <ui/Fence.h>

using android::PixelFormat;
using android::SurfaceComposerClient;
using android::SurfaceControl;
using android::String8;
using android::sp;
using android::BufferQueue;
using android::Fence;
using android::IBinder;
using android::IGraphicBufferConsumer;
using android::IGraphicBufferProducer;

/* Fence is LightRefBase (int32 refcount) followed by the fence fd. */
static_assert(sizeof(Fence) == 2 * sizeof(int32_t), "unexpected android::Fence layout");

extern "C"
{
    /* android::SurfaceControl:setLayer*/
    void _ZN7android14SurfaceControl8setLayerEj(uint32_t) {}
    void _ZN7android14SurfaceControl8setLayerEi(int32_t layer) {
        _ZN7android14SurfaceControl8setLayerEj(static_cast<uint32_t>(layer));
    }

    /* Nougat SurfaceComposerClient::createSurface(name, w, h, format, flags);
     * Oreo added parent, windowType and ownerUid (all defaulted). */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
    sp<SurfaceControl> _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8Ejjij(
            SurfaceComposerClient* client, const String8& name, uint32_t w, uint32_t h,
            PixelFormat format, uint32_t flags) {
        return client->createSurface(name, w, h, format, flags);
    }

    /* Nougat SurfaceComposerClient::getBuiltInDisplay(id); Q uses display tokens. */
    sp<IBinder> _ZN7android21SurfaceComposerClient17getBuiltInDisplayEi(int32_t id) {
        return id == 0 ? SurfaceComposerClient::getInternalDisplayToken() : nullptr;
    }
#pragma clang diagnostic pop

    /* Fence::~Fence() is inline in Q, closes the fd held after the refcount. */
    void _ZN7android5FenceD1Ev(Fence* fence) {
        int32_t* fields = reinterpret_cast<int32_t*>(fence);
        if (fields[1] >= 0) {
            close(fields[1]);
        }
    }

    /* Nougat BufferQueue::createBufferQueue(producer, consumer, allocator);
     * IGraphicBufferAlloc is gone, buffers are allocated through gralloc. */
    void _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(
            sp<IGraphicBufferProducer>* outProducer, sp<IGraphicBufferConsumer>* outConsumer,
            const void* /* allocator */) {
        BufferQueue::createBufferQueue(outProducer, outConsumer);
    }
}
