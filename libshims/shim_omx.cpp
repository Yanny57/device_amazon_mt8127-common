#include <stdint.h>
#include <utils/Errors.h>

using namespace android;

// MtkOmxVdec asks Nougat's AwesomePlayer (removed in N) for the playback clock when the
// player registered itself with the codec. Nothing does that any more, so this is never
// reached; report "no clock" if it ever is.
extern "C" status_t _ZN7android13AwesomePlayer24mtk_omx_get_current_timeEPx(
        void* /* player */, int64_t* timeUs) {
    if (timeUs) *timeUs = -1;
    return INVALID_OPERATION;
}
