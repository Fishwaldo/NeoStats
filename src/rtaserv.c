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
#include "rta.h"
#include "rtaserv.h"
#include <fcntl.h>

static int rta_active = 0;
#define MAXSQLCON 5

/* sqlsrv struct for tracking connections */
typedef struct Sql_Conn {
	struct sockaddr_in cliskt;
	OS_SOCKET fd;
	long long nbytein;
	long long nbyteout;
	char response[50000];
	int responsefree;
	int cmdpos;
	int cmd[1000];
} Sql_Conn;

OS_SOCKET sqlListenSock = -1;

list_t *sqlconnections;
char rtauser[MAXUSER];
char rtapass[MAXPASS];
char rtahost[MAXHOST];
int rtaport;

static void sql_accept_conn(OS_SOCKET srvfd);
static OS_SOCKET sqllisten_on_port(int port);
static int sql_handle_ui_request(lnode_t *sqlnode);
static int sql_handle_ui_output(lnode_t *sqlnode);
int check_sql_sock();

/* this is for sqlserver logging callback */
void rtaservlog(char *logline) 
{
	dlog(DEBUG1, "rtaserv: %s", logline);
}

int InitRTAServ (void)
{
	rtaport = 8888;
	rta_init(rtaservlog);
	/* add the server hash to the sql library */
	neo_bans.address = GetBanHash();
	rta_add_table(&neo_bans);
	/* add the server hash to the sql library */
	neo_chans.address = GetChannelHash();
	rta_add_table(&neo_chans);
	/* add the server hash to the sql library */
	neo_users.address = GetUserHash();
	rta_add_table(&neo_users);
	/* add the module hash to the sql library */
	neo_modules.address = GetModuleHash();
	rta_add_table(&neo_modules);
	/* add the server hash to the sql library */
	neo_servers.address = GetServerHash();
	rta_add_table(&neo_servers);

	/* init the sql Listen Socket now as well */
	sqlconnections = list_create(MAXSQLCON);
	sqlListenSock = sqllisten_on_port(rtaport);
	if (sqlListenSock == -1) {
		nlog(LOG_CRITICAL, "Failed to Setup Sql Port. SQL not available");
		return NS_FAILURE;
	}
   	dlog (DEBUG1, "Init OK");
	return NS_SUCCESS;
}

/** @brief prepare SqlAuthentication defined in the config file
 *
 * load the Sql UserName/Password and Host if we are using SQL Server option
 *
 * @param arg the module name in this case
 * @param configtype an index of what config item is currently being processed. Ignored
 * @returns Nothing
 */

const char *rta_help_set_rtauser[] = {
	"RTAUSER ",
	NULL
};

const char *rta_help_set_rtapass[] = {
	"RTAPASS ",
	NULL
};

const char *rta_help_set_rtahost[] = {
	"RTAHOST ",
	NULL
};

const char *rta_help_set_rtaport[] = {
	"RTAPORT ",
	NULL
};

static int rta_set_rtauser_cb (CmdParams* cmdparams, SET_REASON reason)
{
	rta_change_auth(rtauser, rtapass);
	return NS_SUCCESS;
}

static int rta_set_rtapass_cb (CmdParams* cmdparams, SET_REASON reason)
{
	rta_change_auth(rtauser, rtapass);
	return NS_SUCCESS;
}

static int rta_set_rtahost_cb (CmdParams* cmdparams, SET_REASON reason)
{
	return NS_SUCCESS;
}

static int rta_set_rtaport_cb (CmdParams* cmdparams, SET_REASON reason)
{
	return NS_SUCCESS;
}

static bot_setting rta_settings[]=
{
	{"RTAUSER",	&rtauser,	SET_TYPE_STRING,	0, MAXUSER, NS_ULEVEL_ADMIN, "rtauser",	NULL,	rta_help_set_rtauser, rta_set_rtauser_cb, (void*)"user" },
	{"RTAPASS",	&rtapass,	SET_TYPE_STRING,	0, MAXPASS, NS_ULEVEL_ADMIN, "rtapass",	NULL,	rta_help_set_rtapass, rta_set_rtapass_cb, (void*)"pass" },
	{"RTAHOST",	&rtahost,	SET_TYPE_HOST,		0, MAXHOST, NS_ULEVEL_ADMIN, "rtahost",	NULL,	rta_help_set_rtahost, rta_set_rtahost_cb, (void*)"127.0.0.1" },
	{"RTAPORT",	&rtaport,	SET_TYPE_INT,		0, 0, 		NS_ULEVEL_ADMIN, "rtaport",	NULL,	rta_help_set_rtaport, rta_set_rtaport_cb, (void*)8888 },
	{NULL,		NULL,		0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/* the following functions are taken from the RTA example app shipped with the library, 
 * and modified to work with NeoStats. 
 * Credit for these apps should be given to the respective authors of the RTA library.
 * more info can be found at http://www.linuxappliancedesign.com for more
 * information
 */

/* rehash handler */
int check_sql_sock() 
{
	if (sqlListenSock < 1) {
		dlog(DEBUG1, "Rehashing SQL sock");
		sqlListenSock = sqllisten_on_port(rtaport);
		if (sqlListenSock == -1) {
			nlog(LOG_CRITICAL, "Failed to Setup Sql Port. SQL not available");
			return NS_FAILURE;
		}
	}
	return NS_SUCCESS;
}                                       

/***************************************************************
 * listen_on_port(int port): -  Open a socket to listen for
 * incoming TCP connections on the port given.  Return the file
 * descriptor if OK, and -1 on any error.  The calling routine
 * can handle any error condition.
 *
 * Input:        The interger value of the port number to bind to
 * Output:       The file descriptor of the socket
 * Effects:      none
 ***************************************************************/
OS_SOCKET
sqllisten_on_port(int port)
{
	OS_SOCKET srvfd;      /* FD for our listen server socket */
	struct sockaddr_in srvskt;
	int      adrlen;

	adrlen = sizeof(struct sockaddr_in);
	(void) memset((void *) &srvskt, 0, (size_t) adrlen);
	srvskt.sin_family = AF_INET;
	/* bind to the local IP */
	if (me.dobind) {
		srvskt.sin_addr = me.lsa.sin_addr;
	} else {
		srvskt.sin_addr.s_addr = INADDR_ANY;
	}
	srvskt.sin_port = htons(rtaport);
	if ((srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		nlog(LOG_CRITICAL, "rtaserv: Unable to get socket for port %d.", port);
		return -1;
	}
	os_sock_set_nonblocking (srvfd);
	if (bind(srvfd, (struct sockaddr *) &srvskt, adrlen) < 0)
	{
		nlog(LOG_CRITICAL, "Unable to bind to port %d", port);
		return -1;
	}
	if (listen(srvfd, 1) < 0)
	{
		nlog(LOG_CRITICAL, "Unable to listen on port %d", port);
		return -1;
	}
	return (srvfd);
}

/***************************************************************
 * accept_ui_session(): - Accept a new UI/DB/manager session.
 * This routine is called when a user interface program such
 * as Apache (for the web interface), the SNMP manager, or one
 * of the console interface programs tries to connect to the
 * data base interface socket to do DB like get's and set's.
 * The connection is actually established by the PostgreSQL
 * library attached to the UI program by an XXXXXX call.
 *
 * Input:        The file descriptor of the DB server socket
 * Output:       none
 * Effects:      manager connection table (ui)
 ***************************************************************/
void
sql_accept_conn(OS_SOCKET srvfd)
{
	int      adrlen;     /* length of an inet socket address */
	Sql_Conn *newui;
	char     tmp[16];

	/* if we reached our max connection, just exit */
	if (list_count(sqlconnections) > 5) {
		nlog(LOG_NOTICE, "Can not accept new SQL connection. Full");
		os_sock_close (srvfd);
		return;
	}

	/* We have a new UI/DB/manager connection request.  So make a free
	   slot and allocate it */
	newui = ns_malloc(sizeof(Sql_Conn)); 

	/* OK, we've got the ui slot, now accept the conn */
	adrlen = sizeof(struct sockaddr_in);
	newui->fd = accept(srvfd, (struct sockaddr *) &newui->cliskt, &adrlen);

	if (newui->fd < 0)
	{
		nlog(LOG_WARNING, "rtaserv: Manager accept() error (%s). \n", strerror(errno));
		ns_free(newui);
		os_sock_close (srvfd);
		return;
	}
	else
	{
		inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
		if (!match(rtahost, tmp)) {
    	/* we didnt get a match, bye bye */
			nlog(LOG_NOTICE, "rtaserv: Rejecting SQL Connection from %s", tmp);
			os_sock_close (newui->fd);
			ns_free(newui);
			return;
		}
		/* inc number ui, then init new ui */
		os_sock_set_nonblocking (srvfd);
		newui->cmdpos = 0;
		newui->responsefree = 50000; /* max response packetsize if 50000 */
		newui->nbytein = 0;
		newui->nbyteout = 0;
		lnode_create_append (sqlconnections, newui);
	    inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
		dlog(DEBUG1, "New SqlConnection from %s", tmp);
	}
}

/***************************************************************
 * handle_ui_request(): - This routine is called to read data
 * from the TCP connection to the UI  programs such as the web
 * UI and consoles.  The protocol used is that of Postgres and
 * the data is an encoded SQL request to select or update a 
 * system variable.  Note that the use of callbacks on reading
 * or writing means a lot of the operation of the program
 * starts from this execution path.  The input is an index into
 * the ui table for the manager with data ready.
 *
 * Input:        index of the relevant entry in the ui table
 * Output:       none
 * Effects:      many, many side effects via table callbacks
 ***************************************************************/
int 
sql_handle_ui_request(lnode_t *sqlnode)
{
	int      ret;        /* a return value */
	int      dbstat;     /* a return value */
	int      t;          /* a temp int */
	Sql_Conn *sqlconn;

   	dlog (DEBUG1, "sql_handle_ui_request");
	if ((sqlconn = lnode_get(sqlnode)) == NULL) {
		nlog(LOG_WARNING, "Got a Sql Handle without a valid node");
		return NS_SUCCESS;
	}

	/* We read data from the connection into the buffer in the ui struct. 
       Once we've read all of the data we can, we call the DB routine to
       parse out the SQL command and to execute it. */
	ret = os_sock_read (sqlconn->fd,
		&(sqlconn->cmd[sqlconn->cmdpos]), (1000 - sqlconn->cmdpos));

	/* shutdown manager conn on error or on zero bytes read */
	if (ret <= 0)
	{
		/* log this since a normal close is with an 'X' command from the
		   client program? */
		dlog(DEBUG1, "Disconnecting SqlClient for failed read");
		deldbconnection(sqlconn->fd);
		os_sock_close (sqlconn->fd);
		list_delete(sqlconnections, sqlnode);
		lnode_destroy(sqlnode);
		ns_free(sqlconn);
		return NS_FAILURE;
	}
	sqlconn->cmdpos += ret;
	sqlconn->nbytein += ret;

	/* The commands are in the buffer. Call the DB to parse and execute
	   them */
	do
	{
		t = sqlconn->cmdpos,       /* packet in length */
		dbstat = dbcommand((char *)sqlconn->cmd, /* packet in */
			&sqlconn->cmdpos,        /* packet in length */
			&sqlconn->response[50000 - sqlconn->responsefree], &sqlconn->responsefree, sqlconn->fd);
		t -= sqlconn->cmdpos,      /* t = # bytes consumed */
		/* move any trailing SQL cmd text up in the buffer */
		(void) memmove(sqlconn->cmd, &sqlconn->cmd[t], t);
	} while (dbstat == RTA_SUCCESS);
	if (dbstat == RTA_ERROR) {
		deldbconnection(sqlconn->fd);
	}
	/* the command is done (including side effects).  Send any reply back 
       to the UI */
	if (sql_handle_ui_output(sqlnode) == NS_FAILURE)
		return NS_FAILURE;
	return NS_SUCCESS;
}

/***************************************************************
 * handle_ui_output() - This routine is called to write data
 * to the TCP connection to the UI programs.  It is useful for
 * slow clients which can not accept the output in one big gulp.
 *
 * Input:        index of the relevant entry in the ui table
 * Output:       none
 * Effects:      none
 ***************************************************************/
int 
sql_handle_ui_output(lnode_t *sqlnode)
{
	int      ret;        /* write() return value */
	Sql_Conn *sqlconn;
 
   	dlog (DEBUG1, "sql_handle_ui_output");
	if ((sqlconn = lnode_get(sqlnode)) == NULL) {
		nlog(LOG_WARNING, "Got a Sql write Handle without a valid node");
		return NS_SUCCESS;
	}
  
	if (sqlconn->responsefree < 50000)
	{
		ret = os_sock_write (sqlconn->fd, sqlconn->response, (50000 - sqlconn->responsefree));
		if (ret < 0)
		{
			nlog(LOG_WARNING, "Got a write error when attempting to return data to the SQL Server");
			deldbconnection(sqlconn->fd);
			os_sock_close (sqlconn->fd);
			list_delete(sqlconnections, sqlnode);
			lnode_destroy(sqlnode);
			ns_free(sqlconn);
			return NS_FAILURE;
		}
		else if (ret == (50000 - sqlconn->responsefree))
		{
			sqlconn->responsefree = 50000;
			sqlconn->nbyteout += ret;
		}
		else
		{
			/* we had a partial write.  Adjust the buffer */
			(void) memmove(sqlconn->response, &sqlconn->response[ret],
			(50000 - sqlconn->responsefree - ret));
			sqlconn->responsefree += ret;
			sqlconn->nbyteout += ret;  /* # bytes sent on conn */
		}
	}
	return NS_SUCCESS;
}

//from read_loop
void rta_hook_1 (fd_set *read_fd_set, fd_set *write_fd_set)
{
	lnode_t *sqlnode;
	Sql_Conn *sqldata;
	
	if (!rta_active) {
		return ;
	}
	/* if we have sql support, add the Listen Socket to the fds */
	if (sqlListenSock > 0)
		FD_SET(sqlListenSock, read_fd_set);
	/* if we have any existing connections, add them of course */
	if (list_count(sqlconnections) > 0) {
		sqlnode = list_first(sqlconnections);
		while (sqlnode != NULL) {
			sqldata = lnode_get(sqlnode);
			if (sqldata->responsefree < 50000) {
				FD_SET(sqldata->fd, write_fd_set);
			} else {
				FD_SET(sqldata->fd, read_fd_set);			
			}
			sqlnode = list_next(sqlconnections, sqlnode);
		}
	}
}

void rta_hook_2 (fd_set *read_fd_set, fd_set *write_fd_set)
{
	lnode_t *sqlnode;
	Sql_Conn *sqldata;

	if (!rta_active) {
		return ;
	}
	/* did we get a connection to the SQL listen sock */
	if ((sqlListenSock > 0) && (FD_ISSET(sqlListenSock, read_fd_set)))
		sql_accept_conn(sqlListenSock);
	/* don't bother checking the sql connections if we dont have any active connections! */
restart:
	if (list_count(sqlconnections) > 0) {
		sqlnode = list_first(sqlconnections);
		while (sqlnode != NULL) {
			sqldata = lnode_get(sqlnode);
			if (FD_ISSET(sqldata->fd, read_fd_set)) {
				if (sql_handle_ui_request(sqlnode) == NS_FAILURE) {
					goto restart;
				}
			} else if (FD_ISSET(sqldata->fd, write_fd_set)) {
				if (sql_handle_ui_output(sqlnode) == NS_FAILURE) {
					goto restart;
				}
			}
			sqlnode = list_next(sqlconnections, sqlnode);
		}
	}
}

#if 0
/** Copyright info */
static const char *rta_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"RTAServ",
	"RTA Module",
	rta_copyright,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit (Module *modptr)
{
	ModuleConfig (rta_settings);
	if (InitRTAServ () != NS_SUCCESS) {
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch (void)
{
	if (add_services_set_list (rta_settings) != NS_SUCCESS) {
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return none
 */

void ModFini (void)
{
	del_services_set_list (rta_settings);
}
#else

void rtaserv_init (void)
{
	rta_active = 1;
	ModuleConfig (rta_settings);
	InitRTAServ ();
	add_services_set_list (rta_settings);
}

void rtaserv_fini (void)
{
	rta_active = 1;
	del_services_set_list (rta_settings);
}

#endif