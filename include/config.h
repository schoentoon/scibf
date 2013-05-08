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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <event2/dns.h>
#include <event2/bufferevent.h>

struct connection {
  char* username;
  char* nickname;
  char* hostname;
  struct bufferevent* conn;
};

struct server {
  char* unique_name;
  char* address;
  unsigned short port;
  char* username;
  char* nickname;
  struct connection* conn;
  struct server* next;
};

struct config {
  struct server* servers;
};

struct config* global_config;

struct evdns_base* dns;

int parse_config(char* config_file);

int dispatch_config(struct event_base* base);

#endif //_CONFIG_H