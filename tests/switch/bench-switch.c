/* Copyright 2015 Outscale SAS
 *
 * This file is part of Packetgraph.
 *
 * Packetgraph is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Packetgraph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Packetgraph.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bench.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <packetgraph/packetgraph.h>
#include "utils/tests.h"
#include <packetgraph/switch.h>
#include "packets.h"
#include "brick-int.h"
#include "utils/bench.h"
#include "utils/mempool.h"
#include "utils/bitmask.h"

static void test_switch_benchmarks(int argc, char **argv, int west_max,
				     int east_max, const char *title)
{
	struct pg_error *error = NULL;
	struct pg_brick *sw;
	struct pg_bench bench;
	struct pg_bench_stats stats;
	struct ether_addr mac1 = {{0x52, 0x54, 0x00, 0x12, 0x34, 0x11} };
	struct ether_addr mac2 = {{0x52, 0x54, 0x00, 0x12, 0x34, 0x21} };
	uint32_t len;

	sw = pg_switch_new("switch", west_max, east_max,
			   PG_DEFAULT_SIDE, &error);
	g_assert(!error);
	g_assert(!pg_bench_init(&bench, title, argc, argv, &error));
	bench.input_brick = sw;
	bench.input_side = PG_WEST_SIDE;
	bench.output_brick = sw;
	bench.output_side = PG_EAST_SIDE;
	bench.output_poll = false;
	bench.max_burst_cnt = 1000000;
	bench.count_brick = NULL;
	bench.pkts_nb = 64;
	bench.pkts_mask = pg_mask_firsts(64);
	bench.pkts = pg_packets_create(bench.pkts_mask);
	bench.pkts = pg_packets_append_ether(
		bench.pkts,
		bench.pkts_mask,
		&mac1, &mac2,
		ETHER_TYPE_IPv4);
	bench.brick_full_burst = 1;
	len = sizeof(struct ipv4_hdr) + sizeof(struct udp_hdr) + 1400;
	pg_packets_append_ipv4(
		bench.pkts,
		bench.pkts_mask,
		0x000000EE, 0x000000CC, len, 17);
	bench.pkts = pg_packets_append_udp(
		bench.pkts,
		bench.pkts_mask,
		1000, 2000, 1400);
	bench.pkts = pg_packets_append_blank(bench.pkts, bench.pkts_mask, 1400);

	g_assert(pg_bench_run(&bench, &stats, &error) == 0);
	pg_bench_print(&stats);

	pg_packets_free(bench.pkts, bench.pkts_mask);
	pg_brick_destroy(sw);
	g_free(bench.pkts);
}
void test_benchmark_switch(int argc, char **argv)
{
	test_switch_benchmarks(argc, argv, 20, 20,
			       "switch : 20 edges at each sides");
	test_switch_benchmarks(argc, argv, 10000, 20,
			       "switch : 10000 edge at WEST and 20 at EAST");
	test_switch_benchmarks(argc, argv, 20, 10000,
			       "switch : 20 edge at WEST and 10000 at EAST");
	test_switch_benchmarks(argc, argv, 10000, 10000,
			       "switch : 10000 edges at each sides");
}
