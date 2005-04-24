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
#include "protocol.h"

const char MSG_PRIVATE[] = "PRIVMSG";
const char MSG_WHO[] = "WHO";
const char MSG_WHOIS[] = "WHOIS";
const char MSG_WHOWAS[] = "WHOWAS";
const char MSG_USER[] = "USER";
const char MSG_NICK[] = "NICK";
const char MSG_SERVER[] = "SERVER";
const char MSG_LIST[] = "LIST";
const char MSG_TOPIC[] = "TOPIC";
const char MSG_INVITE[] = "INVITE";
const char MSG_VERSION[] = "VERSION";
const char MSG_QUIT[] = "QUIT";
const char MSG_SQUIT[] = "SQUIT";
const char MSG_KILL[] = "KILL";
const char MSG_INFO[] = "INFO";
const char MSG_LINKS[] = "LINKS";
const char MSG_STATS[] = "STATS";
const char MSG_USERS[] = "USERS";
const char MSG_HELP[] = "HELP";
const char MSG_ERROR[] = "ERROR";
const char MSG_AWAY[] = "AWAY";
const char MSG_CONNECT[] = "CONNECT";
const char MSG_PING[] = "PING";
const char MSG_PONG[] = "PONG";
const char MSG_OPER[] = "OPER";
const char MSG_PASS[] = "PASS";
const char MSG_WALLOPS[] = "WALLOPS";
const char MSG_TIME[] = "TIME";
const char MSG_NAMES[] = "NAMES";
const char MSG_ADMIN[] = "ADMIN";
const char MSG_TRACE[] = "TRACE";
const char MSG_LTRACE[] = "LTRACE";
const char MSG_NOTICE[] = "NOTICE";
const char MSG_JOIN[] = "JOIN";
const char MSG_PART[] = "PART";
const char MSG_LUSERS[] = "LUSERS";
const char MSG_MOTD[] = "MOTD";
const char MSG_MODE[] = "MODE";
const char MSG_KICK[] = "KICK";
const char MSG_USERHOST[] = "USERHOST";
const char MSG_ISON[] = "ISON";
const char MSG_REHASH[] = "REHASH";
const char MSG_RESTART[] = "RESTART";
const char MSG_CLOSE[] = "CLOSE";
const char MSG_SVINFO[] = "SVINFO";
const char MSG_SJOIN[] = "SJOIN";
const char MSG_CAPAB[] = "CAPAB";
const char MSG_DIE[] = "DIE";
const char MSG_HASH[] = "HASH";
const char MSG_DNS[] = "DNS";
const char MSG_OPERWALL[] = "OPERWALL";
const char MSG_KLINE[] = "KLINE";
const char MSG_UNKLINE[] = "UNKLINE";
const char MSG_DLINE[] = "DLINE";
const char MSG_UNDLINE[] = "UNDLINE";
const char MSG_HTM[] = "HTM";
const char MSG_SET[] = "SET";
const char MSG_GLINE[] = "GLINE";
const char MSG_UNGLINE[] = "UNGLINE";
const char MSG_LOCOPS[] = "LOCOPS";
const char MSG_LWALLOPS[] = "LWALLOPS";
const char MSG_KNOCK[] = "KNOCK";
const char MSG_MAP[] = "MAP";
const char MSG_ETRACE[] = "ETRACE";
const char MSG_SINFO[] = "SINFO";
const char MSG_TESTLINE[] = "TESTLINE";
const char MSG_OPERSPY[] = "OPERSPY";
const char MSG_ENCAP[] = "ENCAP";
const char MSG_XLINE[] = "XLINE";
const char MSG_UNXLINE[] = "UNXLINE";
const char MSG_RESV[] = "RESV";
const char MSG_UNRESV[] = "UNRESV";

/* Umodes */
 
/* Cmodes */

/* static void m_server( char *origin, char **argv, int argc, int srv ); */
/* static void m_nick( char *origin, char **argv, int argc, int srv ); */

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	0,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	63,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	9,
	/* Max real name length */
	50,
	/* Max channel name length */
	200,
	/* Max topic length */
	120,
	/* Default operator modes for NeoStats service bots */
	"+o",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{0, 0},
};
