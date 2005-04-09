/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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
#include "ircd.h"
#include "numerics.h"

ircd_cmd numeric_cmd_list[] = {
	/*Message	Token	handler	usage */
	{"351", "351", m_numeric351, 0},
	{"242", "242", m_numeric242, 0},
	{0, 0, 0, 0},
};

/*  RX: :irc.foo.com 250 NeoStats :Highest connection count: 3( 2 clients )
 *  RX: :irc.foo.com 219 NeoStats u :End of /STATS report
 */

/** @brief m_numeric351
 *
 *  process numeric 351
 *  RX: :irc.foo.com 351 stats.neostats.net Unreal3.2. irc.foo.com :FinWXOoZ [*=2303]
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void m_numeric351( char *origin, char **argv, int argc, int srv )
{
	Client *s;

	s = FindServer( origin );
	if( s ) {
		strlcpy( s->version, argv[1], MAXHOST );
	}
}

/** @brief m_numeric242
 *
 *  process numeric 242
 *  RX: :irc.foo.com 242 NeoStats :Server Up 6 days, 23:52:55
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void m_numeric242( char *origin, char **argv, int argc, int srv )
{
	Client *s;

	s = FindServer( origin );
	if( s ) {
		/* Convert "Server Up 6 days, 23:52:55" to seconds*/
		char *ptr;
		time_t secs;

		/* Server Up 6 days, 23:52:55 */
		strtok( argv[argc-1], " " );
		/* Up 6 days, 23:52:55 */
		strtok( NULL, " " );
		/* 6 days, 23:52:55 */
		ptr = strtok( NULL, " " );
		secs = atoi( ptr ) * 86400;
		/* days, 23:52:55 */
		strtok( NULL, " " );
		/* , 23:52:55 */
		ptr = strtok( NULL, "" );
		/* 23:52:55 */
		ptr = strtok( ptr , ":" );
		secs += atoi( ptr )*3600;
		/* 52:55 */
		ptr = strtok( NULL, ":" );
		secs += atoi( ptr )*60;
		/* 55 */
		ptr = strtok( NULL, "" );
		secs += atoi( ptr );

		s->server->uptime = secs;
	}
}

