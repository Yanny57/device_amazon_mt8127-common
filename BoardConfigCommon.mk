DEVICE_COMMON := device/amazon/mt8127-common
KERNEL_COMMON := kernel/amazon/mt8127-common
VENDOR_COMMON := vendor/amazon/mt8127-common

# Headers
TARGET_SPECIFIC_HEADER_PATH := $(DEVICE_COMMON)/include

# Inherit from the proprietary version
-include $(VENDOR_COMMON)/BoardConfigVendor.mk

# Platform
TARGET_BOARD_PLATFORM := mt8127
TARGET_BOARD_PLATFORM_GPU := mali-450mp4
BOARD_HAS_MTK_HARDWARE := true
MTK_HARDWARE := true

# Architecture
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a7
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
ARCH_ARM_HAVE_TLS_REGISTER := true
ARCH_ARM_HAVE_NEON := true

# Non-Treble & Vendor Isolation Overrides (Required for 8.1 Bringup)
PRODUCT_FULL_TREBLE_OVERRIDE := false
TARGET_COPY_OUT_VENDOR := system/vendor
BOARD_PROPERTY_OVERRIDES_SPLIT_ENABLED := false
TARGET_USES_64_BIT_BINDER := false

# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_COMMON)/manifest.xml

# Kernel Configuration
BOARD_KERNEL_IMAGE_NAME := zImage
BOARD_KERNEL_CMDLINE += androidboot.hardware=mt8127
# Q first-stage init only creates by-name links for the listed boot devices
BOARD_KERNEL_CMDLINE += androidboot.boot_devices=mtk-msdc.0
BOARD_KERNEL_BASE := 0x80000000
BOARD_KERNEL_PAGESIZE := 2048
TARGET_KERNEL_SOURCE := $(KERNEL_COMMON)
BOARD_MKBOOTIMG_ARGS := --cmdline "$(BOARD_KERNEL_CMDLINE)" --base 0x80000000 --pagesize 2048 --kernel_offset 0x00008000 --ramdisk_offset 0x04000000 --second_offset 0x00f00000 --tags_offset 0x00000100

# Kernel Toolchain (must stay on GCC 4.8: GCC 4.9 kernels reset before ram console init)
TARGET_KERNEL_CROSS_COMPILE_PREFIX := arm-eabi-
KERNEL_TOOLCHAIN := $(shell pwd)/prebuilts/gcc/linux-x86/arm/$(TARGET_KERNEL_CROSS_COMPILE_PREFIX)4.8/bin

# Wi-Fi
BOARD_WLAN_DEVICE := MediaTek
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_mt66xx
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_mt66xx
WIFI_DRIVER_FW_PATH_PARAM := /dev/wmtWifi
WIFI_DRIVER_FW_PATH_AP := AP
WIFI_DRIVER_FW_PATH_STA := STA
WIFI_DRIVER_FW_PATH_P2P := P2P
WIFI_DRIVER_STATE_CTRL_PARAM := /dev/wmtWifi
WIFI_DRIVER_STATE_ON := 1
WIFI_DRIVER_STATE_OFF := 0

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_MTK := true
BOARD_BLUETOOTH_DOES_NOT_USE_RFKILL := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_COMMON)/bluetooth

# Graphics & SurfaceFlinger
BOARD_EGL_CFG := $(DEVICE_COMMON)/configs/egl.cfg
BOARD_EGL_NEEDS_HANDLE_VALUE := true
USE_OPENGL_RENDERER := true
TARGET_USES_OVERLAY := true
TARGET_USES_ION := true
TARGET_USES_HWC2 := false
BOARD_USES_LEGACY_MMAP := true
MAX_EGL_CACHE_KEY_SIZE := 12*1024
MAX_EGL_CACHE_SIZE := 1024*1024

TARGET_HAS_LEGACY_CAMERA_HAL1 := true
TARGET_USES_MEDIA_EXTENSIONS := true

TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS := true
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3

# Partitions & Filesystem
BOARD_BOOTIMAGE_PARTITION_SIZE := 16777216
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 16777216
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1258291200
BOARD_USERDATAIMAGE_PARTITION_SIZE := 6227082752
# Q's OTA generator sizes its stash from the cache partition (mmcblk0p12, 256000 KiB)
BOARD_CACHEIMAGE_PARTITION_SIZE := 262144000
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 131072
TARGET_USERIMAGES_USE_EXT4 := true
BLOCK_BASED_OTA := true

# Vold
TARGET_USE_CUSTOM_LUN_FILE_PATH := /sys/devices/platform/mt_usb/musb-hdrc.0.auto/gadget/lun%d/file

# Shims
TARGET_LD_SHIM_LIBS := \
    /system/lib/libbinder.so|libshim_libbinder.so \
    /system/lib/liblog.so|libshim_liblog.so \
    /system/lib/libcutils.so|libshim_liblog.so \
    /system/vendor/lib/libwvm.so|libshim_libwvm.so \
    /system/lib/libasp.so|libshim_libbinder.so \
    /system/lib/libcam_utils.so|libshim_libcamera.so \
    /system/lib/libMtkOmxVdec.so|libshim_libcamera.so \
    /system/lib/libMtkOmxVenc.so|libshim_libcamera.so \
    /system/lib/libcam.utils.sensorlistener.so|libshim_libcamera.so \
    /system/lib/libcam_platform.so|libshim_libcamera.so \
    /system/lib/libMali.so|libutilscallstack.so \
    /system/lib/libmtk_mali_user.so|libutilscallstack.so \
    /system/lib/hw/gralloc.mt8127.so|libutilscallstack.so \
    /system/lib/libcam_utils.so|libutilscallstack.so \
    /system/lib/libcam.utils.so|libutilscallstack.so \
    /system/lib/libaudiocomponentengine.so|libutilscallstack.so

# TWRP Recovery
ifneq (,$(strip $(wildcard bootable/recovery-twrp/twrp.cpp)))
RECOVERY_VARIANT := twrp
endif
DEVICE_RESOLUTION := 600x1024
TW_EXCLUDE_MTP := false
RECOVERY_SDCARD_ON_DATA := true
TW_BRIGHTNESS_PATH := /sys/devices/platform/leds-mt65xx/leds/lcd-backlight/brightness
TARGET_RECOVERY_LCD_BACKLIGHT_PATH := /sys/devices/platform/leds-mt65xx/leds/lcd-backlight/brightness
RECOVERY_GRAPHICS_USE_LINELENGTH := true
RECOVERY_FSTAB_VERSION := 2
TW_THEME := landscape_mdpi
RECOVERY_TOUCHSCREEN_SWAP_XY := true
RECOVERY_TOUCHSCREEN_FLIP_X := true
BOARD_HAS_FLIPPED_SCREEN := true

# SELinux / Seccomp
BOARD_SECCOMP_POLICY += $(DEVICE_COMMON)/seccomp
BOARD_SEPOLICY_DIRS += $(DEVICE_COMMON)/sepolicy

# Build guards
WITHOUT_CHECK_API := true

# Legacy MTK daemons violate many Q neverallows (userdebug only)
SELINUX_IGNORE_NEVERALLOWS := true

# Audio: Q only supports the XML audio policy configuration
USE_XML_AUDIO_POLICY_CONF := 1
