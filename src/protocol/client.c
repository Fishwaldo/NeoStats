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

#include "client.h"
#include "neostats.h"
#include "bots.h"
#include "users.h"
#include "ircd.h"

static void m_private (char* origin, char **av, int ac, int cmdptr);
static void m_notice (char* origin, char **av, int ac, int cmdptr);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_topic (char *origin, char **argv, int argc, int srv);
static void m_kick (char *origin, char **argv, int argc, int srv);
static void m_join (char *origin, char **argv, int argc, int srv);
static void m_part (char *origin, char **argv, int argc, int srv);
static void m_vhost (char *origin, char **argv, int argc, int srv);
static void m_emotd (char *origin, char **argv, int argc, int srv);

/* buffer sizes */
const int proto_maxhost		= (128 + 1);
const int proto_maxpass		= (32 + 1);
const int proto_maxnick		= (30 + 1);
const int proto_maxuser		= (10 + 1);
const int proto_maxrealname	= (50 + 1);
const int proto_chanlen		= (32 + 1);
const int proto_topiclen	= (307 + 1);

ProtocolInfo protocol_info = {
	/* Protocol options required by this IRCd */
	PROTOCOL_CLIENTMODE,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	"+oSq",
	"+o",
};

ircd_cmd cmd_list[] = {
	/*Message	Token	Function	usage */
	{MSG_PRIVATE, 0, m_private, 0},
	{MSG_NOTICE, 0, m_notice, 0},
	{"376", 0, m_emotd, 0},
	{MSG_SETHOST, 0, m_vhost, 0},
	{MSG_QUIT, 0, _m_quit, 0},
	{MSG_MODE, 0, _m_mode, 0},
	{MSG_KILL, 0, _m_kill, 0},
	{MSG_PONG, 0, _m_pong, 0},
	{MSG_AWAY, 0, _m_away, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, m_topic, 0},
	{MSG_KICK, 0, m_kick, 0},
	{MSG_JOIN, 0, m_join, 0},
	{MSG_PART, 0, m_part, 0},
	{MSG_PING, 0, _m_ping, 0},
	{MSG_GLOBOPS,	0, _m_globops, 0},
	{MSG_WALLOPS,	0, _m_wallops, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{MSG_ERROR, 0, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'v', CUMODE_VOICE, 0, '+'},
	{'h', CUMODE_HALFOP, 0, '%'},
	{'o', CUMODE_CHANOP, 0, '@'},
	{'a', CUMODE_CHANPROT, 0, '*'},
	{'q', CUMODE_CHANOWNER, 0, '~'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'l', CMODE_LIMIT, MODEPARAM},
	{'p', CMODE_PRIVATE, 0},
	{'s', CMODE_SECRET, 0},
	{'m', CMODE_MODERATED, 0},
	{'n', CMODE_NOPRIVMSGS, 0},
	{'t', CMODE_TOPICLIMIT, 0},
	{'i', CMODE_INVITEONLY, 0},
	{'k', CMODE_KEY, MODEPARAM},
	{'r', CMODE_RGSTR, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'c', CMODE_NOCOLOR, 0},
	{'O', CMODE_OPERONLY, 0},
	{'A', CMODE_ADMONLY, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'Q', CMODE_NOKICKS, 0},
	{'b', CMODE_BAN, MODEPARAM},
	{'S', CMODE_STRIP, 0},
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'K', CMODE_NOKNOCK, 0},
	{'V', CMODE_NOINVITE, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
	{'M', CMODE_MODREG, 0},
	{'G', CMODE_STRIPBADWORDS, 0},
	{'C', CMODE_NOCTCP, 0},
	{'u', CMODE_AUDITORIUM, 0},
	{'z', CMODE_ONLYSECURE, 0},
	{'N', CMODE_NONICKCHANGE, 0},
	{0, 0},
};

mode_init user_umodes[] = {
	{'S', UMODE_SERVICES},
	{'N', UMODE_NETADMIN},
	{'a', UMODE_SADMIN},
	{'A', UMODE_ADMIN},
	{'C', UMODE_COADMIN},
	{'o', UMODE_OPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'i', UMODE_INVISIBLE},
	{'w', UMODE_WALLOP},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'q', UMODE_KIX},
	{'B', UMODE_BOT},
	{'d', UMODE_DEAF},
	{'R', UMODE_RGSTRONLY},
 	{'T', UMODE_NOCTCP},
	{'V', UMODE_WEBTV},
	{'p', UMODE_HIDEWHOIS},
	{'H', UMODE_HIDEOPER},
	{'G', UMODE_STRIPBADWORDS},
	{'t', UMODE_SETHOST},
	{'x', UMODE_HIDE},
	/*{'b', UMODE_CHATOP},*/
	{'W', UMODE_WHOIS},
	{'z', UMODE_SECURE},
	{'v', UMODE_VICTIM},	
	{0, 0},
};

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink)
{
	send_cmd ("%s %s", MSG_PASS, pass);
	send_cmd ("%s %s", MSG_NICK, "NeoStats");
	send_cmd ("%s %s %d %d :%s", MSG_USER, "user", (int) me.now, (int) me.now, "real name");
}

void 
send_quit (const char *source, const char *quitmsg)
{
	send_cmd (":%s %s :%s", source, MSG_QUIT, quitmsg);
}

void 
send_part (const char *source, const char *chan, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSG_PART, chan, reason);
}

void 
send_join (const char *source, const char *chan, const char *key, const unsigned long ts)
{
	send_cmd (":%s %s %s", source, MSG_JOIN, chan);
}

void 
send_cmode (const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s", MSG_NICK, nick);
}

void
send_ping (const char *from, const char *reply, const char *target)
{
	send_cmd (":%s %s %s :%s", from, MSG_PING, reply, target);
}

void 
send_umode (const char *source, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", source, MSG_MODE, target, mode);
}

void 
send_numeric (const char *from, const int numeric, const char *target, const char *buf)
{
	send_cmd (":%s %d %s :%s", from, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	send_cmd ("%s %s", MSG_PONG, reply);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", from, MSG_KILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s", oldnick, MSG_NICK, newnick);
}

void
send_swhois (const char *source, const char *target, const char *swhois)
{
	send_cmd ("%s %s :%s", MSG_SWHOIS, target, swhois);
}

void 
send_kick (const char *source, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", source, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *source, const char *buf)
{
	send_cmd (":%s %s :%s", source, MSG_WALLOPS, buf);
}

void
send_invite (const char *from, const char *target, const char *chan) 
{
	send_cmd (":%s %s %s %s", from, MSG_INVITE, target, chan);
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void 
send_akill (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s %s + G %s %s %s %lu %lu :%s", source, MSG_TKL, ident, host, setby, (ts + length), ts, reason);
}

void 
send_rakill (const char *source, const char *host, const char *ident)
{
	send_cmd (":%s %s - G %s %s %s", source, MSG_TKL, ident, host, source);
}

void
send_privmsg (const char *source, const char *target, const char *buf)
{
	send_cmd (":%s %s %s :%s", source, MSG_PRIVATE, target, buf);
}

void
send_notice (const char *source, const char *target, const char *buf)
{
	send_cmd (":%s %s %s :%s", source, MSG_NOTICE, target, buf);
}

void
send_globops (const char *source, const char *buf)
{
	send_cmd (":%s %s :%s", source, MSG_GLOBOPS, buf);
}

static void
m_vhost (char *origin, char **argv, int argc, int srv)
{
	do_vhost (origin, argv[0]);
}

static void
m_nick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (origin, argv[0], NULL);
}

/* m_topic
 *  argv[0] = channel name
 *  argv[1] = topic text
 */
/* TOPIC #channel :topic */
static void
m_topic (char *origin, char **argv, int argc, int srv)
{
	do_topic (argv[0], NULL, NULL, argv[1]);
}

/* m_kick
 *	argv[0] = channel
 *	argv[1] = client to kick
 *	argv[2] = kick comment
 */
static void
m_kick (char *origin, char **argv, int argc, int srv)
{
	do_kick (origin, argv[0], argv[1], argv[2]);
}

/* m_join
 *	argv[0] = channel
 *	argv[1] = channel password (key)
 */
static void
m_join (char *origin, char **argv, int argc, int srv)
{
	do_join (origin, argv[0], argv[1]);
}

/* m_part
 *	argv[0] = channel
 *	argv[1] = comment
 */
static void
m_part (char *origin, char **argv, int argc, int srv)
{
	do_part (origin, argv[0], argv[1]);
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */
void 
m_notice (char* origin, char **av, int ac, int cmdptr)
{
	_m_notice (origin, av, ac, cmdptr);
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */

void m_private (char* origin, char **av, int ac, int cmdptr)
{
	char *p;
	char nick[MAXNICK];
	
	strlcpy(nick, origin, MAXNICK);
	p = strchr(nick, '!');
	*p = 0;
	AddFakeUser(origin);
	_m_private (nick, av, ac, cmdptr);
	DelFakeUser(origin);
}

static void m_emotd (char *origin, char **argv, int argc, int srv)
{
	send_cmd ("%s mark mark", MSG_OPER);
	do_synch_neostats ();
}
