LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.audio@2.0-service.mt8127
LOCAL_INIT_RC := android.hardware.audio@2.0-service.mt8127.rc
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true
LOCAL_SRC_FILES := service.cpp
LOCAL_CFLAGS := -Wall -Werror
LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libhardware \
    android.hardware.audio@2.0 \
    android.hardware.audio.effect@2.0
LOCAL_MULTILIB := 32
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true
LOCAL_SRC_FILES := audio_hw_wrapper.c
LOCAL_SHARED_LIBRARIES := liblog libdl
LOCAL_HEADER_LIBRARIES := libhardware_headers libsystem_headers
LOCAL_CFLAGS := -Wall -Werror
LOCAL_MULTILIB := 32
include $(BUILD_SHARED_LIBRARY)
