/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "modules.h"
#include "commands.h"

typedef int (*dcc_cmd_handler) (CmdParams* cmdparams);

typedef struct dcc_cmd {
	const char* cmd;
	dcc_cmd_handler req_handler;
} dcc_cmd;

static list_t *dcclist;
static int dccoutput = 0;

static int dcc_req_send (CmdParams* cmdparams);
static int dcc_req_chat (CmdParams* cmdparams);

static dcc_cmd dcc_cmds[]= {
	{"SEND", dcc_req_send},
	{"CHAT", dcc_req_chat},
	{NULL},
};

static void DCCChatDisconnect(Client *dcc)
{
	os_sock_close (dcc->fd);
}

static int DCCChatConnect(Client *dcc, int port)
{
	struct sockaddr_in dccaddr;
	struct hostent *target_host;
	OS_SOCKET socketfd;
	int flags = 1;
	struct in_addr ip;
	int err = 0;

	os_memset((void *) &dccaddr, 0, sizeof(struct sockaddr_in));
	ip.s_addr = inet_addr(dcc->user->hostname);
	if (ip.s_addr != INADDR_NONE)
	{
		target_host = NULL;
	}
	else
	{
		target_host = gethostbyname(dcc->user->hostname);
		if (target_host)
			memcpy(&ip.s_addr, target_host->h_addr_list[0], (size_t) target_host->h_length);
	}
	if (target_host)
	{
		dccaddr.sin_family = target_host->h_addrtype;
		dccaddr.sin_addr.s_addr = ip.s_addr;
	}
	else
	{
		if (ip.s_addr == INADDR_NONE)
		{
			nlog (LOG_WARNING, "DCC: Unable to connect to %s.%d: Unknown hostname", dcc->user->hostname, port);
			return NS_FAILURE;
		}
		dccaddr.sin_family = AF_INET;
		dccaddr.sin_addr.s_addr = ip.s_addr;
	}
	dccaddr.sin_port = (unsigned short) htons((unsigned short) port);
	dlog(DEBUG1, "DCC: Connecting to %s.%d", inet_ntoa(dccaddr.sin_addr), port);
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		nlog (LOG_WARNING, "DCC: Unable to open stream socket: %s", strerror(errno));
		return NS_FAILURE;
	}
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags));
	me.lsa.sin_family = AF_INET;
	me.lsa.sin_addr.s_addr = INADDR_ANY;
	if (bind (socketfd, (struct sockaddr *) &me.lsa, sizeof(me.lsa)) < 0)
	{
		nlog (LOG_WARNING, "bind(): Warning, Couldn't bind to IP address %s", strerror (errno));
		os_sock_close (socketfd);
		return NS_FAILURE;
	}
#if 0
	if ((err = os_sock_set_nonblocking (socketfd)) < 0) {
		nlog (LOG_CRITICAL, "can't set socket non-blocking: %s", strerror (err));
		return NS_FAILURE;
	}
#endif
	if (connect(socketfd, (struct sockaddr *) &dccaddr, sizeof(dccaddr))==-1)
	{
		switch (errno) {
		case EINPROGRESS:
			break;
		default:
			nlog (LOG_WARNING, "Error connecting to dcc host %s.%d: %s", inet_ntoa(dccaddr.sin_addr), port, strerror(errno));
			os_sock_close (socketfd);
			return NS_FAILURE;
		}
	}
	if (socketfd == -1)
	{
		return NS_FAILURE;
	}
	dcc->fd = socketfd;
	dcc->port = port;
	return NS_SUCCESS;
}

void dcc_parse(Client *dcc, char *line)
{
	char buf[BUFSIZE];
	char *cmd;
	CmdParams *cmdparams;

	strcpy(buf, line);
	if(buf[0] == '.')
	{
		cmd = strchr(buf, ' ');
		if(!cmd)
			return;
		*cmd = 0;
		cmd++;
		cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
		cmdparams->source = dcc;
		if (cmdparams->source) {
			cmdparams->target = find_user (buf+1);
			if (cmdparams->target) {
				cmdparams->bot = cmdparams->target->user->bot;
				if (!cmdparams->bot) {
					return;
				}
			}
			if (cmdparams->bot->flags & BOT_FLAG_SERVICEBOT) 
			{
				cmdparams->param = cmd;
				run_bot_cmd (cmdparams, 0);
			}
		}
		ns_free (cmdparams);
	}
}

int dcc_read(Client *dcc)
{
	register int i, j;
	char c;
	char buf[BUFSIZE];

	for (j = 0; j < BUFSIZE; j++) {
		i = os_sock_read (dcc->fd, &c, 1);
		if (i >= 0) {
			buf[j] = c;
			if ((c == '\n') || (c == '\r')) {
				strip (buf);
				dlog(DEBUG1, "DCCRX: %s", buf);
				dcc_parse(dcc, buf);
				break;
			}
		} else {
			nlog (LOG_WARNING, "read returned an Error");
			return -1;
		}
	}
	return NS_SUCCESS;
}

int dcc_write(Client *dcc, char *buf)
{
	static char dcc_buf[BUFSIZE];
	int      ret;        /* write() return value */

	dccoutput = 0;
	dlog(DEBUG1, "DCCTX: %s", buf);
	strlcpy(dcc_buf, buf, BUFSIZE);
	strlcat(dcc_buf, "\n", BUFSIZE);
	ret = os_sock_write (dcc->fd, dcc_buf, strlen(dcc_buf));
	if (ret < 0)
	{
		nlog(LOG_WARNING, "Got a write error when attempting to write %d", errno);
		os_sock_close (dcc->fd);
		return -1;
	}
	return NS_SUCCESS;
}

void dcc_send_msg(Client* dcc, char * buf)
{
	dcc_write(dcc, buf);
}

int dcc_error(int sock_no, char *name)
{
	return NS_SUCCESS;
}

void dcc_hook_1 (fd_set *read_fd_set, fd_set *write_fd_set)
{
	Client *dcc;
	lnode_t *dccnode;

	dccnode = list_first (dcclist);
	while (dccnode) {
		dcc = (Client *)lnode_get(dccnode);
		if (dccoutput) {
			FD_SET(dcc->fd, write_fd_set);
		} else {
			FD_SET(dcc->fd, read_fd_set);			
		}
		dccnode = list_next(dcclist, dccnode);
	}
}

void dcc_hook_2 (fd_set *read_fd_set, fd_set *write_fd_set)
{
	Client *dcc;
	lnode_t *dccnode;

	dccnode = list_first (dcclist);
	while (dccnode) {
		dcc = (Client *)lnode_get(dccnode);
		if (FD_ISSET(dcc->fd, read_fd_set)) {
			dcc_read(dcc);
		} else if (FD_ISSET(dcc->fd, write_fd_set)) {
			dcc_write(dcc, "TEST");
		}
		dccnode = list_next(dcclist, dccnode);
	}
}

int InitDCC(void)
{
	dcclist = list_create(-1);
	return NS_SUCCESS;
}

void FiniDCC(void)
{
	Client *dcc;
	lnode_t *dccnode;
 
	dccnode = list_first (dcclist);
	while (dccnode) {
		dcc = (Client *)lnode_get(dccnode);
		lnode_destroy (dccnode);
		DCCChatDisconnect(dcc);
		ns_free (dcc);
		dccnode = list_next(dcclist, dccnode);
	}
	list_destroy (dcclist);
}

Client *AddDCCClient(CmdParams *cmdparams)
{
	Client *dcc;

	dcc = ns_calloc(sizeof(Client));
	if (dcc) 
	{
		os_memcpy(dcc, cmdparams->source, sizeof(Client));
		lnode_create_append (dcclist, dcc);
		dcc->flags |= CLIENT_FLAG_DCC;
		return dcc;
	}
	return NULL;
}

void DelDCCClient(Client *dcc)
{
	lnode_t *dccnode;

	dccnode = lnode_find (dcclist, dcc->name, comparef);
	if (dccnode) {
		lnode_destroy (dccnode);
		ns_free (dcc);
	}
}

int dcc_req (CmdParams* cmdparams)
{
	dcc_cmd* cmd;
	int len;
    
	cmd = dcc_cmds;
	while (cmd->cmd) {	
		len = strlen (cmd->cmd);
		if ( ircstrncasecmp (cmd->cmd, cmdparams->param, len ) == 0)
		{
			cmdparams->param += (len + 1);		
			if (cmd->req_handler) {
				cmd->req_handler (cmdparams);
			}
			return NS_SUCCESS;
		}
		cmd++;
	}
	return NS_SUCCESS;
}

static int dcc_req_send (CmdParams* cmdparams)
{
	dlog (DEBUG5, "DCC SEND request from %s to %s", cmdparams->source->name, cmdparams->bot->name);
	SendModuleEvent (EVENT_DCCSEND, cmdparams, cmdparams->bot->moduleptr);
	return NS_SUCCESS;
}

/* RX: :Mark ! neostats :\1DCC CHAT chat 2130706433 1028\1 */
static int dcc_req_chat (CmdParams* cmdparams)
{
	int userlevel;
	Client *dcc;
	char **av;
	int ac;

	dlog (DEBUG5, "DCC CHAT request from %s to %s", cmdparams->source->name, cmdparams->bot->name);
	userlevel = UserLevel(cmdparams->source); 
	if (userlevel < NS_ULEVEL_ROOT) {
		dlog (DEBUG5, "Dropping DCC CHAT request from unauthorised user %s", cmdparams->source->name);
		return NS_FAILURE;
	}
	ac = split_buf (cmdparams->param, &av, 0);
	if (ac == 3)
	{
		dcc = AddDCCClient(cmdparams);
		if (DCCChatConnect(dcc, atoi (av[2])) != NS_SUCCESS) 
		{
			DelDCCClient(dcc);
		}
	}
	ns_free (av);
	return NS_SUCCESS;
}
