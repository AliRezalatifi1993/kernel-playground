#!/usr/bin/env bash
set -euo pipefail

IFACE="${1:-eth1}"

SUDO=""
if [ "$EUID" -ne 0 ]; then
    SUDO="sudo"
fi

echo "[+] Detaching XDP program from $IFACE..."
$SUDO ip link set dev "$IFACE" xdp off

echo "[+] Detached."
