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

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "user.h"
#include "config.h"

struct channel {
  char* name;
  struct user* users;
  struct channel* next;
};

struct channel* new_channel(char* name);

void free_channel(struct channel* channel);

struct channel* get_channel(struct connection* connection, char* channel);

int add_user_to_channel(struct channel* channel, struct user* user);

int fill_from_names(struct channel* channel, char* raw_names);

struct user* get_user_from_channel(struct channel* channel, char* nickname);

int remove_user_from_channel(struct channel* channel, struct user* user);

#endif //_CHANNEL_H