DEVICE_COMMON := device/amazon/mt8127-common
VENDOR_COMMON := vendor/amazon/mt8127-common

# Device overlay
DEVICE_PACKAGE_OVERLAYS += $(DEVICE_COMMON)/overlay

# Install init.d scripts
PRODUCT_COPY_FILES += \
    $(DEVICE_COMMON)/configs/99exfat-support:system/etc/init.d/99exfat-support


$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# Device uses high-density artwork where available
PRODUCT_AAPT_CONFIG := normal mdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/tablet_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
	frameworks/native/data/etc/android.hardware.sensor.barometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.barometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml

# Init scripts, ueventd rules and fstab live on /vendor: on Q the root dir comes
# from system.img, which a GSI replaces
PRODUCT_COPY_FILES += \
    $(DEVICE_COMMON)/rootdir/init.mt8127.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.mt8127.rc \
    $(DEVICE_COMMON)/rootdir/init.mt8127.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.mt8127.usb.rc \
    $(DEVICE_COMMON)/rootdir/ueventd.mt8127.rc:$(TARGET_COPY_OUT_VENDOR)/ueventd.rc \
    $(DEVICE_COMMON)/rootdir/fstab.mt8127:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.mt8127

# Config files
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video_le.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    $(DEVICE_COMMON)/media/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    $(DEVICE_COMMON)/media/media_codecs_mediatek_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_mediatek_video.xml \
    $(DEVICE_COMMON)/media/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(DEVICE_COMMON)/media/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml \
    $(DEVICE_COMMON)/media/mtk_omx_core.cfg:$(TARGET_COPY_OUT_VENDOR)/etc/mtk_omx_core.cfg \
    $(DEVICE_COMMON)/seccomp/mediacodec-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy

# WiFi nvram
PRODUCT_COPY_FILES += \
    $(DEVICE_COMMON)/nvram/WIFI:$(TARGET_COPY_OUT_VENDOR)/etc/nvram/WIFI \
    $(DEVICE_COMMON)/nvram/WIFI_CUSTOM:$(TARGET_COPY_OUT_VENDOR)/etc/nvram/WIFI_CUSTOM

# HIDL wrappers for the legacy HALs (see manifest.xml)
PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-impl \
    android.hardware.audio.effect@2.0-impl \
    audio.primary.mt8127 \
    android.hardware.bluetooth@1.0-impl \
    android.hardware.bluetooth@1.0-service \
    android.hardware.camera.provider@2.4-impl \
    camera.device@1.0-impl \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.mapper@2.0-impl \
    android.hardware.keymaster@3.0-impl \
    android.hardware.light@2.0-impl \
    android.hardware.memtrack@1.0-impl \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service \
    android.hardware.power@1.0-impl \
    android.hardware.power@1.0-service \
    android.hardware.sensors@1.0-impl \

# ABI fixes for old binaries
PRODUCT_PACKAGES += \
	libshim_libbinder \
	libshim_liblog \
	libshim_libwvm \
    libshim_libcamera \
    libshim_gui \
    libshim_audio \
    libshim_sensor

# Mali EGL front (see egl/egl_wrapper.c)
PRODUCT_PACKAGES += \
    libEGL_mali

# Audio
PRODUCT_PACKAGES += \
	audio.a2dp.default \
    audio.r_submix.default \
    audio.usb.default \
    audio_policy.stub \
    libalsautils \
    libaudio-resampler \
    libtinyalsa \
    libtinycompress \
    libtinyxml
    
# Bluetooth
PRODUCT_PACKAGES += \
	bluetooth.default

# Lights
PRODUCT_PACKAGES += \
    lights.mt8127

# Graphics
PRODUCT_PACKAGES += \
    libion

# Power
PRODUCT_PACKAGES += \
	power.default

# network
PRODUCT_PACKAGES += \
    netd
    
# IPv6 tethering
PRODUCT_PACKAGES += \
    ebtables \
    ethertypes

# Wifi
PRODUCT_PACKAGES += \
    lib_driver_cmd_mt66xx \
    libwifi-hal-mt66xx \
    wifi_hal \
    libwpa_client \
    hostapd \
    wpa_supplicant \
    android.hardware.wifi@1.0-service

WITH_EXFAT := true

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0

# Android Go (low-RAM) profile for the 1 GB ford: ro.config.low_ram, lmk tuning,
# speed-profile system_server and a 128m/256m dalvik heap
$(call inherit-product, build/target/product/go_defaults.mk)

# call hwui memory config
$(call inherit-product-if-exists, frameworks/native/build/phone-xxhdpi-2048-hwui-memory.mk)

# Get non-open-source specific aspects
$(call inherit-product-if-exists, $(VENDOR_COMMON)/mt8127-common-vendor.mk)

# Audio policy (Q no longer reads the legacy audio_policy.conf)
PRODUCT_COPY_FILES += \
    $(DEVICE_COMMON)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml

# Q two-stage init: first-stage init only sees the boot ramdisk, so it needs
# its own copy of the fstab to mount /system and /vendor
PRODUCT_COPY_FILES += \
    $(DEVICE_COMMON)/rootdir/fstab.mt8127:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt8127

# Libraries the same-process graphics HALs need in the sphal namespace (see sphal/README)
PRODUCT_COPY_FILES += \
    $(foreach f,$(notdir $(wildcard $(DEVICE_COMMON)/sphal/*.so)),$(DEVICE_COMMON)/sphal/$(f):$(TARGET_COPY_OUT_VENDOR)/lib/hw/$(f))
PRODUCT_PACKAGES += libstdc++.vendor

# Non-graphics HALs run as vendor services: their legacy HALs need libs (libmedia,
# libsoftkeymasterdevice, VNDK-core interface libs) that the sphal namespace can't reach
PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-service.mt8127 \
    android.hardware.keymaster@3.0-service \
    android.hardware.camera.provider@2.4-service \
    android.hardware.light@2.0-service \
    android.hardware.memtrack@1.0-service \
    android.hardware.sensors@1.0-service

# The sensors HAL reads /dev/input, which the stock service rc doesn't grant
PRODUCT_COPY_FILES += \
    $(DEVICE_COMMON)/rootdir/sensors-override.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/sensors-override.rc
