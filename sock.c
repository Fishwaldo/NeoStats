/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
                     
#include "stats.h"
#include "dl.h"
#include "adns.h"
#include "conf.h"
#include "log.h"
#include "timer.h"
#include "dns.h"
#include "transfer.h"
#include "curl.h"
#include "ircstring.h"
#include "dotconf.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

static void recvlog (char *line);

static struct sockaddr_in lsa;
static int dobind;

int servsock;
char recbuf[BUFSIZE];


#ifdef SQLSRV

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


static void sql_accept_conn(int srvfd);
static int sqllisten_on_port(int port);
static int sql_handle_ui_request(lnode_t *sqlnode);
static int sql_handle_ui_output(lnode_t *sqlnode);
int check_sql_sock();


#endif 



/** @brief Connect to a server
 *
 *  also setups the SQL listen socket if defined 
 *
 * @param host to connect to
 * @param port on remote host to connect to
 * 
 * @return socket connected to on success
 *         NS_FAILURE on failure 
 */
int
ConnectTo (char *host, int port)
{
	int ret;
	struct hostent *hp;
	struct sockaddr_in sa;
	int s;

	dobind = 0;
	/* bind to a local ip */
	memset (&lsa, 0, sizeof (lsa));
	if (me.local[0] != 0) {
		if ((hp = gethostbyname (me.local)) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "Warning, Couldn't bind to IP address %s", me.local);
		} else {
			memcpy ((char *) &lsa.sin_addr, hp->h_addr, hp->h_length);
			lsa.sin_family = hp->h_addrtype;
			dobind = 1;
		}
	}
	if ((hp = gethostbyname (host)) == NULL) {
		return NS_FAILURE;
	}

	if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		free(hp);
		return NS_FAILURE;
	}
	if (dobind > 0) {
		if (bind (s, (struct sockaddr *) &lsa, sizeof (lsa)) < 0) {
			nlog (LOG_WARNING, LOG_CORE, "bind(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}

	bzero (&sa, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	bcopy (hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

	ret=connect (s, (struct sockaddr *) &sa, sizeof (sa));
	if (ret< 0) {
		close (s);
		return NS_FAILURE;
	}
#ifdef SQLSRV
	/* init the sql Listen Socket now as well */
	sqlconnections = list_create(MAXSQLCON);
	
	sqlListenSock = sqllisten_on_port(me.sqlport);
	if (sqlListenSock == -1) {
		nlog(LOG_CRITICAL, LOG_CORE, "Failed to Setup Sql Port. SQL not available");
	}
#endif	
	return s;
}

/** @brief main recv loop
 *
 * @param none
 * 
 * @return none
 */
void
read_loop ()
{
	register int i, j, SelectResult;
	struct timeval *TimeOut, tvbuf;
	fd_set readfds, writefds, errfds;
	int maxfdsunused;
	char c;
	char buf[BUFSIZE];
	ModSock *mod_sock;
	struct pollfd *ufds;
	int pollsize, pollflag;
	hscan_t ss;
	hnode_t *sn;
#ifdef SQLSRV
	lnode_t *sqlnode;
	Sql_Conn *sqldata;
#endif

	/* XXX Valgrind reports these two as lost memory, Should clean up before we exit */
	TimeOut = malloc (sizeof (struct timeval));
	ufds = malloc((sizeof *ufds) *  me.maxsocks);

	me.lastmsg = me.now;
	while (1) {
		SET_SEGV_LOCATION();
		memset (buf, '\0', BUFSIZE);
		me.now = time(NULL);
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%ld", (long)me.now);
		CheckTimers ();
		SET_SEGV_LOCATION();
		me.now = time(NULL);
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%ld", (long)me.now);
		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_ZERO (&errfds);
//		memset(ufds, 0, (sizeof *ufds) * me.maxsocks);
		pollsize = 0;
		FD_SET (servsock, &readfds);
		hash_scan_begin (&ss, sockh);
		me.cursocks = 1;	/* always one socket for ircd */
		while ((sn = hash_scan_next (&ss)) != NULL) {
			mod_sock = hnode_get (sn);
			if (mod_sock->socktype == SOCK_STANDARD) {
				if (mod_sock->readfnc)
					FD_SET (mod_sock->sock_no, &readfds);
				if (mod_sock->writefnc)
					FD_SET (mod_sock->sock_no, &writefds);
				if (mod_sock->errfnc)
					FD_SET (mod_sock->sock_no, &errfds);
				++me.cursocks;
			} else {
				/* its a poll interface, setup for select instead */
				SET_SEGV_INMODULE(mod_sock->modname);
				j = mod_sock->beforepoll (mod_sock->data, ufds);
				CLEAR_SEGV_INMODULE();
				/* if we don't have any socks, just continue */
				if (j == -1)
					continue;
					
				if (j > pollsize) pollsize = j;
				/* run through the ufds set and translate to select FDSET's */
				for (i = 0; i < j; i++) {
					if (ufds[i].events & POLLIN) {
						FD_SET (ufds[i].fd, &readfds);
					}
					if (ufds[i].events & POLLOUT) {
						FD_SET (ufds[i].fd, &writefds);
					}
					if (ufds[i].events & POLLERR) {
						FD_SET (ufds[i].fd, &errfds);
					}
					++me.cursocks;
				}
			}
		}
		/* adns stuff... whats its interested in */
		adns_beforeselect (ads, &me.maxsocks, &readfds, &writefds, &errfds, &TimeOut, &tvbuf, 0);
		/* adns may change this, but we tell it to go away!!! */
		TimeOut->tv_sec = 1;
		TimeOut->tv_usec = 0;

		/* add the fds for the curl library as well */
		/* XXX Should this be a pollsize or maxfdsunused... not sure yet */ 
		curl_multi_fdset(curlmultihandle, &readfds, &writefds, &errfds, &maxfdsunused);

#ifdef SQLSRV
		/* if we have sql support, add the Listen Socket to the fds */
		if (sqlListenSock > 0)
			FD_SET(sqlListenSock, &readfds);

		/* if we have any existing connections, add them of course */
		if (list_count(sqlconnections) > 0) {
			sqlnode = list_first(sqlconnections);
			while (sqlnode != NULL) {
				sqldata = lnode_get(sqlnode);
				if (sqldata->responsefree < 50000) {
					FD_SET(sqldata->fd, &writefds);
				} else {
					FD_SET(sqldata->fd, &readfds);			
				}
				sqlnode = list_next(sqlconnections, sqlnode);
			}
		}
#endif
		SelectResult = select (FD_SETSIZE, &readfds, &writefds, &errfds, TimeOut);
		me.now = time(NULL);
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%ld", (long)me.now);
		if (SelectResult > 0) {
			/* check ADNS fds */
			adns_afterselect (ads, me.maxsocks, &readfds, &writefds, &errfds, 0);

			/* do any dns related callbacks now */
			do_dns ();

			/* check CURL fds */
			while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curlmultihandle, &maxfdsunused)) {
			}
			transfer_status();
#ifdef SQLSRV
			/* did we get a connection to the SQL listen sock */
			if ((sqlListenSock > 0) && (FD_ISSET(sqlListenSock, &readfds)))
				sql_accept_conn(sqlListenSock);
restartsql:
			/* don't bother checking the sql connections if we dont have any active connections! */
			if (list_count(sqlconnections) > 0) {
				sqlnode = list_first(sqlconnections);
				while (sqlnode != NULL) {
					sqldata = lnode_get(sqlnode);
					if (FD_ISSET(sqldata->fd, &readfds)) {
						if (sql_handle_ui_request(sqlnode) == NS_FAILURE) {
							goto restartsql;
						}
					} else if (FD_ISSET(sqldata->fd, &writefds)) {
						if (sql_handle_ui_output(sqlnode) == NS_FAILURE) {
							goto restartsql;
						}
					}
					sqlnode = list_next(sqlconnections, sqlnode);
				}
			}
#endif
			if (FD_ISSET (servsock, &readfds)) {
				for (j = 0; j < BUFSIZE; j++) {
					i = read (servsock, &c, 1);
					me.RcveBytes++;
					if (i >= 0) {
						buf[j] = c;
						if ((c == '\n') || (c == '\r')) {
							me.RcveM++;
							me.lastmsg = me.now;
							if (config.recvlog)
								recvlog (buf);
							parse (buf);
							break;
						}
					} else {
						nlog (LOG_WARNING, LOG_CORE, "read returned an Error");
						servsock = -1;
						return;
					}
				}
			} else {
				/* this checks if there is any data waiting on a socket for a module */
				hash_scan_begin (&ss, sockh);
				while ((sn = hash_scan_next (&ss)) != NULL) {
					pollflag = 0;
					mod_sock = hnode_get (sn);
					SET_SEGV_INMODULE(mod_sock->modname);
					if (mod_sock->socktype == SOCK_STANDARD) {
						if (FD_ISSET (mod_sock->sock_no, &readfds)) {
							nlog (LOG_DEBUG3, LOG_CORE, "Running module %s readsock function for %s", mod_sock->modname, mod_sock->sockname);
							if (mod_sock->readfnc (mod_sock->sock_no, mod_sock->sockname) < 0)
								continue;
						}
						if (FD_ISSET (mod_sock->sock_no, &writefds)) {
							nlog (LOG_DEBUG3, LOG_CORE, "Running module %s writesock function for %s", mod_sock->modname, mod_sock->sockname);
							if (mod_sock->writefnc (mod_sock->sock_no, mod_sock->sockname) < 0)
								continue;
						}
						if (FD_ISSET (mod_sock->sock_no, &errfds)) {
							nlog (LOG_DEBUG3, LOG_CORE, "Running module %s errorsock function for %s", mod_sock->modname, mod_sock->sockname);
							if (mod_sock->errfnc (mod_sock->sock_no, mod_sock->sockname) < 0)
								continue;
						}
					} else {
						if (pollflag != 1) {
							for (i = 0; i < pollsize; i++) {
								if (FD_ISSET(ufds[i].fd, &readfds)) {
									ufds[i].revents |= POLLIN;
									pollflag = 1;
								} 
								if (FD_ISSET(ufds[i].fd, &writefds)) {
									ufds[i].revents |= POLLOUT;
									pollflag = 1;
								} 
								if (FD_ISSET(ufds[i].fd, &errfds)) {
									ufds[i].revents |= POLLERR;
									pollflag = 1;
								}
							}
						}
						if (pollflag == 1) {
							mod_sock->afterpoll(mod_sock->data, ufds, pollsize);
						}
					}
				}
				CLEAR_SEGV_INMODULE();
				continue;
			}
		} else if (SelectResult == 0) {
			if ((me.now - me.lastmsg) > 180) {
				/* if we havnt had a message for 3 minutes, more than likely, we are on a zombie server */
				/* disconnect and try to reconnect */
				nlog (LOG_WARNING, LOG_CORE, "Eeek, Zombie Server, Reconnecting");
				return;
			}
		} else if (SelectResult == -1) {
			if (errno != EINTR) {
				nlog (LOG_WARNING, LOG_CORE, "Lost connection to server.");
				return;
			}
		}
	}
	nlog (LOG_NORMAL, LOG_CORE, "hu, how did we get here");
}

/** @brief get max available sockets
 *
 * @param none
 * 
 * @return returns the max available socket 
 */
int
getmaxsock ()
{
	struct rlimit *lim;
	int ret;

	lim = malloc (sizeof (struct rlimit));
	getrlimit (RLIMIT_NOFILE, lim);
	ret = lim->rlim_max;
	free (lim);
	return ret;
}

/** @brief recv logging for all data received by us
 *
 * @param line of text received
 * 
 * @return none
 */
static void
recvlog (char *line)
{
	FILE *logfile;

	if ((logfile = fopen (RECV_LOG, "a")) == NULL)
		return;
	if (logfile)
		fprintf (logfile, "%s", line);
	fclose (logfile);
}

/** @brief connect to a socket
 *
 * @param socktype type of socket
 * @param ipaddr ip address of target
 * @param port to connect to
 * @param sockname name of this socket
 * @param module name of the module
 * @param func_read read socket function
 * @param func_write write socket function
 * @param func_error socket error function
 * 
 * @return socket number if connect successful
 *         NS_FAILURE if unsuccessful
 */
int
sock_connect (int socktype, unsigned long ipaddr, int port, char *sockname, char *module, char *func_read, char *func_write, char *func_error)
{
	struct sockaddr_in sa;
	int s;
	int i;

	/* socktype = SOCK_STREAM */
	if ((s = socket (AF_INET, socktype, 0)) < 0)
		return NS_FAILURE;

	/* bind to an IP address */
	if (dobind > 0) {
		if (bind (s, (struct sockaddr *) &lsa, sizeof (lsa)) < 0) {
			nlog (LOG_WARNING, LOG_CORE, "sock_connect(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}

	bzero (&sa, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	sa.sin_addr.s_addr = ipaddr;

	/* set non blocking */

	if ((i = fcntl (s, F_SETFL, O_NONBLOCK)) < 0) {
		nlog (LOG_CRITICAL, LOG_CORE, "can't set socket %s(%s) non-blocking: %s", sockname, module, strerror (i));
		return NS_FAILURE;
	}

	if ((i = connect (s, (struct sockaddr *) &sa, sizeof (sa))) < 0) {
		switch (errno) {
		case EINPROGRESS:
			break;
		default:
			nlog (LOG_WARNING, LOG_CORE, "Socket %s(%s) cant connect %s", sockname, module, strerror (errno));
			close (s);
			return NS_FAILURE;
		}
	}

	add_socket (func_read, func_write, func_error, sockname, s, module);
	return s;
}

/** @brief disconnect socket
 *
 * @param name of socket to disconnect
 * 
 * @return NS_SUCCESS if disconnect successful
 *         NS_FAILURE if unsuccessful
 */
int
sock_disconnect (char *sockname)
{
	ModSock *mod_sock;
	fd_set fds;
	struct timeval tv;
	int i;

	mod_sock = findsock (sockname);
	if (!mod_sock) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Can not find Socket %s in list", sockname);
		return NS_FAILURE;
	}

	/* the following code makes sure its a valid file descriptor */
	FD_ZERO (&fds);
	FD_SET (mod_sock->sock_no, &fds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	i = select (1, &fds, NULL, NULL, &tv);
	if (!i && errno == EBADF) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Bad File Descriptor %s in list", sockname);
		return NS_FAILURE;
	}
	nlog (LOG_DEBUG3, LOG_CORE, "Closing Socket %s with Number %d", sockname, mod_sock->sock_no);
	close (mod_sock->sock_no);
	del_socket (sockname);
	return NS_SUCCESS;
}

/** @brief send to socket
 *
 * @param fmt printf style vaarg list of text to send
 * 
 * @return none
 */
void
sts (const char *buf, const int buflen)
{
	int sent;

	if (servsock == -1) {
		nlog(LOG_WARNING, LOG_CORE, "Not sending to server as we have a invalid socket");
		return;
	}
	sent = write (servsock, buf, buflen);
	if (sent == -1) {
		nlog (LOG_CRITICAL, LOG_CORE, "Write error: %s", strerror(errno));
		servsock = -1;
		do_exit (NS_EXIT_ERROR, NULL);
	}
	me.SendM++;
	me.SendBytes = me.SendBytes + sent;
}

#if SQLSRV
/* the following functions are taken from the RTA example app shipped with the library, 
 * and modified to work with NeoStats. 
 * Credit for these apps should be given to the respective authors of the RTA library.
 * more info can be found at http://www.linuxappliancedesign.com for more
 * information
 */

/* rehash handler */
int check_sql_sock() {
	if (sqlListenSock < 1) {
		nlog(LOG_DEBUG1, LOG_CORE, "Rehashing SQL sock");
        	sqlListenSock = sqllisten_on_port(me.sqlport);
		if (sqlListenSock == -1) {
			nlog(LOG_CRITICAL, LOG_CORE, "Failed to Setup Sql Port. SQL not available");
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
  if (dobind) {
	srvskt.sin_addr = lsa.sin_addr;
  } else {
  	srvskt.sin_addr.s_addr = INADDR_ANY;
  }
  srvskt.sin_port = htons(me.sqlport);
  if ((srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    nlog(LOG_CRITICAL,LOG_CORE, "SqlSrv: Unable to get socket for port %d.", port);
    return -1;
  }
  flags = fcntl(srvfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  (void) fcntl(srvfd, F_SETFL, flags);
  if (bind(srvfd, (struct sockaddr *) &srvskt, adrlen) < 0)
  {
    nlog(LOG_CRITICAL, LOG_CORE, "Unable to bind to port %d", port);
    return -1;
  }
  if (listen(srvfd, 1) < 0)
  {
    nlog(LOG_CRITICAL, LOG_CORE, "Unable to listen on port %d", port);
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
  lnode_t  *newuinode;
  Sql_Conn *newui;
  char     tmp[16];

  /* if we reached our max connection, just exit */
  if (list_count(sqlconnections) > 5) {
  	nlog(LOG_NOTICE, LOG_CORE, "Can not accept new SQL connection. Full");
  	close (srvfd);
  	return;
  }
  
  /* We have a new UI/DB/manager connection request.  So make a free
     slot and allocate it */
  newui = malloc(sizeof(Sql_Conn));
  

  /* OK, we've got the ui slot, now accept the conn */
  adrlen = sizeof(struct sockaddr_in);
  newui->fd = accept(srvfd, (struct sockaddr *) &newui->cliskt, &adrlen);

  if (newui->fd < 0)
  {
    nlog(LOG_WARNING, LOG_CORE, "SqlSrv: Manager accept() error (%s). \n", strerror(errno));
    free(newui);
    close(srvfd);
    return;
  }
  else
  {
    inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
    if (!match(me.sqlhost, tmp)) {
    	/* we didnt get a match, bye bye */
	nlog(LOG_NOTICE, LOG_CORE, "SqlSrv: Rejecting SQL Connection from %s", tmp);
	close(newui->fd);
	free(newui);
        return;
    }
    /* inc number ui, then init new ui */
    flags = fcntl(newui->fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    (void) fcntl(newui->fd, F_SETFL, flags);
    newui->cmdpos = 0;
    newui->responsefree = 50000; /* max response packetsize if 50000 */
    newui->nbytein = 0;
    newui->nbyteout = 0;
    newuinode = lnode_create(newui);
    list_append(sqlconnections, newuinode);
    inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
    nlog(LOG_DEBUG1, LOG_CORE, "New SqlConnection from %s", tmp);
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
  	nlog(LOG_WARNING, LOG_CORE, "Got a Sql Handle without a valid node");
  	return NS_SUCCESS;
  }


  /* We read data from the connection into the buffer in the ui struct. 
     Once we've read all of the data we can, we call the DB routine to
     parse out the SQL command and to execute it. */
  ret = read(sqlconn->fd,
    &(sqlconn->cmd[sqlconn->cmdpos]), (1000 - sqlconn->cmdpos));

  /* shutdown manager conn on error or on zero bytes read */
  if (ret <= 0)
  {
    /* log this since a normal close is with an 'X' command from the
       client program? */
    nlog(LOG_DEBUG1, LOG_CORE, "Disconnecting SqlClient for failed read");
    deldbconnection(sqlconn->fd);
    close(sqlconn->fd);
    list_delete(sqlconnections, sqlnode);
    lnode_destroy(sqlnode);
    free(sqlconn);
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
  	nlog(LOG_WARNING, LOG_CORE, "Got a Sql write Handle without a valid node");
  	return NS_SUCCESS;
  }
  
  if (sqlconn->responsefree < 50000)
  {
    ret = write(sqlconn->fd, sqlconn->response, (50000 - sqlconn->responsefree));
    if (ret < 0)
    {
    	nlog(LOG_WARNING, LOG_CORE, "Got a write error when attempting to return data to the SQL Server");
	deldbconnection(sqlconn->fd);
      	close(sqlconn->fd);
	list_delete(sqlconnections, sqlnode);
	lnode_destroy(sqlnode);
	free(sqlconn);
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


#endif
