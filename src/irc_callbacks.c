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
#include <ctype.h>
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
    else if (len >= 3 && isdigit(line[0]) && isdigit(line[1]) && isdigit(line[2])) {
      unsigned short uRaw = line[0]-'0';
      uRaw = (uRaw * 10) + line[1]-'0';
      uRaw = (uRaw * 10) + line[2]-'0';
      switch (uRaw) {
      case 001:
        break;
      };
    }
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
    node->conn->conn = NULL;
    startConnection(node, base);
  }
};