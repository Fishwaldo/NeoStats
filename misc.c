/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id$
*/

#include <stdio.h>
#include "stats.h"
#include "log.h"

static char misc_buf[BUFSIZE];


/** @brief strip newlines carriage returns
 *
 * removes newlines and carriage returns from a string
 *
 * @param line the line to strip (warning, Modfied!)
 * @retval line the stripped line
 */

void
strip (char *line)
{
	char *c;
	if ((c = strchr (line, '\n')))
		*c = '\0';
	if ((c = strchr (line, '\r')))
		*c = '\0';
}

/** @brief Our Own implementation of Malloc.
 *
 * Allocates memory for internal variables. Usefull for Memory Debugging
 * if enough memory can't be malloced, exit the program 
 *
 * @param size The amount of memory to Malloc
 *
 * @returns size bytes of memory or NULL on malloc error
 *
 * @todo move this to a util type file, as it being in main is crazy
 */

void *
smalloc (long size)
{
	void *buf;

	if (!size) {
		nlog (LOG_WARNING, LOG_CORE, "smalloc(): illegal attempt to allocate 0 bytes!");
		size = 1;
	}
	buf = malloc (size);
	if (!buf) {
		nlog (LOG_CRITICAL, LOG_CORE, "smalloc(): out of memory.");
		do_exit (NS_EXIT_ERROR, "Out of memory");
	}
	return buf;
}

/** @brief Duplicate a string
 *
 * make a copy of a string, with memory allocated for the new string
 *
 * @param s a pointer to the string to duplicate
 *
 * @returns a pointer to the new string
 *
 * @deprecated  Try not to use this function, it will go away one day
 *
 */

char *
sstrdup (const char *s)
{
	char *t = strdup (s);
	if (!t) {
		nlog (LOG_CRITICAL, LOG_CORE, "sstrdup(): out of memory.");
		do_exit (NS_EXIT_ERROR, "Out of memory");
	}
	return t;
}

/** @brief Create a HASH from a string
 *
 * Makes a hash of a string for a table
 *
 * @param name The string to use as the base for the hash
 *
 * @param size_of_table The size of the hash table
 *
 * @returns unsigned long of the hash
 *
 * @deprecated  Try not to use this function, it will go away one day
 *
 */

unsigned long
HASH (const unsigned char *name, int size_of_table)
{
	unsigned long h = 0, g;

	while (*name) {
		h = (h << 4) + *name++;
		if ((g = h & 0xF0000000))
			h ^= g >> 24;
		h &= ~g;
	}
	return h % size_of_table;
}

/** @brief convert a string to lowercase
 *
 * makes a string lowercase
 *
 * @param s the string to convert to lowercase. WARNING: the result overwrites this variable
 *
 * @returns pointer to the lowercase version of s
 *
 */

char *
strlwr (char *s)
{
	char *t;
	t = s;
	while (*t) {
		*t = tolower (*t);
		t++;
	}
	return s;
}

/** @brief Adds a string to an array of strings
 *
 * used for the event functions, adds a string to an array of string pointers to pass to modules
 *
 * @param List the array you wish to append S to 
 * @param S the string you wish to append
 * @param C the current size of the array
 * 
 * @returns Nothing
 *
 */

void
AddStringToList (char ***List, char S[], int *C)
{
	if (*C == 0) {
		*List = calloc (sizeof (char *) * 16, 1);
	} else if (*C >= 16) {
		nlog(LOG_CRITICAL, LOG_CORE, "AddStringToList is full. Not Adding");
		return;
	}
	++*C;
	(*List)[*C - 1] = S;
}

/** @brief 
 *
 * @param 
 * 
 * @returns 
 *
 */
/* this came from eggdrop sources */
/* Remove the color control codes that mIRC,pIRCh etc use to make
 * their client seem so fecking cool! (Sorry, Khaled, you are a nice
 * guy, but when you added this feature you forced people to either
 * use your *SHAREWARE* client or face screenfulls of crap!)
 */
void strip_mirc_codes(char *text)
{
  char *dd = text;

  while (*text) {
    switch (*text) {
    case 1:
    	text++;			/* ctcp stuff */
    	continue;
      break;
    case 2:			/* Bold text */
	text++;
	continue;
      break;
    case 3:			/* mIRC colors? */
	if (isdigit(text[1])) {	/* Is the first char a number? */
	  text += 2;		/* Skip over the ^C and the first digit */
	  if (isdigit(*text))
	    text++;		/* Is this a double digit number? */
	  if (*text == ',') {	/* Do we have a background color next? */
	    if (isdigit(text[1]))
	      text += 2;	/* Skip over the first background digit */
	    if (isdigit(*text))
	      text++;		/* Is it a double digit? */
	  }
	continue;
      }
      break;
    case 7:
	text++;
	continue;
      break;
    case 0x16:			/* Reverse video */
	text++;
	continue;
      break;
    case 0x1f:			/* Underlined text */
	text++;
	continue;
      break;
    case 033:
	text++;
	if (*text == '[') {
	  text++;
	  while ((*text == ';') || isdigit(*text))
	    text++;
	  if (*text)
	    text++;		/* also kill the following char */
	}
	continue;
      break;
    }
    *dd++ = *text++;		/* Move on to the next char */
  }
  *dd = 0;
}

/** @brief 
 *
 * @param 
 * 
 * @returns 
 *
 */
char *
sctime (time_t stuff)
{
	char *s, *c;

	s = ctime (&stuff);
	if ((c = strchr (s, '\n')))
		*c = '\0';

	return s;
}

/** @brief 
 *
 * @param 
 * 
 * @returns 
 *
 */
static char fmtime[TIMEBUFSIZE];

char *
sftime (time_t stuff)
{
	struct tm *ltm = localtime (&stuff);

	strftime (fmtime, TIMEBUFSIZE, "[%b (%a %d) %Y  %I:%M [%p/%Z]]", ltm);

	return fmtime;
}

/** @brief 
 *
 * @param 
 * 
 * @returns 
 *
 */
void
debugtochannel(char *message, ...)
{
	va_list ap;

	SET_SEGV_LOCATION();
#ifndef DEBUG
	if (!me.debug_mode)
		return;
#endif
	va_start (ap, message);
	ircvsnprintf (misc_buf, BUFSIZE, message, ap);
	va_end (ap);
	chanalert (s_Services, misc_buf);
}
