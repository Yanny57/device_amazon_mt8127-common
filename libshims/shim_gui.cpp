#include <stdint.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/SurfaceControl.h>

using android::PixelFormat;
using android::SurfaceComposerClient;
using android::SurfaceControl;
using android::String8;
using android::sp;

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
#pragma clang diagnostic pop
}
