/*
 * Minimal copy of the header aidl generates from
 * frameworks/native/libs/gui/android/gui/DropInputMode.aidl. libgui keeps the
 * generated header private, but gui/LayerState.h includes it.
 */
#pragma once

#include <cstdint>

namespace android {
namespace gui {

enum class DropInputMode : int32_t {
    NONE = 0,
    ALL = 1,
};

}  // namespace gui
}  // namespace android
