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
#include "sqlsrv.h"

#define MAXSQLCON 5

/* sqlsrv struct for tracking connections */
typedef struct Sql_Conn {
	struct sockaddr_in cliskt;
	int fd;
	long long nbytein;
	long long nbyteout;
	char response[50000];
	int responsefree;
	int cmdpos;
	int cmd[1000];
} Sql_Conn;

int sqlListenSock = -1;

list_t *sqlconnections;
char sqlsrvuser[MAXUSER];
char sqlsrvpass[MAXPASS];
char sqlsrvhost[MAXHOST];
int sqlsrvport;

static void sql_accept_conn(int srvfd);
static int sqllisten_on_port(int port);
static int sql_handle_ui_request(lnode_t *sqlnode);
static int sql_handle_ui_output(lnode_t *sqlnode);
int check_sql_sock();

/* this is for sqlserver logging callback */
void sqlsrvlog(char *logline) 
{
	dlog(DEBUG1, "SqlSrv: %s", logline);
}

int InitSqlSrv (void)
{
	sqlsrvport = 8888;
	rta_init(sqlsrvlog);
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
	sqlListenSock = sqllisten_on_port(sqlsrvport);
	if (sqlListenSock == -1) {
		nlog(LOG_CRITICAL, "Failed to Setup Sql Port. SQL not available");
		return NS_FAILURE;
	}
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

const char *sql_help_set_sqluser[] = {
	"SQLUSER ",
	NULL
};

const char *sql_help_set_sqlpass[] = {
	"SQLPASS ",
	NULL
};

const char *sql_help_set_sqlhost[] = {
	"SQLHOST ",
	NULL
};

const char *sql_help_set_sqlport[] = {
	"SQLPORT ",
	NULL
};

static int sql_set_sqluser_cb (CmdParams* cmdparams, SET_REASON reason)
{
	rta_change_auth(sqlsrvuser, sqlsrvpass);
	return NS_SUCCESS;
}

static int sql_set_sqlpass_cb (CmdParams* cmdparams, SET_REASON reason)
{
	rta_change_auth(sqlsrvuser, sqlsrvpass);
	return NS_SUCCESS;
}

static int sql_set_sqlhost_cb (CmdParams* cmdparams, SET_REASON reason)
{
	return NS_SUCCESS;
}

static int sql_set_sqlport_cb (CmdParams* cmdparams, SET_REASON reason)
{
	return NS_SUCCESS;
}

static bot_setting sql_settings[]=
{
	{"SQLUSER",	&sqlsrvuser,	SET_TYPE_STRING,	0, MAXUSER, NS_ULEVEL_ADMIN, "sqluser",	NULL,	sql_help_set_sqluser, sql_set_sqluser_cb, (void*)"user" },
	{"SQLPASS",	&sqlsrvpass,	SET_TYPE_STRING,	0, MAXPASS, NS_ULEVEL_ADMIN, "sqlpass",	NULL,	sql_help_set_sqlpass, sql_set_sqlpass_cb, (void*)"pass" },
	{"SQLHOST",	&sqlsrvhost,	SET_TYPE_HOST,		0, MAXHOST, NS_ULEVEL_ADMIN, "sqlhost",	NULL,	sql_help_set_sqlhost, sql_set_sqlhost_cb, (void*)"127.0.0.1" },
	{"SQLPORT",	&sqlsrvport,	SET_TYPE_INT,		0, 0, 		NS_ULEVEL_ADMIN, "sqlport",	NULL,	sql_help_set_sqlport, sql_set_sqlport_cb, (void*)8888 },
	{NULL,		NULL,			0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/* the following functions are taken from the RTA example app shipped with the library, 
 * and modified to work with NeoStats. 
 * Credit for these apps should be given to the respective authors of the RTA library.
 * more info can be found at http://www.linuxappliancedesign.com for more
 * information
 */

/* rehash handler */
int check_sql_sock() {
	if (sqlListenSock < 1) {
		dlog(DEBUG1, "Rehashing SQL sock");
        	sqlListenSock = sqllisten_on_port(sqlsrvport);
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
int
sqllisten_on_port(int port)
{
  int      srvfd;      /* FD for our listen server socket */
  struct sockaddr_in srvskt;
  int      adrlen;
  int      flags;

  adrlen = sizeof(struct sockaddr_in);
  (void) memset((void *) &srvskt, 0, (size_t) adrlen);
  srvskt.sin_family = AF_INET;
  /* bind to the local IP */
  if (me.dobind) {
	srvskt.sin_addr = me.lsa.sin_addr;
  } else {
  	srvskt.sin_addr.s_addr = INADDR_ANY;
  }
  srvskt.sin_port = htons(sqlsrvport);
  if ((srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    nlog(LOG_CRITICAL, "SqlSrv: Unable to get socket for port %d.", port);
    return -1;
  }
#ifdef WIN32
	flags = 1;
	ioctlsocket(srvfd, FIONBIO, &flags);
#else
  flags = fcntl(srvfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  (void) fcntl(srvfd, F_SETFL, flags);
#endif
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
sql_accept_conn(int srvfd)
{
  int      adrlen;     /* length of an inet socket address */
  int      flags;      /* helps set non-blocking IO */
  Sql_Conn *newui;
  char     tmp[16];

  /* if we reached our max connection, just exit */
  if (list_count(sqlconnections) > 5) {
  	nlog(LOG_NOTICE, "Can not accept new SQL connection. Full");
#ifdef WIN32
	closesocket (srvfd);
#else
	close (srvfd);
#endif      			
  	return;
  }
  
  /* We have a new UI/DB/manager connection request.  So make a free
     slot and allocate it */
  newui = smalloc(sizeof(Sql_Conn));
  

  /* OK, we've got the ui slot, now accept the conn */
  adrlen = sizeof(struct sockaddr_in);
  newui->fd = accept(srvfd, (struct sockaddr *) &newui->cliskt, &adrlen);

  if (newui->fd < 0)
  {
    nlog(LOG_WARNING, "SqlSrv: Manager accept() error (%s). \n", strerror(errno));
    sfree(newui);
#ifdef WIN32
	closesocket (srvfd);
#else
	close (srvfd);
#endif      			
    return;
  }
  else
  {
#ifdef WIN32
	{
		unsigned char* src = &(newui->cliskt.sin_addr.s_addr);
		sprintf(tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]);
	}
#else
    inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
#endif
    if (!match(sqlsrvhost, tmp)) {
    	/* we didnt get a match, bye bye */
	nlog(LOG_NOTICE, "SqlSrv: Rejecting SQL Connection from %s", tmp);
#ifdef WIN32
	closesocket (newui->fd);
#else
	close (newui->fd);
#endif      			
	sfree(newui);
        return;
    }
    /* inc number ui, then init new ui */
#ifdef WIN32
	flags = 1;
	ioctlsocket(srvfd, FIONBIO, &flags);
#else
    flags = fcntl(newui->fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    (void) fcntl(newui->fd, F_SETFL, flags);
#endif
    newui->cmdpos = 0;
    newui->responsefree = 50000; /* max response packetsize if 50000 */
    newui->nbytein = 0;
    newui->nbyteout = 0;
	lnode_create_append (sqlconnections, newui);
#ifdef WIN32
	{
		unsigned char* src = &newui->cliskt.sin_addr.s_addr;
		sprintf(tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]);
	}
#else
    inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
#endif
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

  if ((sqlconn = lnode_get(sqlnode)) == NULL) {
  	nlog(LOG_WARNING, "Got a Sql Handle without a valid node");
  	return NS_SUCCESS;
  }

  /* We read data from the connection into the buffer in the ui struct. 
     Once we've read all of the data we can, we call the DB routine to
     parse out the SQL command and to execute it. */
#ifdef WIN32
  ret = recv(sqlconn->fd,
    &(sqlconn->cmd[sqlconn->cmdpos]), (1000 - sqlconn->cmdpos), 0);
#else
  ret = read(sqlconn->fd,
    &(sqlconn->cmd[sqlconn->cmdpos]), (1000 - sqlconn->cmdpos));
#endif

  /* shutdown manager conn on error or on zero bytes read */
  if (ret <= 0)
  {
    /* log this since a normal close is with an 'X' command from the
       client program? */
    dlog(DEBUG1, "Disconnecting SqlClient for failed read");
    deldbconnection(sqlconn->fd);
#ifdef WIN32
	closesocket (sqlconn->fd);
#else
    close(sqlconn->fd);
#endif      			
    list_delete(sqlconnections, sqlnode);
    lnode_destroy(sqlnode);
    sfree(sqlconn);
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
  sql_handle_ui_output(sqlnode);
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
 
  if ((sqlconn = lnode_get(sqlnode)) == NULL) {
  	nlog(LOG_WARNING, "Got a Sql write Handle without a valid node");
  	return NS_SUCCESS;
  }
  
  if (sqlconn->responsefree < 50000)
  {
    ret = write(sqlconn->fd, sqlconn->response, (50000 - sqlconn->responsefree));
    if (ret < 0)
    {
    	nlog(LOG_WARNING, "Got a write error when attempting to return data to the SQL Server");
	deldbconnection(sqlconn->fd);
#ifdef WIN32
		closesocket (sqlconn->fd);
#else
	    close(sqlconn->fd);
#endif      			
	list_delete(sqlconnections, sqlnode);
	lnode_destroy(sqlnode);
	sfree(sqlconn);
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
MODULEFUNC void rta_hook_1 (fd_set *read_fd_set, fd_set *write_fd_set)
{
	lnode_t *sqlnode;
	Sql_Conn *sqldata;

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

MODULEFUNC void rta_hook_2 (fd_set *read_fd_set, fd_set *write_fd_set)
{
	lnode_t *sqlnode;
	Sql_Conn *sqldata;

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

/** Copyright info */
static const char *sql_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"SqlSrv",
	"RTA Module",
	sql_copyright,
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
	ModuleConfig (sql_settings);
	if (InitSqlSrv () != NS_SUCCESS) {
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
	if (add_services_set_list (sql_settings) != NS_SUCCESS) {
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
	del_services_set_list (sql_settings);
}

#ifdef WIN32
/* temporary work around for linker error */
void main(void) {}
#endif