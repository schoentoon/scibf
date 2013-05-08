/*  Scibf
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

#include "irc_callbacks.h"

#include "config.h"
#include "debug.h"

#include <event.h>
#include <stdlib.h>
#include <string.h>

void irc_conn_readcb(struct bufferevent *bev, void* args) {
  DEBUG(255, "irc_conn_readcb(%p, %p);", bev, args);
  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);
  size_t len;
  char* line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
  while (line) {
    DEBUG(254, "In: '%s'", line);
    char buf[BUFSIZ];
    static const char* IRC_PING_SSCANF = "PING %s";
    const char* IRC_PONG_PRINTF = "PONG %s\r\n";
    if (sscanf(line, IRC_PING_SSCANF, buf) == 1)
      evbuffer_add_printf(output, IRC_PONG_PRINTF, buf);
    free(line);
    line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
  };
};

void irc_conn_eventcb(struct bufferevent *bev, short events, void* args) {
  DEBUG(255, "irc_conn_eventcb(%p, %d, %p);", bev, events, args);
  if (!(events & BEV_EVENT_CONNECTED)) {
    struct event_base* base = bufferevent_get_base(bev);
    bufferevent_free(bev);
    struct server* node = (struct server*) args;
    node->conn->conn = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_socket_connect_hostname(node->conn->conn, dns, AF_INET, node->address, node->port);
    bufferevent_setcb(node->conn->conn, irc_conn_readcb, NULL, irc_conn_eventcb, node);
    bufferevent_enable(node->conn->conn, EV_READ);
  }
};