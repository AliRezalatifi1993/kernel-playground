// SPDX-License-Identifier: GPL-2.0
// E9 — SYN Flood Mitigator
// Basic: count incoming TCP SYN packets
// Intermediate: track SYN rate per source IP using a BPF map
// Advanced: drop SYN packets from sources exceeding a threshold

#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#define WINDOW_NS 1000000000ULL   // 1 second
#define SYN_THRESHOLD 50          // max SYN packets per second per source IP

#define STAT_TOTAL_SYN 0
#define STAT_PASSED_SYN 1
#define STAT_DROPPED_SYN 2
#define STAT_OTHER_PACKETS 3

struct syn_state {
    __u64 window_start_ns;
    __u64 syn_count;
    __u64 drop_count;
};

/*
 * Per-source SYN rate map.
 * Key   = IPv4 source address
 * Value = current time window, SYN count, drop count
 */
struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __uint(max_entries, 1024);
    __type(key, __u32);
    __type(value, struct syn_state);
} syn_rate_map SEC(".maps");

/*
 * Global statistics map.
 * Per-CPU array avoids locking for counters.
 */
struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(max_entries, 4);
    __type(key, __u32);
    __type(value, __u64);
} stats_map SEC(".maps");

static __always_inline void increase_stat(__u32 index)
{
    __u64 *value;

    value = bpf_map_lookup_elem(&stats_map, &index);
    if (value)
        (*value)++;
}

SEC("xdp")
int xdp_syn_flood_mitigator(struct xdp_md *ctx)
{
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = data;

    // Boundary check for Ethernet header
    if ((void *)(eth + 1) > data_end)
        return XDP_PASS;

    // Only handle IPv4 packets
    if (eth->h_proto != bpf_htons(ETH_P_IP)) {
        increase_stat(STAT_OTHER_PACKETS);
        return XDP_PASS;
    }

    struct iphdr *ip = (void *)(eth + 1);

    // Boundary check for IPv4 header
    if ((void *)(ip + 1) > data_end)
        return XDP_PASS;

    // Only handle TCP packets
    if (ip->protocol != IPPROTO_TCP) {
        increase_stat(STAT_OTHER_PACKETS);
        return XDP_PASS;
    }

    // IPv4 header length may be more than 20 bytes if options exist
    __u32 ip_header_len = ip->ihl * 4;

    if (ip_header_len < sizeof(*ip))
        return XDP_PASS;

    if ((void *)ip + ip_header_len > data_end)
        return XDP_PASS;

    struct tcphdr *tcp = (void *)ip + ip_header_len;

    // Boundary check for TCP header
    if ((void *)(tcp + 1) > data_end)
        return XDP_PASS;

    /*
     * TCP flags are located at byte 13 of the TCP header.
     * SYN = 0x02
     * ACK = 0x10
     *
     * We count only initial SYN packets:
     * SYN = 1 and ACK = 0
     */
    __u8 tcp_flags = *((__u8 *)tcp + 13);

    if ((tcp_flags & 0x02) && !(tcp_flags & 0x10)) {
        increase_stat(STAT_TOTAL_SYN);

        __u32 src_ip = ip->saddr;
        __u64 now = bpf_ktime_get_ns();

        struct syn_state *state;
        state = bpf_map_lookup_elem(&syn_rate_map, &src_ip);

        if (!state) {
            struct syn_state new_state = {
                .window_start_ns = now,
                .syn_count = 1,
                .drop_count = 0,
            };

            bpf_map_update_elem(&syn_rate_map, &src_ip, &new_state, BPF_ANY);
            increase_stat(STAT_PASSED_SYN);
            return XDP_PASS;
        }

        // If the 1-second window expired, reset the counter
        if (now - state->window_start_ns > WINDOW_NS) {
            state->window_start_ns = now;
            state->syn_count = 1;

            increase_stat(STAT_PASSED_SYN);
            return XDP_PASS;
        }

        // Same time window: increase SYN counter
        state->syn_count++;

        // Advanced level: drop if threshold exceeded
        if (state->syn_count > SYN_THRESHOLD) {
            state->drop_count++;
            increase_stat(STAT_DROPPED_SYN);

            // Print only the first drop after threshold to avoid log flooding
            if (state->syn_count == SYN_THRESHOLD + 1)
                bpf_printk("E9 SYN flood detected: dropping source IP");

            return XDP_DROP;
        }

        increase_stat(STAT_PASSED_SYN);
        return XDP_PASS;
    }

    increase_stat(STAT_OTHER_PACKETS);
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
