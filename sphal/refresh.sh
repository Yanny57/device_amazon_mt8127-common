#!/bin/bash
# Refresh the sphal copies from the vendor variants Soong built in this tree (run from the tree top
# after a build). See README.
set -e
D=device/amazon/mt8127-common/sphal
V=android_vendor.30_arm_armv7-a-neon_cortex-a7_shared
for lib in android.hardware.graphics.allocator@2.0 android.hardware.graphics.allocator@3.0 \
           libbinder libfmq libui; do
# libnetutils (for libbwc): R builds no vendor variant; the Q copy only needs libcutils/liblog.
    src=$(find out/soong/.intermediates -path "*/$lib/$V/$lib.so" -not -path '*/unstripped/*' | head -1)
    [ -n "$src" ] || { echo "no vendor variant of $lib"; exit 1; }
    cp "$src" "$D/$lib.so"
    echo "$lib.so <- $src"
done
