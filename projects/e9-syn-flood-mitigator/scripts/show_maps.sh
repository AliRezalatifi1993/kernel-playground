#!/usr/bin/env bash
set -euo pipefail

SUDO=""
if [ "$EUID" -ne 0 ]; then
    SUDO="sudo"
fi

echo "========== Loaded BPF maps =========="
$SUDO bpftool map show | grep -E "syn_rate_map|stats_map" || true

echo
echo "========== Global stats map =========="
$SUDO bpftool map dump name stats_map || true

echo
echo "========== Per-source SYN rate map =========="
$SUDO bpftool map dump name syn_rate_map || true
