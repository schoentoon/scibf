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
    return 1;
  }
  struct user* node = channel->users;
  for (;;) {
    if (strcmp(node->nick, user->nick) == 0) {
      if (user->mode == node->mode)
        return 0;
      else {
        node->mode = user->mode;
        return 0;
      }
    }
    if (!node->next)
      break;
    node = node->next;
  };
  node->next = user;
  return 1;
};

int fill_from_names(struct channel* channel, char* raw_names) {
  if (!channel || !raw_names)
    return 0;
  char* names = strdup(raw_names);
  char* name = NULL;
  while ((name = strsep(&names, " "))) {
    struct user* user = new_user(name);
    if (!user->nick || add_user_to_channel(channel, user) == 0)
      free_user(user);
  }
  free(names);
  return 1;
};

struct user* get_user_from_channel(struct channel* channel, char* nickname) {
  if (!channel || !nickname)
    return NULL;
  struct user* node = channel->users;
  while (node) {
    if (strcmp(node->nick, nickname) == 0)
      return node;
    node = node->next;
  };
  return NULL;
};

int remove_user_from_channel(struct channel* channel, struct user* user) {
  if (!channel || !user)
    return 0;
  struct user* node = channel->users;
  while (node) {
    if (node->next == user) {
      if (user->next)
        node->next = user->next;
      else
        node->next = NULL;
      free_user(user);
      return 1;
    }
    node = node->next;
  };
  return 0;
};