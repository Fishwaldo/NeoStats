/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "main.h"
#include "services.h"
#include "ircstring.h"

/* @brief hrand
 * 
 * 
 * 
 * @params upperbound
 * @params lowerbound
 *
 * @returns result
 */

unsigned hrand(unsigned upperbound, unsigned lowerbound) 
{
	if ((upperbound < 1)) {
		nlog (LOG_WARNING, "hrand() invalid value for upperbound");
		return -1;
	}
	return ((unsigned)(rand()%((int)(upperbound-lowerbound+1))-((int)(lowerbound-1))));
}

/* @brief Make the name of a file safe for a filename
 * 
 * given a name, make sure its a safe name for a filename
 * 
 * @params name the name to check. Warning, the name is modified
 *
 * @returns a modified version of the name, that is safe to use as a filename
 */

char *make_safe_filename (char *name) 
{
	char *ptr;

	ptr = name;
	while (*ptr) {
		switch (*ptr) {
#ifdef WIN32
			case '#':
			*ptr = '_';
			break;
#endif
			case '/':
#ifdef WIN32
			*ptr = '.';
#else
			*ptr = ':';
#endif
			break;
		}
		ptr++;
	}
	return name;
}

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

char* sstrdup (const char *s)
{
	char *t = ns_malloc (strlen(s)+1);
	strlcpy (t, s, strlen(s)+1);
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

char* strlwr (char *s)
{
	char *t;
	t = s;
	while (*t) {
		*t = tolower (*t);
		t++;
	}
	return s;
}

/** @brief split_buf
 *  Split a buffer into argument list
 *
 * @return 
 */

EXPORTFUNC int ircsplitbuf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = ns_calloc (sizeof (char *) * argvsize);
	argc = 0;
	/*if (*buf == ':')
		buf++;*/
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = ns_realloc (*argv, sizeof (char *) * argvsize);
		}
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
			if(colon_special) {
				(*argv)[argc++] = buf;
				break;
			}
		}
		s = strpbrk (buf, " ");
		if (s) {
			*s++ = 0;
			while (isspace (*s))
				s++;
		} else {
			s = buf + strnlen (buf, BUFSIZE);
		}
		if (*buf == 0) {
			buf++;
		}
		(*argv)[argc++] = buf;
		buf = s;
	}
	return argc;
}

/** @brief split_buf
 *  Split a buffer into argument list
 *
 * @return 
 */

int split_buf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = ns_calloc (sizeof (char *) * argvsize);
	argc = 0;
	if (*buf == ':')
		buf++;
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = ns_realloc (*argv, sizeof (char *) * argvsize);
		}
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
		}
		s = strpbrk (buf, " ");
		if (s) {
			*s++ = 0;
			while (isspace (*s))
				s++;
		} else {
			s = buf + strnlen (buf, BUFSIZE);
		}
		if (*buf == 0) {
			buf++;
		}
		(*argv)[argc++] = buf;
		buf = s;
	}
	return argc;
}

/** @brief joinbuf 
 *
 *  join an array of arguments into a single buffer
 *
 * @return 
 */

char* joinbuf (char **av, int ac, int from)
{
	int i;
	char *buf;

	buf = ns_malloc (BUFSIZE);
	if(from >= ac) {
		dlog(DEBUG1, "joinbuf: from (%d) >= ac (%d)", from, ac);
		strlcpy (buf, "(null)", BUFSIZE);
	}
	else {
		strlcpy (buf, av[from], BUFSIZE);
		for (i = from + 1; i < ac; i++) {
			strlcat (buf, " ", BUFSIZE);
			strlcat (buf, av[i], BUFSIZE);
		}
	}
	return (char *) buf;
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

void AddStringToList (char ***List, char S[], int *C)
{
	static int numargs = 8;

	if (*C == 0) {
		numargs = 8;
		*List = ns_calloc (sizeof (char *) * numargs);
	} else if (*C  == numargs) {
		numargs += 8;
		*List = ns_realloc (*List, sizeof (char *) * numargs);
	}
	(*List)[*C] = S;
	++*C;
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

char* sctime (time_t stuff)
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

char* sftime (time_t stuff)
{
	struct tm *ltm = localtime (&stuff);

	strftime (fmtime, TIMEBUFSIZE, "%a %b %d %Y %I:%M %p %Z", ltm);
	return fmtime;
}

/** @brief ValidateNick
 *  
 *  Check that passed string is a valid nick
 *  
 *  @param nick to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int ValidateNick (char* nick)
{
	char* ptr;

	ptr = nick;
	while (*ptr) {
		if (!IsNickChar(*ptr)) {
			return NS_FAILURE;
		}
		ptr++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateUser
 *  
 *  Check that passed string is a valid username
 *  
 *  @param username to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int ValidateUser (char* username)
{
	char* ptr;

	ptr = username;
	while (*ptr) {
		if (!IsUserChar(*ptr)) {
			return NS_FAILURE;
		}
		ptr++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateHost
 *  
 *  Check that passed string is a valid hostname
 *  
 *  @param hostname to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int ValidateHost (char* hostname)
{
	char* ptr;

	ptr = hostname;
	while (*ptr) {
		if (!IsHostChar(*ptr)) {
			return NS_FAILURE;
		}
		ptr++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateURL
 *  
 *  Check that passed string is a valid url
 *  
 *  @param url to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int ValidateURL (char* url)
{
	char* ptr;

	if (ircstrncasecmp (url, "http://", 7) !=0)
		return NS_FAILURE;
	ptr = url;
	ptr += 7;
	while (*ptr) {
		if (!IsURLChar(*ptr)) {
			return NS_FAILURE;
		}
		ptr++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateChannel
 *  
 *  Check that passed string is a valid channel name
 *  
 *  @param channel name to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int ValidateChannel (char* channel_name)
{
	char* ptr;

	ptr = channel_name;
	if (!IsChanPrefix(*ptr)) {
		return NS_FAILURE;
	}
	ptr ++;
	while (*ptr) {
		if (!IsChanChar(*ptr)) {
			return NS_FAILURE;
		}
		ptr++;
	}
	return NS_SUCCESS;
}
