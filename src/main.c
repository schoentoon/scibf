/*  Ventistipes
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "debug.h"
#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <event.h>

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "debug",      optional_argument, 0, 'D' },
  { "foreground", no_argument,       0, 'f' },
  { "config",     required_argument, 0, 'C' },
  { "test-config",required_argument, 0, 'T' },
  { 0, 0, 0, 0 }
};

struct event_base* event_base = NULL;

void onSignal(int signal) {
  event_base_free(event_base);
  exit(0);
};

static int usage() {
  fprintf(stderr, "USAGE: scibf [options]\n");
  fprintf(stderr, "-h, --help\t\tShow this help message.\n");
  fprintf(stderr, "-D, --debug\t\tIncrease debug level.\n");
  fprintf(stderr, "\t\t\tYou can also directly set a certain debug level with -D5\n");
  fprintf(stderr, "-f, --foreground\tDon't fork into the background (-D won't fork either).\n");
  fprintf(stderr, "-C, --config\t\tUse this config file.\n");
  fprintf(stderr, "-T, --test-config\tSimply test the config file for any errors.\n");
  return 0;
};

int main(int argc, char** argv) {
  int arg, optindex;
  unsigned char foreground = 0;
  while ((arg = getopt_long(argc, argv, "hD::fC:T:", g_LongOpts, &optindex)) != -1) {
    switch (arg) {
    case 'h':
      return usage();
    case 'D':
      if (optarg) {
        errno = 0;
        long tmp = strtol(optarg, NULL, 10);
        if (errno == 0 && tmp < 256)
          debug = (unsigned char) tmp;
        else
          fprintf(stderr, "%ld is an invalid debug level.\n", tmp);
      } else
        debug++;
      break;
    case 'f':
      foreground = 1;
      break;
    case 'C':
      if (parse_config(optarg) == 0)
        return 1;
      break;
    case 'T':
      if (parse_config(optarg)) {
        fprintf(stderr, "Config file seems to be valid.\n");
        return 0;
      } else {
        fprintf(stderr, "Your config seems to be invalid.\n");
        return 1;
      }
    }
  }
  if (foreground || debug || (fork() == 0)) {
    event_base = event_base_new();
    signal(SIGTERM, onSignal);
    signal(SIGSTOP, onSignal);
    while (1)
      event_base_dispatch(event_base);
  }
  return 0;
};