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
          if (sscanf(rest, NAMES_REST_SSCANF, me, chan, names) == 3) {
            struct channel* channel = get_channel(server->conn, chan);
            fill_from_names(channel, names);
          }
          break;
        };
        case 366: { /* :localhost 366 IAmABot #test :End of /NAMES list. */
          DEBUG(255, "rest: '%s'", rest);
          static const char* END_OF_NAMES_CHAN = "%s %s";
          char me[32];
          char chan[32];
          if (sscanf(rest, END_OF_NAMES_CHAN, me, chan) == 2) {
            static const char* WHO_CHAN_PRINTF = "WHO %s\r\n";
            evbuffer_add_printf(output, WHO_CHAN_PRINTF, chan);
          }
          break;
        };
        case 422:   /* :localhost 422 IAmABot :MOTD File is missing */
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
          static const char* IRC_JOIN_EVENT = "JOIN";
          static const char* IRC_NICK_EVENT = "NICK";
          static const char* IRC_PART_EVENT = "PART";
          static const char* IRC_QUIT_EVENT = "QUIT";
          if (strcmp(event, IRC_JOIN_EVENT) == 0) {
            struct channel* channel = get_channel(server->conn, &rest[1]);
            struct user* user = new_user(&server_name[1]);
            add_user_to_channel(channel, user);
          } else if (strcmp(event, IRC_NICK_EVENT) == 0) {
            char buf[32];
            if (get_nickname(server_name, buf)) {
              struct channel* node = server->conn->channels;
              while (node) {
                struct user* user = get_user_from_channel(node, buf);
                if (user) {
                  DEBUG(255, "Changing user '%s' to '%s' in channel %s", user->nick, &rest[1], node->name);
                  free(user->nick);
                  user->nick = strdup(&rest[1]);
                }
                node = node->next;
              };
            }
          } else if (strcmp(event, IRC_PART_EVENT) == 0) {
            struct channel* channel = get_channel(server->conn, &rest[1]);
            if (channel) {
              char buf[32];
              if (get_nickname(server_name, buf)) {
                struct user* user = get_user_from_channel(channel, buf);
                remove_user_from_channel(channel, user);
              }
            }
          } else if (strcmp(event, IRC_QUIT_EVENT) == 0) {
            char buf[32];
            if (get_nickname(server_name, buf)) {
              struct channel* node = server->conn->channels;
              while (node) {
                struct user* user = get_user_from_channel(node, buf);
                remove_user_from_channel(node, user);
                node = node->next;
              };
            };
          };
        }
      }
    }
    free(line);
    line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
  };
};

struct reconnect_struct {
  struct event_base* base;
  struct server* server;
};

static void reconnectServer(evutil_socket_t fd, short event, void* args) {
  DEBUG(255, "reconnectServer(%d, 0x%02x, %p);", fd, event, args);
  struct reconnect_struct* reconnect_struct = (struct reconnect_struct*) args;
  startConnection(reconnect_struct->server, reconnect_struct->base);
  free(reconnect_struct);
};

void irc_conn_eventcb(struct bufferevent *bev, short event, void* args) {
  DEBUG(255, "irc_conn_eventcb(%p, 0x%02x, %p);", bev, event, args);
  if (!(event & BEV_EVENT_CONNECTED)) {
    struct event_base* base = bufferevent_get_base(bev);
    struct server* node = (struct server*) args;
    free_connection(node->conn);
    node->conn = NULL;
    struct timeval tv = { node->retry_time, 0 };
    struct reconnect_struct* reconnect_struct = malloc(sizeof(struct reconnect_struct));
    reconnect_struct->server = node;
    reconnect_struct->base = base;
    event_base_once(base, -1, EV_TIMEOUT, reconnectServer, reconnect_struct, &tv);
  }
};