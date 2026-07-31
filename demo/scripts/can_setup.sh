#!/bin/sh
# can_setup.sh — bring up the CAN interface and smoke-test it with can-utils.
# IE Pro 400 GlobalStandard: interface can0, default bitrate 250000.

set -e

IFACE="${1:-can0}"
BITRATE="${2:-250000}"

ip link set "$IFACE" down 2>/dev/null || true
ip link set "$IFACE" type can bitrate "$BITRATE"
ip link set "$IFACE" up

echo "Interface $IFACE is up at ${BITRATE} bps."
echo "Monitor traffic with:  candump $IFACE"
echo "Send a test frame with: cansend $IFACE 123#1122334455667788"
