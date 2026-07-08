# ╔════════════════════════════════════════╗
# ║ E9 — SYN Flood Mitigator with eBPF/XDP ║
# ╚════════════════════════════════════════╝

This project implements **E9 — SYN Flood Mitigator** for the Software Networks course.

The idea is simple: the router watches incoming TCP **SYN** packets. If one source sends too many SYN packets in the selected time window, the XDP program drops the extra packets before they reach the victim.

```text
Attacker-Host  ───▶  Mitigator-router  ───▶  Victim-Host
10.0.1.1              10.0.1.254 / 10.0.2.254    10.0.2.1
                         XDP attached on eth1
```

## What is implemented

| Level | Result |
|---|---|
| Basic | Counts incoming TCP SYN packets |
| Intermediate | Tracks SYN packets per source IP using a BPF map |
| Advanced | Drops SYN packets after the source exceeds the threshold |

The main code is in:

```text
xdp_syn_flood.bpf.c
```

The useful scripts are in:

```text
scripts/load.sh
scripts/unload.sh
scripts/show_maps.sh
scripts/test_syn_flood.sh
```

---

## 1. Build the XDP program

From the project folder:

```bash
cd ~/kernel-playground/projects/e9-syn-flood-mitigator
make clean
make
```

Expected output:

```text
xdp_syn_flood.o
```

---

## 2. Deploy the Containerlab testbed

The testbed is based on the course routing lab, renamed for this project:

```text
Attacker-Host
Mitigator-router
Victim-Host
```

Deploy it from the Containerlab folder:

```bash
cd ~/softnet-container-lab/containerlab/e9-syn-lab
chmod +x deploy.sh
./deploy.sh
```

Check containers:

```bash
sudo docker ps
```

Expected container names:

```text
clab-e9-syn-lab-Attacker-Host
clab-e9-syn-lab-Mitigator-router
clab-e9-syn-lab-Victim-Host
```

---

## 3. Check basic connectivity

Enter the attacker:

```bash
sudo docker exec -it clab-e9-syn-lab-Attacker-Host bash
ping 10.0.2.1
```

The ping should work before attaching XDP.

---

## 4. Copy the project into the mitigator

From the host VM:

```bash
sudo docker cp ~/kernel-playground/projects/e9-syn-flood-mitigator \
  clab-e9-syn-lab-Mitigator-router:/root/e9
```

Enter the mitigator:

```bash
sudo docker exec -it clab-e9-syn-lab-Mitigator-router bash
cd /root/e9
```

Compile again inside the container if needed:

```bash
make clean
make
```

---

## 5. Attach the XDP program

On the mitigator, find the interface facing the attacker:

```bash
ip addr
```

In this lab, the attacker-facing interface is usually `eth1` with IP `10.0.1.254`.

Attach the program:

```bash
cd /root/e9
./scripts/load.sh eth1
```

Check attachment:

```bash
ip -details link show dev eth1
```

---

## 6. Start the victim server

Open a new terminal and enter the victim:

```bash
sudo docker exec -it clab-e9-syn-lab-Victim-Host bash
apt update
apt install -y python3
python3 -m http.server 80
```

Leave this running.

---

## 7. Generate traffic from the attacker

Open another terminal and enter the attacker:

```bash
sudo docker exec -it clab-e9-syn-lab-Attacker-Host bash
apt update
apt install -y curl hping3
```

Normal traffic:

```bash
curl http://10.0.2.1
```

SYN flood test:

```bash
hping3 -S -c 300 -i u1000 -p 80 10.0.2.1
```

Expected behavior: only the allowed SYN packets pass; after the threshold, the mitigator starts dropping packets with `XDP_DROP`.

---

## 8. Check counters

Inside the mitigator:

```bash
cd /root/e9
./scripts/show_maps.sh
```

If `bpftool` inside the container has problems, check maps from the host VM:

```bash
sudo bpftool map show
sudo bpftool map dump name stats_map
sudo bpftool map dump name syn_rate_map
```

Example successful result:

```text
syn_count  = 300
drop_count = 250
```

This means the first 50 SYN packets were allowed and the remaining 250 were dropped.

---

## 9. Detach XDP

Inside the mitigator:

```bash
cd /root/e9
./scripts/unload.sh eth1
```

If needed, detach manually:

```bash
ip link set dev eth1 xdpgeneric off
```

---

## Final result

```text
Normal traffic passes.
SYN packets are counted per source IP.
When the attacker exceeds the threshold, extra SYN packets are dropped by XDP.
```

## Notes

- `SYN_THRESHOLD` controls how many SYN packets are allowed.
- `WINDOW_NS` controls the time window.
- For slower attacks, increase `WINDOW_NS`, for example from 1 second to 10 seconds.
- `xdpgeneric` is used because Containerlab interfaces are virtual Ethernet interfaces.


