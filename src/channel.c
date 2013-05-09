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

#include "channel.h"

#include <stdlib.h>
#include <string.h>

struct channel* new_channel(char* name) {
  struct channel* output = malloc(sizeof(struct channel));
  memset(output, 0, sizeof(struct channel));
  output->name = strdup(name);
  return output;
};

void free_channel(struct channel* channel) {
  if (channel) {
    if (channel->name)
      free(channel->name);
    free(channel);
  };
};

struct channel* get_channel(struct connection* connection, char* channel) {
  if (!connection || !channel)
    return NULL;
  if (!connection->channels) {
    connection->channels = new_channel(channel);
    return connection->channels;
  }
  struct channel* node = connection->channels;
  for (;;) {
    if (strcmp(node->name, channel) == 0)
      return node;
    if (!node->next)
      break;
    node = node->next;
  };
  node->next = new_channel(channel);
  return node->next;
};

int add_user_to_channel(struct channel* channel, struct user* user) {
  if (!channel || !user)
    return 0;
  if (!channel->users) {
    channel->users = user;
    return 0;
  }
  struct user* node = channel->users;
  for (;;) {
    if (strcmp(node->nick, user->nick) == 0)
      return 0;
    if (!node->next)
      break;
    node = node->next;
  };
  node->next = user;
  return 1;
};