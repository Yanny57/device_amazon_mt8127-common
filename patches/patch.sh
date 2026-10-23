ROOT="${PWD}"
DEVICE_DIR="device/amazon/mt8127-common"
REPOSITORIES=(
    'frameworks/av'
    'frameworks/base'
    'hardware/interfaces'
    'system/bt'
)

for repository in "${REPOSITORIES[@]}"; do
    cd "${ROOT}/${repository}"

    git am --whitespace=nowarn --keep-cr "${ROOT}/${DEVICE_DIR}/patches/${repository}"/*

    cd "${ROOT}"
done
