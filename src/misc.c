/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "neostats.h"
#include "services.h"

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

/** @brief NeoStats wrapper for malloc.
 *
 * Allocates memory for internal variables. 
 * If enough memory can't be allocated, exit the program 
 *
 * @param size The amount of memory to alloc
 *
 * @returns pointer to allocated buffer
 */

void * smalloc ( const int size )
{
	unsigned int allocsize;
	void *buf;

	allocsize = size;
	if (!allocsize) {
		nlog (LOG_WARNING, "smalloc: illegal attempt to allocate 0 bytes!");
		allocsize = 1;
	}
	buf = malloc (allocsize);
	if (!buf) {
		nlog (LOG_CRITICAL, "smalloc: out of memory.");
		do_exit (NS_EXIT_ERROR, "Out of memory");
	}
	return buf;
}

/** @brief NeoStats wrapper for calloc.
 *
 * Allocates memory for internal variables. 
 * If enough memory can't be allocated, exit the program 
 *
 * @param size The amount of memory to alloc
 *
 * @returns pointer to allocated buffer
 */

void * scalloc ( const int size )
{
	void *buf;
	unsigned int allocsize;

	allocsize = size;
	if (!allocsize) {
		nlog (LOG_WARNING, "scalloc: illegal attempt to allocate 0 bytes!");
		allocsize = 1;
	}
	buf = calloc (1, allocsize);
	if (!buf) {
		nlog (LOG_CRITICAL, "scalloc: out of memory.");
		do_exit (NS_EXIT_ERROR, "Out of memory");
	}
	return buf;
}

/** @brief NeoStats wrapper for realloc.
 *
 * Reallocates memory
 * If enough memory can't be allocated, exit the program 
 *
 * @param size The amount of memory to realloc
 *
 * @returns pointer to allocated buffer
 */

void * srealloc ( void* ptr, const int size )
{
	void* newptr;

	newptr = realloc (ptr, size);
	if (!newptr) {
		nlog (LOG_CRITICAL, "srealloc: out of memory.");
		do_exit (NS_EXIT_ERROR, "Out of memory");
	}
	return newptr;
}

/** @brief NeoStats wrapper for free.
 *
 * Free memory associated with pointer.
 * If NULL pointer log error and ignore free
 *
 * @param size Pointer to buffer to free
 *
 * @returns none
 */

void sfree ( void *buf )
{
	if (!buf) {
		nlog (LOG_WARNING, "sfree: illegal attempt to free NULL pointer");
		return;
	}
	free (buf);
	buf = 0;
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
		nlog (LOG_CRITICAL, "sstrdup(): out of memory.");
		do_exit (NS_EXIT_ERROR, "Out of memory");
	}
	return t;
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
	static int numargs = 8;

	if (*C == 0) {
		numargs = 8;
		*List = scalloc (sizeof (char *) * numargs);
	} else if (*C  == numargs) {
		numargs += 8;
		*List = srealloc (*List, sizeof (char *) * numargs);
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
	chanalert (ns_botptr->nick, misc_buf);
}
