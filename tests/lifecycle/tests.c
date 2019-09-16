/* Copyright 2014 Nodalink EURL
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

#include <glib.h>

#include <rte_config.h>
#include <rte_common.h>
#include <rte_eal.h>
#include <rte_ethdev.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <packetgraph/packetgraph.h>
#include <utils/mempool.h>
#include "utils/tests.h"
#include "tests.h"
#include <rte_eal.h>

static void print_usage(void)
{
	printf("tests usage: [EAL options] -- [-help]\n");
	exit(0);
}

static uint64_t parse_args(int argc, char **argv)
{
	int i;
	int ret = 0;

	for (i = 0; i < argc; ++i) {
		if (!strcmp("-help", argv[i])) {
			ret |= PRINT_USAGE;
		} else {
			printf("tests: invalid option -- %s\n", argv[i]);
			return FAIL | PRINT_USAGE;
		}
	}
	return ret;
}

int main(int argc, char **argv)
{
	int ret;
	uint64_t test_flags;
	/* Tests in the same order as the header function declarations. */
	g_test_init(&argc, &argv, NULL);

	/* Initialize packetgraph while testing the --no-huge case. */
	ret = pg_start_str("--no-huge");
	g_assert(ret >= 0);
	g_assert(!pg_init_seccomp());

	/* accounting program name */
	ret += + 1;
	argc -= ret;
	argv += ret;
	test_flags = parse_args(argc, argv);
	if (test_flags & PRINT_USAGE)
		print_usage();
	g_assert(!(test_flags & FAIL));

	pg_stop();
	/* Test pg_start_str with unparsable args.
	 * So we make an error in g_shell_parse_argv().*/
	g_assert(pg_start_str("\"") == -1);
	/* Test pg_start_str with too many args. */
	char buffer[500] = {0};

	for(int i = 0; i < 450; i+=3){
		buffer[i] = '-';
		buffer[i+1] = 'x';
		buffer[i+2] = ' ';
	}
	g_assert(pg_start_str(buffer) == -1);

	return g_test_run();
}
