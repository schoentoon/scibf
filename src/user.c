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

#include "user.h"

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct user* new_user(char* raw) {
  struct user* output = malloc(sizeof(struct user));
  memset(output, 0, sizeof(struct user));
  static const char* USER_SSCANF = "%31[^!]!%31[^@]@%31s";
  char nick[32];
  char user[32];
  char host[32];
  if (sscanf(raw, USER_SSCANF, nick, user, host) == 3) {
    output->nick = strdup(nick);
    output->user = strdup(user);
    output->host = strdup(host);
  };
  DEBUG(255, "New user %s!%s@%s", output->nick, output->user, output->host);
  return output;
};

void free_user(struct user* user) {
  if (user) {
    if (user->host)
      free(user->host);
    if (user->nick)
      free(user->nick);
    if (user->user)
      free(user->user);
    free(user);
  }
};