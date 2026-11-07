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

# Treble: split sepolicy, VINTF, VNDK 29 vendor. The Nougat blobs still need
# non-VNDK system libs (libmedia, libstagefright, libcamera_client...), so the
# linker runs in VNDK-lite mode (ro.vndk.lite=true; a "vndklite" GSI)
PRODUCT_FULL_TREBLE_OVERRIDE := true
BOARD_VNDK_VERSION := current
BOARD_VNDK_RUNTIME_DISABLE := true
# Real /vendor partition (mmcblk0p14 on the "big" GPT, see ~/ClaudeCode/ford-gpt.sh)
TARGET_COPY_OUT_VENDOR := vendor
BOARD_VENDORIMAGE_PARTITION_SIZE := 268435456
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
# Device props go to /vendor/build.prop (a GSI replaces /system/build.prop)
BOARD_PROPERTY_OVERRIDES_SPLIT_ENABLED := true
TARGET_USES_64_BIT_BINDER := false

# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_COMMON)/manifest.xml

# Kernel Configuration
BOARD_KERNEL_IMAGE_NAME := zImage
BOARD_KERNEL_CMDLINE += androidboot.hardware=mt8127
# Q first-stage init only creates by-name links for the listed boot devices
BOARD_KERNEL_CMDLINE += androidboot.boot_devices=mtk-msdc.0
# TEMP DEBUG (17.1 bring-up): keep early init messages in dmesg
BOARD_KERNEL_CMDLINE += log_buf_len=1M
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
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648
BOARD_USERDATAIMAGE_PARTITION_SIZE := 5197381120
# Q's OTA generator sizes its stash from the cache partition (mmcblk0p12, 128 MiB)
BOARD_CACHEIMAGE_PARTITION_SIZE := 134217728
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 131072
TARGET_USERIMAGES_USE_EXT4 := true
BLOCK_BASED_OTA := true

# Vold
TARGET_USE_CUSTOM_LUN_FILE_PATH := /sys/devices/platform/mt_usb/musb-hdrc.0.auto/gadget/lun%d/file

# Shims: added to the blobs' DT_NEEDED by vendor/amazon/mt8127-common/patch-shims.sh

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
