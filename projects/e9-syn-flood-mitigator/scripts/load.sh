#!/usr/bin/env bash
set -euo pipefail

IFACE="${1:-eth1}"

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_DIR"

SUDO=""
if [ "$EUID" -ne 0 ]; then
    SUDO="sudo"
fi

echo "[+] Building XDP program..."
make

echo "[+] Detaching old XDP program from $IFACE if present..."
$SUDO ip link set dev "$IFACE" xdp off 2>/dev/null || true

echo "[+] Attaching E9 SYN Flood Mitigator to $IFACE using xdpgeneric..."
$SUDO ip link set dev "$IFACE" xdpgeneric obj xdp_syn_flood.o sec xdp

echo "[+] Attached successfully."
echo
$SUDO bpftool net show
