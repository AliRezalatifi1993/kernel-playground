#!/usr/bin/env bash
set -euo pipefail

DST_IP="${1:-10.0.2.1}"
DST_PORT="${2:-80}"
COUNT="${3:-200}"

echo "[+] Target: $DST_IP:$DST_PORT"
echo "[+] Number of SYN packets: $COUNT"

if command -v hping3 >/dev/null 2>&1; then
    echo "[+] Using hping3 to generate SYN packets..."
    hping3 -S -c "$COUNT" -p "$DST_PORT" "$DST_IP"
else
    echo "[!] hping3 not found."
    echo "[!] Install it with: apt update && apt install -y hping3"
    exit 1
fi
