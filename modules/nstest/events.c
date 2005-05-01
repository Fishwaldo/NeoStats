/* NeoStats - IRC Statistical Services 
** Copyright (c) 2005 Mark Hetherington
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
#include "main.h"

static int ns_event_moduleload( CmdParams *cmdparams );
static int ns_event_moduleunload( CmdParams *cmdparams );
static int ns_event_server( CmdParams *cmdparams );
static int ns_event_squit( CmdParams *cmdparams );
static int ns_event_ping( CmdParams *cmdparams );
static int ns_event_pong( CmdParams *cmdparams );
static int ns_event_signon( CmdParams *cmdparams );
static int ns_event_quit( CmdParams *cmdparams );
static int ns_event_nickip( CmdParams *cmdparams );
static int ns_event_kill( CmdParams *cmdparams );
static int ns_event_localkill( CmdParams *cmdparams );
static int ns_event_globalkill( CmdParams *cmdparams );
static int ns_event_serverkill( CmdParams *cmdparams );
static int ns_event_botkill( CmdParams *cmdparams );
static int ns_event_nick( CmdParams *cmdparams );
static int ns_event_away( CmdParams *cmdparams );
static int ns_event_umode( CmdParams *cmdparams );
static int ns_event_smode( CmdParams *cmdparams );
static int ns_event_newchan( CmdParams *cmdparams );
static int ns_event_delchan( CmdParams *cmdparams );
static int ns_event_join( CmdParams *cmdparams );
static int ns_event_part( CmdParams *cmdparams );
static int ns_event_partbot( CmdParams *cmdparams );
static int ns_event_emptychan( CmdParams *cmdparams );
static int ns_event_kick( CmdParams *cmdparams );
static int ns_event_kickbot( CmdParams *cmdparams );
static int ns_event_topic( CmdParams *cmdparams );
static int ns_event_cmode( CmdParams *cmdparams );
static int ns_event_private( CmdParams *cmdparams );
static int ns_event_notice( CmdParams *cmdparams );
static int ns_event_cprivate( CmdParams *cmdparams );
static int ns_event_cnotice( CmdParams *cmdparams );
static int ns_event_globops( CmdParams *cmdparams );
static int ns_event_chatops( CmdParams *cmdparams );
static int ns_event_wallops( CmdParams *cmdparams );
static int ns_event_ctcpversionrpl( CmdParams *cmdparams );
static int ns_event_ctcpversionreq( CmdParams *cmdparams );
static int ns_event_ctcpfingerrpl( CmdParams *cmdparams );
static int ns_event_ctcpfingerreq( CmdParams *cmdparams );
static int ns_event_ctcpactionreq( CmdParams *cmdparams );
static int ns_event_ctcptimerpl( CmdParams *cmdparams );
static int ns_event_ctcptimereq( CmdParams *cmdparams );
static int ns_event_ctcppingrpl( CmdParams *cmdparams );
static int ns_event_ctcppingreq( CmdParams *cmdparams );
static int ns_event_dccsend( CmdParams *cmdparams );
static int ns_event_dccchat( CmdParams *cmdparams );
static int ns_event_dccchatmsg( CmdParams *cmdparams );
static int ns_event_addban( CmdParams *cmdparams );
static int ns_event_delban( CmdParams *cmdparams );

/** Module Events */
ModuleEvent module_events[] = 
{
	{EVENT_MODULELOAD,	ns_event_moduleload},
	{EVENT_MODULEUNLOAD,	ns_event_moduleunload},
	{EVENT_SERVER,	ns_event_server},
	{EVENT_SQUIT,	ns_event_squit},
	{EVENT_PING,	ns_event_ping},
	{EVENT_PONG,	ns_event_pong},
	{EVENT_SIGNON,	ns_event_signon},
	{EVENT_QUIT,	ns_event_quit},
	{EVENT_NICKIP,	ns_event_nickip},
	{EVENT_KILL,	ns_event_kill},
	{EVENT_LOCALKILL,	ns_event_localkill},
	{EVENT_GLOBALKILL,	ns_event_globalkill},
	{EVENT_SERVERKILL,	ns_event_serverkill},
	{EVENT_BOTKILL,	ns_event_botkill},
	{EVENT_NICK,	ns_event_nick},
	{EVENT_AWAY,	ns_event_away},
	{EVENT_UMODE,	ns_event_umode},
	{EVENT_SMODE,	ns_event_smode},
	{EVENT_NEWCHAN,	ns_event_newchan},
	{EVENT_DELCHAN,	ns_event_delchan},
	{EVENT_JOIN,	ns_event_join},
	{EVENT_PART,	ns_event_part},
	{EVENT_PARTBOT,	ns_event_partbot},
	{EVENT_EMPTYCHAN,	ns_event_emptychan},
	{EVENT_KICK,	ns_event_kick},
	{EVENT_KICKBOT,	ns_event_kickbot},
	{EVENT_TOPIC,	ns_event_topic},
	{EVENT_CMODE,	ns_event_cmode},
	{EVENT_PRIVATE,	ns_event_private},
	{EVENT_NOTICE,	ns_event_notice},
	{EVENT_CPRIVATE,	ns_event_cprivate},
	{EVENT_CNOTICE,	ns_event_cnotice},
	{EVENT_GLOBOPS,	ns_event_globops},
	{EVENT_CHATOPS,	ns_event_chatops},
	{EVENT_WALLOPS,	ns_event_wallops},
	{EVENT_CTCPVERSIONRPL,	ns_event_ctcpversionrpl},
	{EVENT_CTCPVERSIONREQ,	ns_event_ctcpversionreq},
	{EVENT_CTCPFINGERRPL,	ns_event_ctcpfingerrpl},
	{EVENT_CTCPFINGERREQ,	ns_event_ctcpfingerreq},
	{EVENT_CTCPACTIONREQ,	ns_event_ctcpactionreq},
	{EVENT_CTCPTIMERPL,	ns_event_ctcptimerpl},
	{EVENT_CTCPTIMEREQ,	ns_event_ctcptimereq},
	{EVENT_CTCPPINGRPL,	ns_event_ctcppingrpl},
	{EVENT_CTCPPINGREQ,	ns_event_ctcppingreq},
	{EVENT_DCCSEND,	ns_event_dccsend},
	{EVENT_DCCCHAT,	ns_event_dccchat},
	{EVENT_DCCCHATMSG,	ns_event_dccchatmsg},
	{EVENT_ADDBAN,	ns_event_addban},
	{EVENT_DELBAN,	ns_event_delban},
	{EVENT_NULL,		NULL}
};

static int ns_event_moduleload( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_MODULELOAD %s", cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_MODULELOAD %s", cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_moduleunload( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_MODULEUNLOAD %s", cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_MODULEUNLOAD %s", cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_server( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_SERVER %s", cmdparams->source->name );
	irc_chanalert( ns_bot, "EVENT_SERVER %s", cmdparams->source->name );
	return NS_SUCCESS;
}

static int ns_event_squit( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_SQUIT %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_SQUIT %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ping( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_PING %s", cmdparams->source->name );
	irc_chanalert( ns_bot, "EVENT_PING %s", cmdparams->source->name );
	return NS_SUCCESS;
}

static int ns_event_pong( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_PONG %s", cmdparams->source->name );
	irc_chanalert( ns_bot, "EVENT_PONG %s", cmdparams->source->name );
	return NS_SUCCESS;
}

static int ns_event_signon( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_SIGNON %s", cmdparams->source->name );
	irc_chanalert( ns_bot, "EVENT_SIGNON %s", cmdparams->source->name );
	return NS_SUCCESS;
}

static int ns_event_quit( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_QUIT %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_QUIT %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_nickip( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_NICKIP %s", cmdparams->source->name );
	irc_chanalert( ns_bot, "EVENT_NICKIP %s", cmdparams->source->name );
	return NS_SUCCESS;
}

static int ns_event_kill( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_KILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_KILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_localkill( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_LOCALKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_LOCALKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_globalkill( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_GLOBALKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_GLOBALKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_serverkill( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_SERVERKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_SERVERKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_botkill( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_BOTKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_BOTKILL %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_nick( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_NICK %s %s", cmdparams->param , cmdparams->source->name);
	irc_chanalert( ns_bot, "EVENT_NICK %s %s", cmdparams->param , cmdparams->source->name);
	return NS_SUCCESS;
}

static int ns_event_away( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_AWAY %s", cmdparams->source->name );
	irc_chanalert( ns_bot, "EVENT_AWAY %s", cmdparams->source->name );
	return NS_SUCCESS;
}

static int ns_event_umode( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_UMODE %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_UMODE %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_smode( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_SMODE %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_SMODE %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_newchan( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_NEWCHAN %s", cmdparams->channel->name );
	irc_chanalert( ns_bot, "EVENT_NEWCHAN %s", cmdparams->channel->name );
	return NS_SUCCESS;
}

static int ns_event_delchan( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_DELCHAN %s", cmdparams->channel->name );
	irc_chanalert( ns_bot, "EVENT_DELCHAN %s", cmdparams->channel->name );
	return NS_SUCCESS;
}

static int ns_event_join( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_JOIN %s %s", cmdparams->source->name, cmdparams->channel->name );
	irc_chanalert( ns_bot, "EVENT_JOIN %s %s", cmdparams->source->name, cmdparams->channel->name );
	return NS_SUCCESS;
}

static int ns_event_part( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_PART %s %s %s", cmdparams->source->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_PART %s %s %s", cmdparams->source->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_partbot( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_PARTBOT %s %s %s", cmdparams->source->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_PARTBOT %s %s %s", cmdparams->source->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_emptychan( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_EMPTYCHAN %s %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_EMPTYCHAN %s %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_kick( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_KICK %s %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_KICK %s %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_kickbot( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_KICKBOT %s %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_KICKBOT %s %s %s %s", cmdparams->source->name, cmdparams->target->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_topic( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_TOPIC %s", cmdparams->channel->name );
	irc_chanalert( ns_bot, "EVENT_TOPIC %s", cmdparams->channel->name );
	return NS_SUCCESS;
}

static int ns_event_cmode( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CMODE %s", cmdparams->channel->name );
	irc_chanalert( ns_bot, "EVENT_CMODE %s", cmdparams->channel->name );
	return NS_SUCCESS;
}

static int ns_event_private( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_PRIVATE %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_PRIVATE %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_notice( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_NOTICE %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_NOTICE %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_cprivate( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CPRIVATE %s %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_CPRIVATE %s %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_cnotice( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CNOTICE %s %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->channel->name, cmdparams->param  );
	irc_chanalert( ns_bot, "EVENT_CNOTICE %s %s %s %s", cmdparams->source->name, cmdparams->bot->u->name, cmdparams->channel->name, cmdparams->param  );
	return NS_SUCCESS;
}

static int ns_event_globops( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_GLOBOPS %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_GLOBOPS %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_chatops( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CHATOPS %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CHATOPS %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_wallops( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_WALLOPS %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_WALLOPS %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcpversionrpl( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPVERSIONRPL %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPVERSIONRPL %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcpversionreq( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPVERSIONREQ %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPVERSIONREQ %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcpfingerrpl( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPFINGERRPL %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPFINGERRPL %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcpfingerreq( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPFINGERREQ %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPFINGERREQ %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcpactionreq( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPACTIONREQ %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPACTIONREQ %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcptimerpl( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPTIMERPL %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPTIMERPL %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcptimereq( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPTIMEREQ %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPTIMEREQ %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcppingrpl( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPPINGRPL %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPPINGRPL %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_ctcppingreq( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_CTCPPINGREQ %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_CTCPPINGREQ %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_dccsend( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_DCCSEND %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_DCCSEND %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_dccchat( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_DCCCHAT %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_DCCCHAT %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_dccchatmsg( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_DCCCHATMSG %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_DCCCHATMSG %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_addban( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_ADDBAN %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_ADDBAN %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

static int ns_event_delban( CmdParams *cmdparams )
{
	dlog( DEBUG1, "EVENT_DELBAN %s %s", cmdparams->source->name, cmdparams->param );
	irc_chanalert( ns_bot, "EVENT_DELBAN %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}
