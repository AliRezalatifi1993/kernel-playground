# E9:SYN Flood Mitigator — Containerlab Testbed

## Goal:

This Containerlab topology is used to test the **E9 — SYN Flood Mitigator** eBPF/XDP project.

The topology contains three nodes:

Attacker-Host <--> Mitigator-Router <--> Victim-Host

## What we are testing

This testbed checks whether the **Mitigator-Router** can detect and reduce a TCP SYN flood attack using an eBPF/XDP program.


Attacker-Host ---> Mitigator-Router ---> Victim-Host


The Attacker-Host sends TCP SYN packets to the Victim-Host.
All packets pass through the Mitigator-Router, where the XDP program is attached.

We test three cases:

1. **Normal traffic**
   A small number of SYN packets should pass normally.

2. **SYN flood traffic**
   The attacker sends many SYN packets in a short time.
   The XDP program counts SYN packets per source IP using a BPF map.

3. **Mitigation**
   If one source sends more SYN packets than the threshold, the Mitigator-Router drops the extra packets with `XDP_DROP`.

## Expected result

Normal TCP SYN packets are allowed, but excessive SYN packets from the attacker are detected and dropped before reaching the Victim-Host.

## Verification tools

We verify the result using:

bpftool prog show
bpftool net show
bpftool map dump id <MAP_ID>
tcpdump
hping3


