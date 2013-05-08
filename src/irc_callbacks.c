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

#include "irc_callbacks.h"

#include "config.h"
#include "debug.h"

#include <event.h>

void irc_conn_readcb(struct bufferevent *bev, void* args) {
  DEBUG(255, "irc_conn_readcb(%p, %p);", bev, args);
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