#!/bin/sh

set -e

WORKSPACE_DIR=$1
JLINK_SERIAL_NUMBER=$2

. "$WORKSPACE_DIR/build/project_configuration.sh"

if [ -z "$JLINK_SERIAL_NUMBER" ]; then
    echo "Missing J-Link serial number. Set pico.jlinkSerialNumber in .vscode/settings.json." >&2
    exit 1
fi

case "${PICO_JLINK_CORE:-0}" in
    0|1)
        ;;
    *)
        echo "Unsupported PICO_JLINK_CORE: ${PICO_JLINK_CORE}. Use 0 or 1." >&2
        exit 1
        ;;
esac

case "$PICO_UPLOAD_TARGET" in
    rp2040)
        JLINK_DEVICE="RP2040_M0_${PICO_JLINK_CORE}"
        ;;
    rp2350)
        JLINK_DEVICE="RP2350_M33_${PICO_JLINK_CORE}"
        ;;
    *)
        echo "Unsupported PICO_UPLOAD_TARGET for J-Link: $PICO_UPLOAD_TARGET" >&2
        exit 1
        ;;
esac

SCRIPT=$(mktemp -t jlink-XXXXXX.jlink)
cleanup() {
    rm -f "$SCRIPT"
}
trap cleanup EXIT

cat >"$SCRIPT" <<EOF
r
loadfile "$WORKSPACE_DIR/build/$PICO_FIRMWARE_NAME.elf"
r
g
exit
EOF

echo "Using J-Link serial: $JLINK_SERIAL_NUMBER"
echo "Using J-Link device: $JLINK_DEVICE"
/usr/local/bin/JLinkExe \
    -device "$JLINK_DEVICE" \
    -if SWD \
    -speed 4000 \
    -SelectEmuBySN "$JLINK_SERIAL_NUMBER" \
    -CommanderScript "$SCRIPT"
