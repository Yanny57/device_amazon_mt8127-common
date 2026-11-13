#!/bin/bash
# Build the TWRP repartition zips for ford.
#   1. m ford_repart          (from the top of the source tree, after lunch)
#   2. device/amazon/mt8127-common/repartition/mkzips.sh [OUTDIR]
# Produces ford-repartition-treble-<date>.zip and ford-repartition-stock-<date>.zip.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
TOP=$(cd "$HERE/../../../.." && pwd)
OUT=${1:-$TOP/out/target/product/ford}
BIN=$(find "$TOP/out/target/product/ford" "$TOP/out/soong/.intermediates/device/amazon/mt8127-common/repartition" \
          -name ford_repart -type f -perm -u+x 2>/dev/null | head -1)
[ -n "$BIN" ] || { echo "ford_repart not built: run 'm ford_repart' first" >&2; exit 1; }
file "$BIN" | grep -q "ARM.*statically linked" || { echo "$BIN is not a static ARM binary" >&2; exit 1; }

DATE=$(date +%Y%m%d)
for target in treble stock; do
    W=$(mktemp -d)
    cp -r "$HERE/zip/META-INF" "$W/"
    cp "$BIN" "$W/ford_repart"
    echo "$target" > "$W/target"
    chmod 0755 "$W/META-INF/com/google/android/update-binary" "$W/ford_repart"
    Z="$OUT/ford-repartition-$target-$DATE.zip"
    rm -f "$Z"
    (cd "$W" && zip -qr9 "$Z" META-INF ford_repart target)
    rm -rf "$W"
    echo "$Z"
done
