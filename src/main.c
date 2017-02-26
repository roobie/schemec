/*

  main.c -- Template main()

  This program is free software you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <libguile.h>

#define E_OUT_OF_MEMORY 2

#define BUF_SIZE 4096	// How many bytes to read at a time

int
readline(char *buffer, int buf_size) {
  while (1) {
    int c = getchar();
    if (c == EOF) {
      break;
    }

    if (!isspace(c)) {
      ungetc(c, stdin);
      break;
    }
  }

  int i = 0;
  while (1) {
    int c = getchar();
    if (isspace(c) || c == EOF) {
      buffer[i] = 0;
      break;
    }
    buffer[i] = c;
    if (i == buf_size - 1) {
      buf_size += buf_size;
      buffer = (char*)realloc(buffer, buf_size);

      if (buffer == NULL) {
        return E_OUT_OF_MEMORY;
      }
    }
    ++i;
  }

  return 0;
}

static SCM
my_hostname (void)
{
  char *s = getenv("HOSTNAME");
  if (s == NULL) {
    return SCM_BOOL_F;
  }
  else {
    return scm_from_locale_string(s);
  }
}

static void
inner_main(void *closure, int argc, char **argv)
{
  /* preparation */
  scm_c_define_gsubr("my-hostname", 0, 0, 0, my_hostname);
  scm_c_primitive_load("./scheme/main.scm");

  // start shell
  scm_shell(argc, argv);
  /* after exit */
}

int
main(int argc, char **argv)
{
  scm_boot_guile(argc, argv, inner_main, 0);
  return 0; /* never reached, see inner_main */
}
