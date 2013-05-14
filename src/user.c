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
  if (raw) {
    unsigned char run = 1;
    while (run) {
      switch (*raw) {
      case '+':
        output->mode |= VOICED_USER;
        raw++;
        break;
      case '%':
        output->mode |= HALFOPERATOR;
        raw++;
        break;
      case '@':
        output->mode |= CHAN_OPERATOR;
        raw++;
        break;
      case '&': /* We are simply ignoring these for now, should be fairly easy to add them later. */
      case '~':
        raw++;
        break;
      default:
        run = 0;
        break;
      };
    }
    static const char* USER_SSCANF = "%31[^!]!%31[^@]@%31s";
    char nick[32];
    char user[32];
    char host[32];
    if (sscanf(raw, USER_SSCANF, nick, user, host) == 3) {
      output->nick = strdup(nick);
      output->user = strdup(user);
      output->host = strdup(host);
    } else if (strlen(raw) > 0)
      output->nick = strdup(nick);
  }
  DEBUG(255, "New user %s!%s@%s", output->nick, output->user, output->host);
  return output;
};

void free_user(struct user* user) {
  if (user) {
    free(user->host);
    free(user->nick);
    free(user->user);
    free(user);
  }
};

int get_nickname(char* rawline, char* buf) {
  static const char* USER_SSCANF = ":%31[^!]!";
  return sscanf(rawline, USER_SSCANF, buf);
};