LOCAL_PATH := $(call my-dir)

# The shims live on /vendor and are added to the blobs' DT_NEEDED (see
# vendor/amazon/mt8127-common/patch-shims.sh), so they work without Lineage's
# LD_SHIM_LIBS, e.g. under a GSI. libgui is VNDK-private and libmedia is not
# VNDK, so those are left undefined here: the blobs that load a shim already
# depend on them (resolved through the VNDK-lite namespace). libsensor isn't
# reachable from a GSI's vendor namespace at all; libshim_sensor stubs it.

include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_binder.cpp
LOCAL_SHARED_LIBRARIES := libbinder
LOCAL_MODULE := libshim_libbinder
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_camera.cpp shim_omx.cpp
LOCAL_SHARED_LIBRARIES := libui liblog libcutils libbinder libutils libshim_sensor
LOCAL_HEADER_LIBRARIES := libhardware_headers
LOCAL_C_INCLUDES := frameworks/native/libs/sensor/include
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_MODULE := libshim_libcamera
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_liblog.c shim_atomic.c shim_thread.cpp shim_unicode.cpp shim_stopwatch.cpp
LOCAL_SHARED_LIBRARIES := liblog libutils
LOCAL_MODULE := libshim_liblog
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_gui.cpp
LOCAL_SHARED_LIBRARIES := libui libutils libbinder libhidlbase libhidltransport \
    android.hardware.graphics.bufferqueue@1.0 android.hardware.graphics.bufferqueue@2.0 \
    android.hidl.token@1.0-utils
LOCAL_HEADER_LIBRARIES := libgui_headers gl_headers libnativebase_headers libnativewindow_headers libbinder_headers
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_C_INCLUDES := frameworks/native/include $(LOCAL_PATH)/include
LOCAL_MODULE := libshim_gui
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_libwvm.cpp
LOCAL_MODULE := libshim_libwvm
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_32_BIT_ONLY := true
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_sensor.cpp
LOCAL_MODULE := libshim_sensor
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := shim_audio.cpp
LOCAL_SHARED_LIBRARIES := libdl
LOCAL_MODULE := libshim_audio
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)
