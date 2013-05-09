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

#include "debug.h"
#include "config.h"
#include "channel.h"

#include <event.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void irc_conn_readcb(struct bufferevent *bev, void* args) {
  DEBUG(255, "irc_conn_readcb(%p, %p);", bev, args);
  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);
  struct server* server = (struct server*) args;
  size_t len;
  char* line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
  while (line) {
    DEBUG(254, "In: '%s'", line);
    char buf[BUFSIZ];
    static const char* IRC_PING_SSCANF = "PING %s";
    static const char* IRC_PONG_PRINTF = "PONG %s\r\n";
    if (sscanf(line, IRC_PING_SSCANF, buf) == 1)
      evbuffer_add_printf(output, IRC_PONG_PRINTF, buf);
    else if (len >= 3) {
      static const char* IRC_NUMBERED_EVENT = "%s %d %[^\r\n]";
      static const char* IRC_GENERAL_EVENT = "%s %s %[^\r\n]";
      int uRaw = 0;
      char rest[BUFSIZ];
      char server_name[BUFSIZ];
      if (sscanf(line, IRC_NUMBERED_EVENT, server_name, &uRaw, rest) == 3) {
        switch (uRaw) {
        case 353: { /* :localhost 353 IAmABot = #test :IAmABot ~@Schoentoon @SomeOp */
          static const char* NAMES_REST_SSCANF = "%s = %s :%[^\r\n]";
          char me[32];
          char chan[32];
          char names[BUFSIZ];
          DEBUG(255, "Line '%s'", line);
          DEBUG(255, "Rest '%s'", rest);
          if (sscanf(rest, NAMES_REST_SSCANF, me, chan, names) == 3) {
            struct channel* channel = get_channel(server->conn, chan);
            if (channel)
              fill_from_names(channel, names);
          }
          break;
        };
        case 376: { /* :localhost 376 IAmABot :End of MOTD command. */
          static const char* IRC_JOIN_PRINTF = "JOIN %s\r\n";
          unsigned int i;
          for (i = 0; server->channels[i]; i++)
            evbuffer_add_printf(output, IRC_JOIN_PRINTF, server->channels[i]);
          break;
        };
        };
      } else {
        char event[BUFSIZ];
        if (sscanf(line, IRC_GENERAL_EVENT, server_name, event, rest) == 3) {
          static const char* IRC_JOIN = "JOIN";
          if (strcmp(event, IRC_JOIN) == 0) {
            struct channel* channel = get_channel(server->conn, &rest[1]);
            struct user* user = new_user(&server_name[1]);
            add_user_to_channel(channel, user);
          }
        }
      }
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