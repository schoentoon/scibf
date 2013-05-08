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

#include "config.h"

#include "debug.h"
#include "irc_callbacks.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

struct config* global_config = NULL;

struct evdns_base* dns = NULL;

struct server* getServer(char* name);

int parse_config(char* config_file) {
  if (global_config)
    return 0;
  FILE *f = fopen(config_file, "r");
  if (f == NULL) {
    fprintf(stderr, "Error '%s' while opening '%s'.\n", strerror(errno), config_file);
    return 0;
  }
  global_config = malloc(sizeof(struct config));
  struct server* current_server = NULL;
  char line_buffer[BUFSIZ];
  unsigned int line_count = 0;
  while (fgets(line_buffer, sizeof(line_buffer), f)) {
    line_count++;
    if (line_buffer[0] == '#')
      continue;
    char key[BUFSIZ];
    char value[BUFSIZ];
    if (sscanf(line_buffer, "%[a-z_] = %[^\t\n]", key, value) == 2) {
      DEBUG(255, "key: '%s', value: '%s'", key, value);
      if (strcasecmp(key, "name") == 0) {
        if (current_server) {
          if (!current_server->address || !current_server->port
            || !current_server->username || !current_server->nickname) {
            fprintf(stderr, "Error, missing a required key for server %s\n", current_server->unique_name);
            return 0;
          }
        }
        current_server = getServer(value);
        if (!current_server->unique_name) {
          current_server->unique_name = malloc(strlen(value) + 1);
          strcpy(current_server->unique_name, value);
        }
      } else if (current_server == NULL) {
        fprintf(stderr, "You specified the key %s before you specified the name key, which is required.\n", key);
        fprintf(stderr, "You should probably go and fix your config.\n");
        return 0;
      } else if (strcasecmp(key, "address") == 0) {
        if (current_server->address)
          free(current_server->address);
        current_server->address = malloc(strlen(value) + 1);
        strcpy(current_server->address, value);
      } else if (strcasecmp(key, "port") == 0) {
        long port = strtol(value, NULL, 10);
        if ((errno == ERANGE || (port == LONG_MAX || port == LONG_MIN)) || (errno != 0 && port == 0) || port < 0 || port > 65535) {
          fprintf(stderr, "Error at line %d. Port %ld is out of range.\n", line_count, port);
          return 0;
        } else
          current_server->port = port;
      } else if (strcasecmp(key, "username") == 0) {
        if (current_server->username)
          free(current_server->username);
        current_server->username = malloc(strlen(value) + 1);
        strcpy(current_server->username, value);
      } else if (strcasecmp(key, "nickname") == 0) {
        if (current_server->nickname)
          free(current_server->nickname);
        current_server->nickname = malloc(strlen(value) + 1);
        strcpy(current_server->nickname, value);
      }
    } else {
      fprintf(stderr, "Parsing error at line %d.\n", line_count);
      return 0;
    }
  };
  if (current_server) {
    if (!current_server->address || !current_server->port
      || !current_server->username || !current_server->nickname) {
      fprintf(stderr, "Error, missing a required key for server %s\n", current_server->unique_name);
      return 0;
    }
  }
  fclose(f);
  return 1;
};

struct server* getServer(char* name) {
  if (!name)
    return NULL;
  struct server* node = global_config->servers;
  if (!node) {
    global_config->servers = malloc(sizeof(struct server));
    memset(global_config->servers, 0, sizeof(struct server));
    return global_config->servers;
  }
  for (;;) {
    if (strcmp(name, node->unique_name) == 0)
      return node;
    if (!node->next) /* Doing this check right here and breaking out of it */
      break; /* so I won't have to loop through all of them again to get */
    node = node->next; /* the last one to append a new one. */
  }
  node->next = malloc(sizeof(struct server));
  memset(node->next, 0, sizeof(struct server));
  return node->next;
};

int dispatch_config(struct event_base* base) {
  struct server* node = global_config->servers;
  if (!node)
    return 0;
  if (!dns)
    dns = evdns_base_new(base, 1);
  while (node) {
    if (!node->conn) {
      node->conn = malloc(sizeof(struct connection));
      memset(node->conn, 0, sizeof(struct connection));
      node->conn->conn = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
      bufferevent_socket_connect_hostname(node->conn->conn, dns, AF_INET, node->address, node->port);
      bufferevent_setcb(node->conn->conn, irc_conn_readcb, NULL, irc_conn_eventcb, node);
      bufferevent_enable(node->conn->conn, EV_READ);
    }
    node = node->next;
  };
  return 1;
};