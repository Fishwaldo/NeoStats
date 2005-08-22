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

/** @brief hrand
 * 
 * 
 * 
 *  @param upperbound
 *  @param lowerbound
 *
 *  @returns result
 */

unsigned int hrand( const unsigned int upperbound, const unsigned int lowerbound ) 
{
	if( ( upperbound < 1 ) ) {
		nlog( LOG_WARNING, "hrand() invalid value for upperbound" );
		return -1;
	}
	return( ( unsigned )( rand()%( ( int )( upperbound - lowerbound + 1 ) )-( ( int )( lowerbound - 1 ) ) ) );
}

/** @brief make_safe_filename
 * 
 *  given a name, make sure its a safe name for a filename
 * 
 *  @param name to check; name is modified
 *
 *  @returns modified version of name that is safe to use as a filename
 */

char *make_safe_filename( char *name ) 
{
	char *ptr;

	ptr = name;
	while( *ptr ) {
		switch( *ptr ) {
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

/** @brief strip
 * 
 *  removes newlines and carriage returns from a string
 * 
 *  @param line to strip; line is modified
 *
 *  @returns stripped line
 */

void strip( char *line )
{
	char *c;
	if( ( c = strchr( line, '\n' ) ) )
		*c = '\0';
	if( ( c = strchr( line, '\r' ) ) )
		*c = '\0';
}

/** @brief sstrdup
 * 
 *  make a copy of a string allocating memory for the new string
 * 
 *  @param pointer to the string to duplicate
 *
 *  @returns pointer to the new string
 */

char *sstrdup( const char *s )
{
	char *t = ns_malloc( strlen( s )+1 );
	strlcpy( t, s, strlen( s )+1 );
	if( !t ) {
		nlog( LOG_CRITICAL, "sstrdup(): out of memory." );
		do_exit( NS_EXIT_ERROR, "Out of memory" );
	}
	return t;
}

/** @brief strlwr
 * 
 *  make string string lowercase
 * 
 *  @param string to convert to lowercase; string is modified
 *
 *  @returns pointer to the new string
 */

char *strlwr( char *s )
{
	char *t;
	t = s;
	while( *t ) {
		*t = tolower( *t );
		t++;
	}
	return s;
}

/** @brief ircsplitbuf
 * 
 *  Split a buffer into argument list observing IRC protocols
 * 
 *  @param buf buffer to convert
 *  @param argv list of arguments to write
 *  @param colon_special flag to indicate colon processing
 *
 *  @returns count of arguments created from split
 */

int ircsplitbuf( char *buf, char ***argv, int colon_special )
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = ns_calloc( sizeof( char * ) * argvsize );
	argc = 0;
	/*if( *buf == ':' )
		buf++;*/
	while( *buf ) {
		if( argc == argvsize ) {
			argvsize += 8;
			*argv = ns_realloc( *argv, sizeof( char * ) * argvsize );
		}
		if( ( *buf == ':' ) &&( colcount < 1 ) ) {
			buf++;
			colcount++;
			if( colon_special ) {
				( *argv )[argc++] = buf;
				break;
			}
		}
		s = strpbrk( buf, " " );
		if( s ) {
			*s++ = 0;
			while( isspace( *s ) )
				s++;
		} else {
			s = buf + strnlen( buf, BUFSIZE );
		}
		if( *buf == 0 ) {
			buf++;
		}
		( *argv )[argc++] = buf;
		buf = s;
	}
	return argc;
}

/** @brief split_buf
 * 
 *  Split a buffer into argument list
 * 
 *  @param buf buffer to convert
 *  @param argv list of arguments to write
 *  @param colon_special flag to indicate colon processing
 *
 *  @returns count of arguments created from split
 */

int split_buf( char *buf, char ***argv, int colon_special )
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = ns_calloc( sizeof( char * ) * argvsize );
	argc = 0;
	if( *buf == ':' )
		buf++;
	while( *buf ) {
		if( argc == argvsize ) {
			argvsize += 8;
			*argv = ns_realloc( *argv, sizeof( char * ) * argvsize );
		}
		if( ( *buf == ':' ) &&( colcount < 1 ) ) {
			buf++;
			colcount++;
		}
		s = strpbrk( buf, " " );
		if( s ) {
			*s++ = 0;
			while( isspace( *s ) )
				s++;
		} else {
			s = buf + strnlen( buf, BUFSIZE );
		}
		if( *buf == 0 ) {
			buf++;
		}
		( *argv )[argc++] = buf;
		buf = s;
	}
	return argc;
}

/** @brief joinbuf
 * 
 *  join an array of arguments into a single buffer
 * 
 *  @param av list of arguments to combine
 *  @param ac count of arguments to combine
 *  @param from position to start combine
 *
 *  @returns buffer containing combined arguments
 */

char *joinbuf( char **av, int ac, int from )
{
	int i;
	char *buf;

	buf = ns_malloc( BUFSIZE );
	if( from >= ac ) {
		dlog( DEBUG1, "joinbuf: from (%d) >= ac (%d)", from, ac );
		strlcpy( buf, "( null )", BUFSIZE );
	}
	else {
		strlcpy( buf, av[from], BUFSIZE );
		for( i = from + 1; i < ac; i++ ) {
			strlcat( buf, " ", BUFSIZE );
			strlcat( buf, av[i], BUFSIZE );
		}
	}
	return( char * ) buf;
}

/** @brief AddStringToList
 * 
 *  Adds a string to an array of strings
 * 
 *  @param List the array you wish to append S to 
 *  @param S the string you wish to append
 *  @param C current size of the array
 *
 *  @returns none
 */

void AddStringToList( char ***List, char S[], int *C )
{
	static int numargs = 8;

	if( *C == 0 ) {
		numargs = 8;
		*List = ns_calloc( sizeof( char * ) * numargs );
	} else if( *C  == numargs ) {
		numargs += 8;
		*List = ns_realloc( *List, sizeof( char * ) * numargs );
	}
	( *List )[*C] = S;
	++*C;
}

/** @brief strip_mirc_codes
 * 
 *  Remove colour control codes from mirc, pirch etc.
 *  taken from eggdrop
 * 
 *  @param text to strip
 *
 *  @returns none
 */

void strip_mirc_codes( char *text )
{
	char *dd = text;

	while( *text ) {
		switch( *text ) {
			case 1:
				text++;			/* ctcp stuff */
				continue;
				break;
			case 2:			/* Bold text */
				text++;
				continue;
				break;
			case 3:			/* mIRC colors? */
				if( isdigit( text[1] ) ) {	/* Is the first char a number? */
					text += 2;		/* Skip over the ^C and the first digit */
					if( isdigit( *text ) )
						text++;		/* Is this a double digit number? */
					if( *text == ',' ) {	/* Do we have a background color next? */
						if( isdigit( text[1] ) )
							text += 2;	/* Skip over the first background digit */
						if( isdigit( *text ) )
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
				if( *text == '[' ) {
					text++;
					while( ( *text == ';' ) || isdigit( *text ) )
						text++;
					if( *text )
						text++;		/* also kill the following char */
				}
				continue;
				break;
		}
		*dd++ = *text++;		/* Move on to the next char */
	}
	*dd = 0;
}

/** @brief sctime
 * 
 *  
 * 
 *  @param stuff
 *
 *  @returns 
 */

char *sctime( time_t stuff )
{
	char *s, *c;

	s = ctime( &stuff );
	if( ( c = strchr( s, '\n' ) ) )
		*c = '\0';

	return s;
}

/** @brief sftime
 * 
 *  
 * 
 *  @param stuff
 *
 *  @returns 
 */

static char fmtime[TIMEBUFSIZE];

char *sftime( time_t stuff )
{
	struct tm *ltm = localtime( &stuff );

	strftime( fmtime, TIMEBUFSIZE, "%a %b %d %Y %I:%M %p %Z", ltm );
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

int ValidateNick( const char *nick )
{
	while( *nick )
	{
		if( !IsNickChar( *nick ) )
			return NS_FAILURE;
		nick++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateNickWild
 *  
 *  Check that passed string is a valid nick
 *  Wild cards allowed
 *  
 *  @param nick to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateNickWild( const char *nick )
{
	while( *nick )
	{
		if( !IsNickChar( *nick ) && !IsWildChar( *nick ) )
			return NS_FAILURE;
		nick++;
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

int ValidateUser( const char *username )
{
	while( *username )
	{
		if( !IsUserChar( *username ) )
			return NS_FAILURE;
		username++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateUserWild
 *  
 *  Check that passed string is a valid username
 *  Wild cards allowed
 *  
 *  @param username to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateUserWild( const char *username )
{
	while( *username )
	{
		if( !IsUserChar( *username ) && !IsWildChar( *username ) )
			return NS_FAILURE;
		username++;
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

int ValidateHost( const char *hostname )
{
	while( *hostname )
	{
		if( !IsHostChar( *hostname ) )
			return NS_FAILURE;
		hostname++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateHostWild
 *  
 *  Check that passed string is a valid hostname
 *  Wild cards allowed
 *  
 *  @param hostname to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateHostWild( const char *hostname )
{
	while( *hostname )
	{
		if( !IsHostChar( *hostname ) && !IsWildChar( *hostname ) )
			return NS_FAILURE;
		hostname++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateUserHost
 *  
 *  Check that passed string is a valid nick!user@host
 *  
 *  @param hostname to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateUserHost( const char *userhost )
{
	static char localuserhost[USERHOSTLEN];
	char *nick, *user , *host;

	if( !index( userhost, '!' ) || !index( userhost, '@' ) )
		return NS_FAILURE;

	strlcpy( localuserhost, userhost, USERHOSTLEN );
	nick = strtok( localuserhost, "!" );
	user = strtok( NULL, "@" );
	host = strtok( NULL, "" );

	if( ValidateNick( nick ) != NS_SUCCESS )
		return NS_FAILURE;
	if( ValidateUser( user ) != NS_SUCCESS )
		return NS_FAILURE;
	if( ValidateHost( host ) != NS_SUCCESS )
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief ValidateUserHostWild
 *  
 *  Check that passed string is a valid nick!user@host
 *  Wild cards allowed
 *  
 *  @param hostname to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateUserHostWild( const char *userhost )
{
	static char localuserhost[USERHOSTLEN];
	char *nick, *user , *host;

	if( !index( userhost, '!' ) || !index( userhost, '@' ) )
		return NS_FAILURE;

	strlcpy( localuserhost, userhost, USERHOSTLEN );
	nick = strtok( localuserhost, "!" );
	user = strtok( NULL, "@" );
	host = strtok( NULL, "" );

	if( ValidateNickWild( nick ) != NS_SUCCESS )
		return NS_FAILURE;
	if( ValidateUserWild( user ) != NS_SUCCESS )
		return NS_FAILURE;
	if( ValidateHostWild( host ) != NS_SUCCESS )
		return NS_FAILURE;
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

int ValidateURL( const char *url )
{
	/* URL must begin with http:// */
	if( ircstrncasecmp( url, "http://", 7 ) != 0 )
		return NS_FAILURE;
	/* Get pointer to rest of URL to test */
	url += 7;
	while( *url )
	{
		if( !IsURLChar( *url ) )
			return NS_FAILURE;
		url++;
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

int ValidateChannel( const char *channel_name )
{
	/* Channel name must start with channel prefix */
	if( !IsChanPrefix( *channel_name ) )
		return NS_FAILURE;
	channel_name ++;
	while( *channel_name )
	{
		if( !IsChanChar( *channel_name ) )
			return NS_FAILURE;
		channel_name++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateChannelWild
 *  
 *  Check that passed string is a valid channel name
 *  Wild cards allowed
 *  
 *  @param channel name to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateChannelWild( const char *channel_name )
{
	/* Channel name must start with channel prefix */
	if( !IsChanPrefix( *channel_name ) )
		return NS_FAILURE;
	channel_name ++;
	while( *channel_name )
	{
		if( !IsChanChar( *channel_name ) && !IsWildChar( *channel_name ) )
			return NS_FAILURE;
		channel_name++;
	}
	return NS_SUCCESS;
}

/** @brief ValidateChannelKey
 *  
 *  Check that passed string is a valid channel key
 *  
 *  @param channel key to check
 *  
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ValidateChannelKey( const char *key )
{
	while( *key )
	{
		if( !IsChanKeyChar( *key ) )
			return NS_FAILURE;
		key++;
	}
	return NS_SUCCESS;
}
