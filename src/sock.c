/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef WIN32
#include <arpa/inet.h>
#include <poll.h>
#else
struct pollfd { int fd; short events; short revents; };
#define POLLIN  1
#define POLLPRI 2
#define POLLOUT 4
#define POLLERR 8
#endif
#include "neostats.h"
#include <fcntl.h>

                     
#include "adns.h"
#include "timer.h"
#include "dns.h"
#include "transfer.h"
#include "curl.h"
#include "dotconf.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif
#include "services.h"
#include "ircd.h"

static void recvlog (char *line);

static struct sockaddr_in lsa;
static int dobind;
/* @brief Module Socket List hash */
static hash_t *sockethash;
/* @brief server socket */
static int servsock;
char recbuf[BUFSIZE];
static struct timeval *TimeOut;
static struct pollfd *ufds;

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
static int
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
			nlog (LOG_WARNING, "Warning, Couldn't bind to IP address %s", me.local);
		} else {
			memcpy ((char *) &lsa.sin_addr, hp->h_addr, hp->h_length);
			lsa.sin_family = hp->h_addrtype;
			dobind = 1;
		}
	}
	if ((hp = gethostbyname (host)) == NULL) {
		return NS_FAILURE;
	}

	if ((s = (int)socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		sfree(hp);
		return NS_FAILURE;
	}
	if (dobind > 0) {
		if (bind (s, (struct sockaddr *) &lsa, sizeof (lsa)) < 0) {
			nlog (LOG_WARNING, "bind(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}

	bzero (&sa, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	bcopy (hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

	ret=connect (s, (struct sockaddr *) &sa, sizeof (sa));
	if (ret< 0) {
#ifdef WIN32
		nlog (LOG_ERROR, "Winsock error: %d", WSAGetLastError());
#endif
#ifdef WIN32
		closesocket (s);
#else
		close (s);
#endif      			
		return NS_FAILURE;
	}
#ifdef SQLSRV
	/* init the sql Listen Socket now as well */
	sqlconnections = list_create(MAXSQLCON);
	
	sqlListenSock = sqllisten_on_port(me.sqlport);
	if (sqlListenSock == -1) {
		nlog(LOG_CRITICAL, "Failed to Setup Sql Port. SQL not available");
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
#ifndef WIN32
static void
#else
void
#endif
read_loop ()
{
	register int i, j, SelectResult;
	struct timeval tvbuf;
	fd_set readfds, writefds, errfds;
	int maxfdsunused;
	char c;
	char buf[BUFSIZE];
	Sock *sock;
	int pollsize, pollflag;
	hscan_t ss;
	hnode_t *sn;
#ifdef SQLSRV
	lnode_t *sqlnode;
	Sql_Conn *sqldata;
#endif

	me.lastmsg = me.now;
	while (1) {
		SET_SEGV_LOCATION();
		memset (buf, '\0', BUFSIZE);
		me.now = time(NULL);
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
		CheckTimers ();
		SET_SEGV_LOCATION();
		me.now = time(NULL);
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_ZERO (&errfds);
//		memset(ufds, 0, (sizeof *ufds) * me.maxsocks);
		pollsize = 0;
		FD_SET (servsock, &readfds);
		hash_scan_begin (&ss, sockethash);
		me.cursocks = 1;	/* always one socket for ircd */
		while ((sn = hash_scan_next (&ss)) != NULL) {
			sock = hnode_get (sn);
			if (sock->socktype == SOCK_STANDARD) {
				if (sock->readfnc)
					FD_SET (sock->sock_no, &readfds);
				if (sock->writefnc)
					FD_SET (sock->sock_no, &writefds);
				if (sock->errfnc)
					FD_SET (sock->sock_no, &errfds);
				++me.cursocks;
			} else {
				/* its a poll interface, setup for select instead */
				SET_RUN_LEVEL(sock->moduleptr);
				j = sock->beforepoll (sock->data, ufds);
				RESET_RUN_LEVEL();
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
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
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
#ifdef WIN32
					i = recv (servsock, &c, 1, 0);
#else
					i = read (servsock, &c, 1);
#endif
					me.RcveBytes++;
					if (i >= 0) {
						buf[j] = c;
						if ((c == '\n') || (c == '\r')) {
							me.RcveM++;
							me.lastmsg = me.now;
							if (config.recvlog)
								recvlog (buf);
							strip (buf);
							strlcpy (recbuf, buf, BUFSIZE);
							irc_parse (buf);
							break;
						}
					} else {
						nlog (LOG_WARNING, "read returned an Error");
						servsock = -1;
						return;
					}
				}
			} else {
				/* this checks if there is any data waiting on a socket for a module */
				hash_scan_begin (&ss, sockethash);
				while ((sn = hash_scan_next (&ss)) != NULL) {
					pollflag = 0;
					sock = hnode_get (sn);
					SET_RUN_LEVEL(sock->moduleptr);
					if (sock->socktype == SOCK_STANDARD) {
						if (FD_ISSET (sock->sock_no, &readfds)) {
							dlog(DEBUG3, "Running module %s readsock function for %s", sock->moduleptr->info->name, sock->name);
							if (sock->readfnc (sock->sock_no, sock->name) < 0)
								continue;
						}
						if (FD_ISSET (sock->sock_no, &writefds)) {
							dlog(DEBUG3, "Running module %s writesock function for %s", sock->moduleptr->info->name, sock->name);
							if (sock->writefnc (sock->sock_no, sock->name) < 0)
								continue;
						}
						if (FD_ISSET (sock->sock_no, &errfds)) {
							dlog(DEBUG3, "Running module %s errorsock function for %s", sock->moduleptr->info->name, sock->name);
							if (sock->errfnc (sock->sock_no, sock->name) < 0)
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
							sock->afterpoll(sock->data, ufds, pollsize);
						}
					}
					RESET_RUN_LEVEL();
				}				
				continue;
			}
		} else if (SelectResult == 0) {
			if ((me.now - me.lastmsg) > 180) {
				/* if we havnt had a message for 3 minutes, more than likely, we are on a zombie server */
				/* disconnect and try to reconnect */
				nlog (LOG_WARNING, "Eeek, Zombie Server, Reconnecting");
				return;
			}
		} else if (SelectResult == -1) {
			if (errno != EINTR) {
				nlog (LOG_WARNING, "Lost connection to server.");
				return;
			}
		}
	}
	nlog (LOG_NORMAL, "hu, how did we get here");
}

/** @brief Connects to IRC and starts the main loop
 *
 * Connects to the IRC server and attempts to login
 * If it connects and logs in, then starts the main program loop
 * if control is returned to this function, restart
 * 
 * @return Nothing
 *
 * @todo make the restart code nicer so it doesn't go mad when we can't connect
 */
void
Connect (void)
{
	SET_SEGV_LOCATION();
	nlog (LOG_NOTICE, "Connecting to %s:%d", me.uplink, config.port);
	servsock = ConnectTo (me.uplink, config.port);
	if (servsock <= 0) {
		nlog (LOG_WARNING, "Unable to connect to %s", me.uplink);
	} else {
		/* Call the IRC specific function send_server_connect to login as a server to IRC */
		irc_send_server_connect (me.name, me.numeric, me.infoline, config.pass, (unsigned long)me.t_start, (unsigned long)me.now);
#ifndef WIN32
		read_loop ();
#endif
	}
#ifndef WIN32
	do_exit (NS_EXIT_RECONNECT, NULL);
#endif
}

/** @brief get max available sockets
 *
 * @param none
 * 
 * @return returns the max available socket 
 */
static int
getmaxsock (void)
{
#ifdef WIN32
	return 0xffff;
#else
	struct rlimit *lim;
	int ret;

	lim = smalloc (sizeof (struct rlimit));
	getrlimit (RLIMIT_NOFILE, lim);
	ret = lim->rlim_max;
	sfree (lim);
	if(ret<0)
		ret = 0xffff;
	return ret;
#endif
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
 * @param name name of this socket
 * @param module name of the module
 * @param func_read read socket function
 * @param func_write write socket function
 * @param func_error socket error function
 * 
 * @return socket number if connect successful
 *         NS_FAILURE if unsuccessful
 */
int
sock_connect (int socktype, unsigned long ipaddr, int port, const char *name, sock_func func_read, sock_func func_write, sock_func func_error)
{
	struct sockaddr_in sa;
	int s;
	int i;
	Module* moduleptr;

	moduleptr = GET_CUR_MODULE();
	/* socktype = SOCK_STREAM */
	if ((s = (int)socket (AF_INET, socktype, 0)) < 0)
		return NS_FAILURE;

	/* bind to an IP address */
	if (dobind > 0) {
		if (bind (s, (struct sockaddr *) &lsa, sizeof (lsa)) < 0) {
			nlog (LOG_WARNING, "sock_connect(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}

	bzero (&sa, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	sa.sin_addr.s_addr = ipaddr;

	/* set non blocking */

#ifdef WIN32
	{
		int flags;
		flags = 1;
		return ioctlsocket(s, FIONBIO, &flags);
	}
#else
	if ((i = fcntl (s, F_SETFL, O_NONBLOCK)) < 0) {
		nlog (LOG_CRITICAL, "can't set socket %s(%s) non-blocking: %s", name, moduleptr->info->name, strerror (i));
		return NS_FAILURE;
	}
#endif

	if ((i = connect (s, (struct sockaddr *) &sa, sizeof (sa))) < 0) {
		switch (errno) {
		case EINPROGRESS:
			break;
		default:
			nlog (LOG_WARNING, "Socket %s(%s) cant connect %s", name, moduleptr->info->name, strerror (errno));
#ifdef WIN32
			closesocket (s);
#else
			close (s);
#endif      			
			return NS_FAILURE;
		}
	}

	add_sock (func_read, func_write, func_error, name, s);
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
sock_disconnect (const char *name)
{
	Sock *sock;
	fd_set fds;
	struct timeval tv;
	int i;

	sock = find_sock (name);
	if (!sock) {
		nlog (LOG_WARNING, "Warning, Can not find Socket %s in list", name);
		return NS_FAILURE;
	}

	/* the following code makes sure its a valid file descriptor */
	FD_ZERO (&fds);
	FD_SET (sock->sock_no, &fds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	i = select (1, &fds, NULL, NULL, &tv);
	if (!i && errno == EBADF) {
		nlog (LOG_WARNING, "Warning, Bad File Descriptor %s in list", name);
		return NS_FAILURE;
	}
	dlog(DEBUG3, "Closing Socket %s with Number %d", name, sock->sock_no);
#ifdef WIN32
	closesocket (sock->sock_no);
#else
	close (sock->sock_no);
#endif      			
	del_sock (name);
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
		nlog(LOG_WARNING, "Not sending to server as we have a invalid socket");
		return;
	}
#ifdef WIN32
	sent = send (servsock, buf, buflen, 0);
#else	
	sent = write (servsock, buf, buflen);
#endif
	if (sent == -1) {
		nlog (LOG_CRITICAL, "Write error: %s", strerror(errno));
		/* Try to close socket then reset the servsock value to avoid cyclic calls */
#ifdef WIN32
		closesocket (servsock);
#else
		close (servsock);
#endif      			
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
		dlog(DEBUG1, "Rehashing SQL sock");
        	sqlListenSock = sqllisten_on_port(me.sqlport);
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
  if (dobind) {
	srvskt.sin_addr = lsa.sin_addr;
  } else {
  	srvskt.sin_addr.s_addr = INADDR_ANY;
  }
  srvskt.sin_port = htons(me.sqlport);
  if ((srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    nlog(LOG_CRITICAL, "SqlSrv: Unable to get socket for port %d.", port);
    return -1;
  }
  flags = fcntl(srvfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  (void) fcntl(srvfd, F_SETFL, flags);
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
  lnode_t  *newuinode;
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
    inet_ntop(AF_INET, &newui->cliskt.sin_addr.s_addr, tmp, 16);
    if (!match(me.sqlhost, tmp)) {
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
    &(sqlconn->cmd[sqlconn->cmdpos]), (1000 - sqlconn->cmdpos));
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


#endif

int InitSocks (void)
{
	me.maxsocks = getmaxsock ();
	sockethash = hash_create (me.maxsocks, 0, 0);
	if(!sockethash) {
		nlog (LOG_CRITICAL, "Unable to create socks hash");
		return NS_FAILURE;
	}
	TimeOut = smalloc (sizeof (struct timeval));
	ufds = smalloc((sizeof *ufds) *  me.maxsocks);
	return NS_SUCCESS;
}

int FiniSocks (void) 
{
	sfree(TimeOut);
	sfree(ufds);
	if (servsock > 0)
#ifdef WIN32
		closesocket (servsock);
#else
		close(servsock);
#endif      			
	hash_destroy(sockethash);
	return NS_SUCCESS;
}

/** @brief create a new socket
 *
 * For core use only, creates a new socket for a module
 *
 * @param sock_name the name of the socket to create
 * 
 * @return pointer to created socket on success, NULL on error
*/
static Sock *
new_sock (const char *sock_name)
{
	Sock *sock;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "new_sock: %s", sock_name);
	if (hash_isfull (sockethash)) {
		nlog (LOG_CRITICAL, "new_sock: socket hash is full");
		return NULL;
	}
	sock = smalloc (sizeof (Sock));
	strlcpy (sock->name, sock_name, MAX_MOD_NAME);
	sn = hnode_create (sock);
	hash_insert (sockethash, sn, sock->name);
	return sock;
}

/** \fn @brief find socket
 *
 * For core use only, finds a socket in the current list of socket
 *
 * @param sock_name the name of socket to find
 * 
 * @return pointer to socket if found, NULL if not found
 */
Sock *
find_sock (const char *sock_name)
{
	hnode_t *sn;

	sn = hash_lookup (sockethash, sock_name);
	if (sn)
		return hnode_get (sn);
	return NULL;
}

/** @brief add a socket to the socket list
 *
 * For core use. Adds a socket with the given functions to the socket list
 *
 * @param readfunc the name of read function to register with this socket
 * @param writefunc the name of write function to register with this socket
 * @param errfunc the name of error function to register with this socket
 * @param sock_name the name of socket to register
 * @param socknum the socket number to register with this socket
 * @param mod_name the name of module registering the socket
 * 
 * @return pointer to socket if found, NULL if not found
*/
int
add_sock (sock_func readfunc, sock_func writefunc, sock_func errfunc, const char *sock_name, int socknum)
{
	Sock *sock;
	Module* moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if (!readfunc) {
		nlog (LOG_WARNING, "add_sock: read socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NS_FAILURE;
	}
	if (!writefunc) {
		nlog (LOG_WARNING, "add_sock: write socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NS_FAILURE;
	}
	if (!errfunc) {
		nlog (LOG_WARNING, "add_sock: error socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NS_FAILURE;
	}
	sock = new_sock (sock_name);
	sock->sock_no = socknum;
	sock->moduleptr = moduleptr;
	sock->readfnc = readfunc;
	sock->writefnc = writefunc;
	sock->errfnc = errfunc;
	sock->socktype = SOCK_STANDARD;
	
	dlog(DEBUG2, "add_sock: Registered Module %s with Standard Socket functions %s", moduleptr->info->name, sock->name);
	return NS_SUCCESS;
}

/** @brief add a poll interface to the socket list
 *
 * For core use. Adds a socket with the given functions to the socket list
 *
 * @param beforepoll the name of function to call before we select
 * @param afterpoll the name of the function to call after we select
 * @param sock_name the name of socket to register
 * @param mod_name the name of module registering the socket
 * 
 * @return pointer to socket if found, NULL if not found
*/
int
add_sockpoll (before_poll_func beforepoll, after_poll_func afterpoll, const char *sock_name, void *data)
{
	Sock *sock;
	Module* moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if (!beforepoll) {
		nlog (LOG_WARNING, "add_sockpoll: read socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NS_FAILURE;
	}
	if (!afterpoll) {
		nlog (LOG_WARNING, "add_sockpoll: write socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NS_FAILURE;
	}
	sock = new_sock (sock_name);
	sock->moduleptr = moduleptr;
	sock->socktype = SOCK_POLL;
	sock->beforepoll = beforepoll;
	sock->afterpoll = afterpoll;
	sock->data = data;
	dlog(DEBUG2, "add_sockpoll: Registered Module %s with Poll Socket functions %s", moduleptr->info->name, sock->name);
	return NS_SUCCESS;
}

/** @brief delete a socket from the socket list
 *
 * For module use. Deletes a socket with the given name from the socket list
 *
 * @param socket_name the name of socket to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_sock (const char *sock_name)
{
	Sock *sock;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	sn = hash_lookup (sockethash, sock_name);
	if (sn) {
		sock = hnode_get (sn);
		dlog(DEBUG2, "del_sock: Unregistered Socket function %s from Module %s", sock_name, sock->moduleptr->info->name);
		hash_scan_delete (sockethash, sn);
		hnode_destroy (sn);
		sfree (sock);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete a socket from the socket list
 *
 * For module use. Deletes a socket with the given name from the socket list
 *
 * @param socket_name the name of socket to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_sockets (Module *mod_ptr)
{
	Sock *sock;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, sockethash);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		sock = hnode_get (modnode);
		if (sock->moduleptr == mod_ptr) {
			dlog(DEBUG1, "del_sockets: deleting socket %s from module %s.", sock->name, mod_ptr->info->name);
			del_sock (sock->name);
		}
	}
	return NS_SUCCESS;
}

/** @brief list sockets in use
 *
 * NeoStats command to list the current sockets from IRC
 *
 * @param u pointer to user structure of the user issuing the request
 * 
 * @return none
*/
int
list_sockets (CmdParams* cmdparams)
{
	Sock *sock = NULL;
	hscan_t ss;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, "Sockets List: (%d)", (int)hash_count (sockethash));
	hash_scan_begin (&ss, sockethash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		sock = hnode_get (sn);
		irc_prefmsg (ns_botptr, cmdparams->source, "%s:--------------------------------", sock->moduleptr->info->name);
		irc_prefmsg (ns_botptr, cmdparams->source, "Socket Name: %s", sock->name);
		if (sock->socktype == SOCK_STANDARD) {
			irc_prefmsg (ns_botptr, cmdparams->source, "Socket Number: %d", sock->sock_no);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, "Poll Interface");
		}
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "End of Socket List");
	return 0;
}

