#!/bin/bash
# Apply the mt8127 platform patches for LineageOS 18.1.
# Run from the top of the source tree after `repo sync`.
ROOT="${PWD}"
DEVICE_DIR="device/amazon/mt8127-common"
REPOSITORIES=(
    'frameworks/av'
    'hardware/interfaces'
    'packages/apps/AudioFX'
    'packages/modules/NetworkStack'
    'system/bt'
    'system/keymaster'
    'system/libhidl'
)

for repository in "${REPOSITORIES[@]}"; do
    cd "${ROOT}/${repository}" || exit 1

    git am --whitespace=nowarn --keep-cr "${ROOT}/${DEVICE_DIR}/patches/${repository}"/*.patch || {
        echo "Failed to apply patches to ${repository}" >&2
        exit 1
    }

    cd "${ROOT}"
done
