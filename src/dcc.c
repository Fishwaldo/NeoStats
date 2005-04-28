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
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "modules.h"
#include "commands.h"
#include "services.h"

typedef int( *dcc_cmd_handler )( CmdParams* cmdparams );

typedef struct dcc_cmd {
	const char* cmd;
	dcc_cmd_handler req_handler;
} dcc_cmd;

static list_t *dcclist;
static int dccoutput = 0;

static int dcc_req_send( CmdParams* cmdparams );
static int dcc_req_chat( CmdParams* cmdparams );
static int DCCChatConnect(Client *dcc, int port );
static int dcc_parse(void *arg, void *line, size_t );
static int dcc_error(int what, void *arg );
static void DelDCCClient(Client *dcc );
static int dcc_write(Client *dcc, char *buf );

static dcc_cmd dcc_cmds[]= 
{
	{"SEND", dcc_req_send},
	{"CHAT", dcc_req_chat},
	{NULL},
};

static void DCCChatDisconnect(Client *dcc )
{
	DelSock(dcc->sock );
}

static void DCCGotAddr(void *data, adns_answer *a )
{
	Client *u =( Client * )data;
	if( a && a->nrrs > 0 && u && a->status == adns_s_ok ) {
		u->ip.s_addr = a->rrs.addr->addr.inet.sin_addr.s_addr;
		if( u->ip.s_addr > 0 ) {
			DCCChatConnect(u, u->port );
			return;
		}
	}
	/* if we get here, there was something wrong */
	nlog( LOG_WARNING, "DCC: Unable to connect to %s.%d: Unknown hostname", u->user->hostname, u->port );
	DelDCCClient(u );
	return;
}

static int DCCChatStart(Client *dcc, int port )
{
	dcc->port = port;
	if( dcc->ip.s_addr > 0 ) {
		/* we have a valid IP address for this user, so just go and kick off the connection straight away */
		return DCCChatConnect(dcc, port );
	} else {
		/* we don't have a valid IP address, kick off a DNS lookup */
		dns_lookup(dcc->user->hostname, adns_r_addr, DCCGotAddr,( void * )dcc );
	}
	return NS_SUCCESS;
}

static int DCCChatConnect(Client *dcc, int port ) 
{
	OS_SOCKET socketfd;
	char tmpname[BUFSIZE];

	if( (socketfd = sock_connect(SOCK_STREAM, dcc->ip, port ) ) == NS_FAILURE ) {
		nlog(LOG_WARNING, "Error Connecting to DCC Host %s(%s:%d )", dcc->user->hostname, inet_ntoa(dcc->ip ), port );
		DelDCCClient(dcc );
		return NS_FAILURE;
	}			
	/* ok, now add it as a linebuffered protocol */
	ircsnprintf(tmpname, BUFSIZE, "DCC-%s", dcc->name );
	if( (dcc->sock = add_linemode_socket(tmpname, socketfd, dcc_parse, dcc_error,( void* )dcc ) ) == NULL ) {
		nlog(LOG_WARNING, "Can't add a Linemode Socket for DCC %s", dcc->user->hostname );
		os_sock_close(socketfd );
		DelDCCClient(dcc );
		return NS_FAILURE;
	}

	dcc->fd = socketfd;
	dcc->port = port;
	return NS_SUCCESS;
}

static int 
dcc_partyline( Client *dcc, char *line ) {
	Client *todcc;
	lnode_t *dccnode;
	char tmpbuf[BUFSIZE];
 
 	ircsnprintf(tmpbuf, BUFSIZE, "\2%s\2: %s", dcc->name, line );
	dccnode = list_first( dcclist );
	while( dccnode ) {
		todcc =( Client * )lnode_get(dccnode );
		dcc_write(todcc, tmpbuf );
		dccnode = list_next(dcclist, dccnode );
	}
	irc_chanalert(ns_botptr, tmpbuf );
	return NS_SUCCESS;
}
static int
dcc_parse(void *arg, void *rline, size_t len )
{
	char buf[BUFSIZE];
	char *cmd;
	char *line =( char * )rline;
	Client *dcc =( Client * )arg;
	CmdParams *cmdparams;

	strcpy(buf, line );
	dlog(DEBUG1, "DCCRX: %s", line );
	if(buf[0] == '.' )
	{
		cmd = strchr(buf, ' ' );
		if( !cmd ) {
	         	dcc_write(dcc, "Error, You must specify a command to execute" );
	         	return NS_SUCCESS;
		}
   		*cmd = 0;
   		cmd++;
		cmdparams =( CmdParams* ) ns_calloc( sizeof(CmdParams ) );
		cmdparams->source = dcc;
		if( cmdparams->source ) {
			cmdparams->target = FindUser( buf+1 );
			if( cmdparams->target ) {
				cmdparams->bot = cmdparams->target->user->bot;
			} else {
				dcc_write(dcc, "Use .<botname> to send a command to a NeoStats Bot" );
				dcc_write(dcc, "Otherwise, jsut type test without a leading . to send to the DCC" );
				dcc_write(dcc, "partyline" );
				return NS_SUCCESS;
			}
			if( cmdparams->bot->flags & BOT_FLAG_SERVICEBOT ) 
			{
				cmdparams->param = cmd;
				run_bot_cmd( cmdparams, 0 );
				return NS_SUCCESS;
			} 
		}
		ns_free( cmdparams );
		return NS_SUCCESS;
	}
	dcc_partyline(dcc, line );
	return NS_SUCCESS;
}

int dcc_write(Client *dcc, char *buf )
{
	static char dcc_buf[BUFSIZE];

	dccoutput = 0;
	dlog(DEBUG1, "DCCTX: %s", buf );
	strlcpy(dcc_buf, buf, BUFSIZE );
	strlcat(dcc_buf, "\n", BUFSIZE );
	if( send_to_sock(dcc->sock, dcc_buf, strlen(dcc_buf ) ) == NS_FAILURE ) {
		nlog(LOG_WARNING, "Got a write error when attempting to write %d", errno );
		DelDCCClient(dcc );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

void dcc_send_msg(const Client* dcc, char * buf )
{
	dcc_write((Client * )dcc, buf );
}

int dcc_error(int sock_no, void *name )
{
	Sock *sock =( Sock * )name;
	if( sock->data ) {
		DelDCCClient(sock->data );
	} else {
		nlog(LOG_WARNING, "Problem, Sock->data is NULL, therefore we can't delete DCCClient!" );
	}
	DelSock(sock );
	return NS_SUCCESS;
}

int InitDCC(void )
{
	dcclist = list_create(-1 );
	return NS_SUCCESS;
}

void FiniDCC(void )
{
	Client *dcc;
	lnode_t *dccnode;
 
	dccnode = list_first( dcclist );
	while( dccnode ) {
		dcc =( Client * )lnode_get(dccnode );
		DCCChatDisconnect(dcc );
		ns_free( dcc );
		dccnode = list_next(dcclist, dccnode );
	}
	list_destroy_nodes( dcclist );
}

Client *AddDCCClient(CmdParams *cmdparams )
{
	Client *dcc;

	dcc = ns_calloc(sizeof(Client ) );
	if( dcc ) 
	{
		os_memcpy(dcc, cmdparams->source, sizeof(Client ) );
		lnode_create_append( dcclist, dcc );
		dcc->flags = CLIENT_FLAG_DCC;
		return dcc;
	}
	return NULL;
}

static void DelDCCClient(Client *dcc )
{
	lnode_t *dccnode;

	dccnode = lnode_find( dcclist, dcc->name, comparef );
	if( dccnode ) {
		lnode_destroy( dccnode );
		ns_free( dcc );
	}
}

int dcc_req( CmdParams* cmdparams )
{
	dcc_cmd* cmd;
	int len;
    
	cmd = dcc_cmds;
	while( cmd->cmd ) {	
		len = strlen( cmd->cmd );
		if( ircstrncasecmp( cmd->cmd, cmdparams->param, len  ) == 0 )
		{
			cmdparams->param +=( len + 1 );		
			if( cmd->req_handler ) {
				cmd->req_handler( cmdparams );
			}
			return NS_SUCCESS;
		}
		cmd++;
	}
	return NS_SUCCESS;
}

static int dcc_req_send( CmdParams* cmdparams )
{
	dlog( DEBUG5, "DCC SEND request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_DCCSEND, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/* RX: :Mark ! neostats :\1DCC CHAT chat 2130706433 1028\1 */
static int dcc_req_chat( CmdParams* cmdparams )
{
	int userlevel;
	Client *dcc;
	char **av;
	int ac;

	dlog( DEBUG5, "DCC CHAT request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	userlevel = UserLevel(cmdparams->source ); 
	if( userlevel < NS_ULEVEL_ROOT ) {
		dlog( DEBUG5, "Dropping DCC CHAT request from unauthorised user %s", cmdparams->source->name );
		return NS_FAILURE;
	}
	ac = split_buf( cmdparams->param, &av, 0 );
	if( ac == 3 )
	{
		dcc = AddDCCClient(cmdparams );
		if( DCCChatStart(dcc, atoi( av[2] ) ) != NS_SUCCESS ) 
		{
			DelDCCClient(dcc );
		}
	}
	ns_free( av );
	return NS_SUCCESS;
}
