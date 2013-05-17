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

#ifndef _USER_H
#define _USER_H

#define VOICED_USER   0x01
#define HALFOPERATOR  0x02
#define CHAN_OPERATOR 0x04
#define CHAN_OWNER    0x08
#define SUPEROP       0x10

struct user {
  char* user;
  char* nick;
  char* host;
  unsigned char mode;
  struct user* next;
};


/** @summary Allocates a new struct user based on the input.
 * Should be used internally only.
 * @param raw The raw input, this can be from either NAMES,
 * after which it'll also set the correct permissions. And
 * it can be the normal :nick!user@host
 * @return This will always return a newly allocated struct user
 */
struct user* new_user(char* raw);

/** @summary This'll free a struct user pointer corrrectly.
 * Should be used internally only.
 */
void free_user(struct user* user);

/** @summary Used to quickly strip just a nickname from :nick!user@host
 * Should be used internally only.
 * @param rawline The rawline to actually parse the nick from.
 * @param buf The buffer which will be used to set the nickname to.
 * @return 1 in case we got the nickname correctly, 0 otherwise.
 */
int get_nickname(char* rawline, char* buf);

#endif //_USER_H