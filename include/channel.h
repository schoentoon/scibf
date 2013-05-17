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
#include "defines.h"

struct channel {
  char* name;
  struct user* users;
  struct channel* next;
};

/** @summary Allocates a new channel
 * Should be used internally only.
 * @param name The name to give the actual channel
 */
struct channel* new_channel(char* name);

/** @summary Frees the entire channel, including the users
 * @see free_user
 * Should be used internally only.
 * @param channel The channel to free.
 */
void free_channel(struct channel* channel);

/** @summary Looking for a certain channel?
 * @param connection We're searching in this connection for the channel
 * @param channel Name of the channel we're actually looking for
 * @return Either the found channel struct or a newly allocated one
 * @see new_channel
 */
struct channel* get_channel(struct connection* connection, char* channel);

/** @summary Used to add a user to a channel
 * Should be used internally only.
 * @param channel The channel to add the user to
 * @param user The user to add to the channel
 * @return 1 if the user pointer was actually added, 0 otherwise
 */
int add_user_to_channel(struct channel* channel, struct user* user);

/** @summary Quickly fill a channel with names from the NAMES header
 * Should be used internally only.
 * @param channel The channel to fill
 * @param raw_names A space seperated list of nicknames like in the NAMES header
 * @return 1 if everything went right, 0 otherwise.
 */
int fill_from_names(struct channel* channel, char* raw_names);

/** @summary Lookup a user with a certain nickname
 * @param channel Look in this channel for the nickname
 * @param nickname The nickname to match
 * @return The user that matches the nickname if it was found, NULL otherwise
 */
struct user* get_user_from_channel(struct channel* channel, char* nickname);

/** @summary Remove a user from a channel and deallocate it
 * @param channel The channel to remove the user from
 * @param user The user to remove from the channel and free
 * @see free_user
 * @return 1 if it was removed and freed, 0 otherwise
 */
int remove_user_from_channel(struct channel* channel, struct user* user);

/** @summary Update the user info from raw_line in the channel
 * @param channel The channel to loop through to get the user to update
 * @param raw_line A part of the raw WHO header to parse
 * @return 1 if a user was actually updated, 0 otherwise
 */
int parse_who_header(struct channel* channel, char* raw_line);

#endif //_CHANNEL_H