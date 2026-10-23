class SkColorTable;
#include <SkBitmap.h>
#include <SkCanvas.h>

extern "C"
{
    // Old blobs call SkBitmap::allocPixels(Allocator*, SkColorTable*), which Oreo's
    // Skia dropped. Member functions take `this` as the first argument.
    bool _ZN8SkBitmap11allocPixelsEPNS_9AllocatorEP12SkColorTable(SkBitmap* bitmap,
            SkBitmap::Allocator* allocator, SkColorTable* /* ctable */) {
        return bitmap->tryAllocPixels(allocator);
    }

    // SkCanvas::drawColor(SkColor, SkXfermode::Mode): SkBlendMode replaced
    // SkXfermode::Mode with the same enum values.
    void _ZN8SkCanvas9drawColorEjN10SkXfermode4ModeE(SkCanvas* canvas, SkColor color,
            int mode) {
        canvas->drawColor(color, static_cast<SkBlendMode>(mode));
    }
}
