# E9 — SYN Flood Mitigator

This project implements an **eBPF/XDP SYN Flood Mitigator** for the Software Networks course.

The goal is to detect TCP SYN flood traffic early in the Linux networking stack and drop excessive SYN packets before they reach the victim host.

## Project idea

A SYN flood attack happens when an attacker sends many TCP SYN packets to a server but does not complete the TCP handshake. This can create many half-open connections and overload the victim.

This project uses **XDP** because it runs very early, before packets enter the normal Linux network stack. If the traffic is considered suspicious, the packet can be dropped immediately with `XDP_DROP`.

## Implemented levels

* **Basic:** Count incoming TCP SYN packets.
* **Intermediate:** Track the SYN rate per source IP using a BPF map.
* **Advanced:** Drop SYN packets from sources exceeding a threshold using `XDP_DROP`.

## How it works

The XDP program inspects each incoming packet:

1. Parse Ethernet header.
2. Check if the packet is IPv4.
3. Check if the packet is TCP.
4. Check if the TCP SYN flag is set.
5. Count SYN packets per source IP.
6. If the source exceeds the configured threshold, return `XDP_DROP`.
7. Otherwise, return `XDP_PASS`.

In simple form:


Packet arrives
     |
     v
XDP program checks TCP SYN
     |
     v
Count SYNs per source IP
     |
     v
Below threshold?  -> XDP_PASS
Above threshold?  -> XDP_DROP


## Project files


xdp_syn_flood.bpf.c     Main eBPF/XDP source code
Makefile                Build file for compiling the XDP program
containerlab/           Test topology with attacker, mitigator, and victim
scripts/                Helper scripts
docs/                   Extra documentation
README.md               Project documentation


## Build

From this folder:


make


This compiles:


xdp_syn_flood.bpf.c


and creates the XDP object file:


xdp_syn_flood.o


## Test environment

The project is tested using a 3-node Containerlab topology:


Attacker-Host <--> Mitigator-Router <--> Victim-Host


The XDP program is attached on the Mitigator-Router interface facing the Attacker-Host.

The detailed testbed instructions are inside:


containerlab/README.md

## Verification

The project can be verified using:


bpftool prog show
bpftool net show
bpftool map show
bpftool map dump
tcpdump
hping3


Expected behavior:

* Normal SYN packets are allowed.
* SYN packets are counted per source IP.
* When the attacker sends too many SYN packets, excessive packets are dropped.
* The victim receives fewer SYN packets after mitigation starts.

## Cleanup

To remove the Containerlab testbed:


cd containerlab
./destroy.sh


## Author

Alireza Latifi

