# E9 — SYN Flood Mitigator

This project implements an eBPF/XDP SYN Flood Mitigator.

## Levels

- Basic: Count incoming TCP SYN packets.
- Intermediate: Track SYN rate per source IP using a BPF map.
- Advanced: Drop SYN packets from sources exceeding a threshold using XDP_DROP.

## Author

Alireza Latifi
