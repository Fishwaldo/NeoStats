/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "main.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_POLL_H 
#include <poll.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif
#include "timer.h"
#include "dns.h"
#include "transfer.h"
#include "curl.h"
#include "dotconf.h"
#include "services.h"
#include "ircd.h"
#include "rtaserv.h"
#include "dcc.h"
#include "sock.h"

/* @brief Module Socket List hash */
static hash_t *sockethash;

char recbuf[BUFSIZE];
static struct timeval *TimeOut;
static struct pollfd *ufds;


void write_from_ircd_socket (struct bufferevent *bufferevent, void *arg);


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

	me.dobind = 0;
	/* bind to a local ip */
	memset (&me.lsa, 0, sizeof (me.lsa));
	if (me.local[0] != 0) {
		if ((hp = gethostbyname (me.local)) == NULL) {
			nlog (LOG_WARNING, "Warning, Couldn't bind to IP address %s", me.local);
		} else {
			memcpy ((char *) &me.lsa.sin_addr, hp->h_addr, hp->h_length);
			me.lsa.sin_family = hp->h_addrtype;
			me.dobind = 1;
		}
	}
	if ((hp = gethostbyname (host)) == NULL) {
		return NS_FAILURE;
	}

	if ((s = (int)socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		ns_free(hp);
		return NS_FAILURE;
	}
	if (me.dobind > 0) {
		if (bind (s, (struct sockaddr *) &me.lsa, sizeof (me.lsa)) < 0) {
			nlog (LOG_WARNING, "bind(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}

	memset (&sa, 0, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	memcpy ((char *) &sa.sin_addr, hp->h_addr, hp->h_length);

	ret=connect (s, (struct sockaddr *) &sa, sizeof (sa));
	if (ret< 0) {
#ifdef WIN32
		nlog (LOG_ERROR, "Winsock error: %d", WSAGetLastError());
#endif
		os_sock_close (s);
		return NS_FAILURE;
	}
	return s;
}

/** @brief update the me.now struct 
 * @param none
 * @return none
 */
static void 
update_time() {
	me.now = time(NULL);
	ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
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
	printf("readloop called\n");
	me.lastmsg = me.now;
	while (1) { /* loop till we get a error */
		SET_SEGV_LOCATION();
		update_time();
		event_dispatch();
		printf("loop1\n");
		SET_SEGV_LOCATION();
	}


#if 0
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
		rta_hook_1 (&readfds, &writefds);
		dcc_hook_1 (&readfds, &writefds);
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
			rta_hook_2 (&readfds, &writefds);
			dcc_hook_2 (&readfds, &writefds);
			if (FD_ISSET (servsock, &readfds)) {
				for (j = 0; j < BUFSIZE; j++) {
					i = os_sock_read (servsock, &c, 1);
					me.RcveBytes++;
					if (i >= 0) {
						buf[j] = c;
						if ((c == '\n') || (c == '\r')) {
							me.RcveM++;
							me.lastmsg = me.now;
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
					int err;

					pollflag = 0;
					sock = hnode_get (sn);
					if (sock->socktype == SOCK_STANDARD) {
						if (FD_ISSET (sock->sock_no, &readfds)) {
							dlog(DEBUG3, "Running module %s readsock function for %s", sock->moduleptr->info->name, sock->name);
							SET_RUN_LEVEL(sock->moduleptr);
							err = sock->readfnc (sock->sock_no, sock->name);
							RESET_RUN_LEVEL();
							if(err < 0)
								continue;
						}
						if (FD_ISSET (sock->sock_no, &writefds)) {
							dlog(DEBUG3, "Running module %s writesock function for %s", sock->moduleptr->info->name, sock->name);
							SET_RUN_LEVEL(sock->moduleptr);
							err = sock->writefnc (sock->sock_no, sock->name);
							RESET_RUN_LEVEL();
							if(err < 0)
								continue;
						}
						if (FD_ISSET (sock->sock_no, &errfds)) {
							dlog(DEBUG3, "Running module %s errorsock function for %s", sock->moduleptr->info->name, sock->name);
							SET_RUN_LEVEL(sock->moduleptr);
							err = sock->errfnc (sock->sock_no, sock->name);
							RESET_RUN_LEVEL();
							if(err < 0)
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
							SET_RUN_LEVEL(sock->moduleptr);
							sock->afterpoll(sock->data, ufds, pollsize);
							RESET_RUN_LEVEL();
						}
					}
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
#endif /* new sock code */
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
	int mysock;
	SET_SEGV_LOCATION();
	nlog (LOG_NOTICE, "Connecting to %s:%d", me.uplink, me.port);
	mysock = ConnectTo (me.uplink, me.port);
	if (mysock <= 0) {
		nlog (LOG_WARNING, "Unable to connect to %s: %s", me.uplink, strerror(errno));
	} else {
		me.servsock=add_linemode_socket("IRCd", mysock, irc_parse, write_from_ircd_socket, error_from_ircd_socket);

		/* Call the IRC specific function send_server_connect to login as a server to IRC */
		irc_connect (me.name, me.numeric, me.infoline, nsconfig.pass, (unsigned long)me.ts_boot, (unsigned long)me.now);
#ifndef WIN32
		read_loop ();
#endif
	}
#ifndef WIN32
	do_reconnect();
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

	lim = ns_calloc (sizeof (struct rlimit));
	getrlimit (RLIMIT_NOFILE, lim);
	ret = lim->rlim_max;
	ns_free (lim);
	if(ret<0)
		ret = 0xffff;
	return ret;
#endif
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
	if (me.dobind > 0) {
		if (bind (s, (struct sockaddr *) &me.lsa, sizeof (me.lsa)) < 0) {
			nlog (LOG_WARNING, "sock_connect(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}

	memset (&sa, 0, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	sa.sin_addr.s_addr = ipaddr;

	/* set non blocking */

	if ((i = os_sock_set_nonblocking (s)) < 0) {
		nlog (LOG_CRITICAL, "can't set socket %s(%s) non-blocking: %s", name, moduleptr->info->name, strerror (i));
		return NS_FAILURE;
	}

	if ((i = connect (s, (struct sockaddr *) &sa, sizeof (sa))) < 0) {
		switch (errno) {
		case EINPROGRESS:
			break;
		default:
			nlog (LOG_WARNING, "Socket %s(%s) cant connect %s", name, moduleptr->info->name, strerror (errno));
			os_sock_close (s);
			return NS_FAILURE;
		}
	}

#warning TODO
#if 0
	add_sock (name, s, func_read, func_write, func_error);
#endif
	return s;
}

#if 0
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
	os_sock_close (sock->sock_no);
	del_sock (name);
	return NS_SUCCESS;
}
#endif
/** @brief send to socket
 *
 * @param buf the text we want to send to the IRC Server
 * @param buflen the size of the text we are sending
 * 
 * @return none
 */
void
send_to_ircd_socket (const char *buf, const int buflen)
{
	int sent;

	if (!me.servsock) {
		nlog(LOG_WARNING, "Not sending to server as we have a invalid socket");
		return;
	}
	/* the ircd socket is a buffered socket */
	sent = bufferevent_write(me.servsock->event.buffered, (void *)buf, buflen);
	if (sent == -1) {
		nlog (LOG_CRITICAL, "Write error: %s", strerror(errno));
		/* Try to close socket then reset the servsock value to avoid cyclic calls */
		del_sock(me.servsock);
		me.servsock = NULL;
		/* XXX really exit? */
		do_exit (NS_EXIT_ERROR, NULL);
	}
	me.servsock->smsgs++;
	me.servsock->sbytes += buflen;
}

#undef SOCKDEBUG 

/** @brief This function actually reads the data from our event buffer and 
 *  places the data in a temporary buffer assocated with this socket
 *  it then checks teh temporary buffer for new line charactors and if found
 *  sends the line of text to the function specified when this socket was created
 *
 * @param bufferevent the bufferevent structure direct from the event subsystem
 * @param arg is actually the Sock structure for this socket.
 * 
 * @return Nothing
 *
 */
void
linemode_read(struct bufferevent *bufferevent, void *arg) {
	Sock *thisock = (Sock*)arg;

	char buf[512];
	size_t len;
	int bufpos;
#ifdef SOCKDEBUG
	printf("Start Reading\n");
#endif

	while ((len = bufferevent_read(bufferevent, buf, 512)) > 0) {
#ifdef SOCKDEBUG
		printf("Buffer: %d |%s|\n\n", thisock->linemode->readbufsize, thisock->linemode->readbuf);
		printf("RecievedSize %d |%s|\n\n", len, buf);
#endif
		if (len > 0) {
			/* firstly, if what we have read plus our existing buffer size exceeds our recvq
			 * we should drop the connection
			 */
			if ((len + thisock->sfunc.linemode.readbufsize) > thisock->sfunc.linemode.recvq) {
				nlog(LOG_ERROR, "RecvQ for %s(%s) exceeded. Dropping Connection", thisock->name, thisock->moduleptr?thisock->moduleptr->info->name:"N/A");
				del_sock(thisock);
				/* XXX really exit? */
				do_exit (NS_EXIT_ERROR, NULL);
			}
			thisock->rbytes += len;

			for (bufpos = 0; bufpos < len; bufpos++) {
				/* if the static buffer now contains newline chars,
				 * we are ready to parse the line 
				 * else we just keep putting the recieved charactors onto the buffer
				 */
				if ((buf[bufpos] == '\n') || (buf[bufpos] == '\r')) {
					/* only process if we have something to do. This might be a trailing NL or CR char*/
					if (thisock->sfunc.linemode.readbufsize > 0) {
						/* update some stats */
						thisock->rmsgs++;
#if 0
						me.lastmsg = me.now;
#endif
						/* make sure its null terminated */
						thisock->sfunc.linemode.readbuf[thisock->sfunc.linemode.readbufsize++] = '\0';
						/* copy it to our recbuf for debugging */
						strlcpy(recbuf, thisock->sfunc.linemode.readbuf, BUFSIZE);
						/* parse the buffer */
#ifdef SOCKDEBUG
						printf("line:|%s| %d %d\n", thisock->sfunc.linemode.readbuf, thisock->sfunc.linemode.readbufsize, bufpos);
#endif
						thisock->sfunc.linemode.funccb(thisock->sfunc.linemode.readbuf);
						/* ok, reset the recbuf */
						thisock->sfunc.linemode.readbufsize = 0;
					}
				} else {
					/* add it to our static buffer */
					thisock->sfunc.linemode.readbuf[thisock->sfunc.linemode.readbufsize] = buf[bufpos];
					thisock->sfunc.linemode.readbufsize++;
				}
			}
			thisock->sfunc.linemode.readbuf[thisock->sfunc.linemode.readbufsize] = '\0';
		}
	}
}


/** @brief This function signals that the socket has sent all available data 
 *  in the write buffer. We don't actually use it Currently, but we could
 *
 * @param bufferevent the bufferevent structure direct from the event subsystem
 * @param arg is actually the Sock structure for this socket.
 * 
 * @return Nothing
 *
 */


void
write_from_ircd_socket (struct bufferevent *bufferevent, void *arg) {
/* NOOP - We require this otherwise the event subsystem segv's */
}


/** @brief This function signals that a error has occured on the socket
 *  such as EOF etc. We use this to detect disconnections etc rather than 
 *  having to worry about the return code from a read function call
 *
 * @param bufferevent the bufferevent structure direct from the event subsystem
 * @param what indicates what caused this error, a read or write call
 * @param arg is actually the Sock structure for this socket.
 * 
 * @return Nothing
 *
 */


void 
error_from_ircd_socket(struct bufferevent *bufferevent, short what, void *arg) {
#if 0
	Sock *sock = (Sock*)arg;
#endif	
	switch (what) {
		case EVBUFFER_READ:
		case EVBUFFER_WRITE:
		case EVBUFFER_EOF:
		case EVBUFFER_ERROR:
		case EVBUFFER_TIMEOUT:
			nlog(LOG_ERROR, "error_from_ircd_socket: Error: %d", what);
			break;
		default:
			nlog(LOG_ERROR, "Unknown Error from IRCd Socket: %d", what);
			break;
	}
#warning this needs to be cleaned up so we can exit cleanly.
	del_sock(me.servsock);
	me.servsock = NULL;
	/* XXX really exit? */
	do_exit (NS_EXIT_ERROR, NULL);
}	
	

/** @brief Init the socket subsystem
 *
 * 
 * @return Nothing
 *
 */

int InitSocks (void)
{
	me.maxsocks = getmaxsock ();
	sockethash = hash_create (me.maxsocks, 0, 0);
	if(!sockethash) {
		nlog (LOG_CRITICAL, "Unable to create socks hash");
		return NS_FAILURE;
	}
	TimeOut = ns_calloc (sizeof (struct timeval));
	ufds = ns_calloc ((sizeof *ufds) *  me.maxsocks);
	return NS_SUCCESS;
}


/** @brief Finish up the socket subsystem
 *
 * 
 * @return Nothing
 *
 */


void FiniSocks (void) 
{
	ns_free(TimeOut);
	ns_free(ufds);
#warning we should actually scan for any existing sockets still registered here
	if (me.servsock) {
		del_sock(me.servsock);
		me.servsock = NULL;
	}
	hash_destroy(sockethash);
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

	SET_SEGV_LOCATION();
	if (hash_isfull (sockethash)) {
		nlog (LOG_CRITICAL, "new_sock: socket hash is full");
		return NULL;
	}
	dlog(DEBUG2, "new_sock: %s", sock_name);
	sock = ns_calloc (sizeof (Sock));
	strlcpy (sock->name, sock_name, MAX_MOD_NAME);
	hnode_create_insert (sockethash, sock, sock->name);
	return sock;
}

/** @brief find socket
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
	Sock *sock;

	/* XXX shouldn't hnode_get be here? */
	sock = (Sock *)hnode_find (sockethash, sock_name);
	if (!sock) {
		dlog (DEBUG3, "find_sock: %s not found!", sock_name);		
	}
	return sock;
}

/** @brief create a new socket that's protocol is based on lines
 *
 * This sets up the core to create a buffered, newline terminated communication. 
 * The socket connection should have already been established, and socknum should be a valid
 * socket. 
 *
 * @param sock_name the name of the new socket
 * @param socknum the Socket number, from the socket function call
 * @param readcb function prototype that is called when a new "newline" terminated string is recieved.
 * @param writecb function to call that indicates we have sent all buffered data
 * @param errcb function to call when we get a error, such as EOF 
 * 
 * @return pointer to socket structure for reference to future calls
 */

Sock *
add_linemode_socket(const char *sock_name, int socknum, linemodecb readcb, evbuffercb writecb, everrorcb errcb) {
	Sock *sock;
	sock = add_buffered_socket(sock_name, socknum, linemode_read, writecb, errcb);
	if (sock) {
		if (nsconfig.recvq < 1024) {
			nsconfig.recvq = 1024;
		}
		sock->sfunc.linemode.readbuf = os_malloc(nsconfig.recvq);
		sock->sfunc.linemode.recvq = nsconfig.recvq;
		sock->sfunc.linemode.funccb = readcb;
		sock->sfunc.linemode.readbufsize = 0;
		sock->socktype = SOCK_LINEMODE;
		dlog(DEBUG3, "add_linemode_socket: Added a new Linemode Socket called %s (%s)", sock_name, sock->moduleptr?sock->moduleptr->info->name:"N/A");
	}
	return sock;
}


/** @brief create a new socket that's input and output are buffered.
 *
 * This sets up the core to create a buffered communication. 
 * The socket connection should have already been established, and socknum should be a valid
 * socket. 
 *
 * @param sock_name the name of the new socket
 * @param socknum the Socket number, from the socket function call
 * @param readcb function prototype that is called when a new buffered data is ready.
 * @param writecb function to call that indicates we have sent all buffered data
 * @param errcb function to call when we get a error, such as EOF 
 * 
 * @return pointer to socket structure for reference to future calls
 */


Sock *
add_buffered_socket(const char *sock_name, int socknum, evbuffercb readcb, evbuffercb writecb, everrorcb errcb) {
	Sock *sock;
	Module *moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if (!readcb) {
		nlog(LOG_WARNING, "add_buffered_socket: read buffer function doesn't exist = %s (%s)", sock_name, moduleptr?moduleptr->info->name:"N/A");
		return NULL;
	}
	if (!errcb) {
		nlog(LOG_WARNING, "add_buffered_socket: error function doesn't exist = %s (%s)", sock_name, moduleptr?moduleptr->info->name:"N/A");
		return NULL;
	}
	/* write callback is not mandatory */
	
	sock = new_sock(sock_name);
	sock->sock_no = socknum;
	sock->moduleptr = moduleptr;
	sock->socktype = SOCK_BUFFERED;
	
	/* add the buffered socket to the core event subsystem */
	sock->event.buffered = bufferevent_new(sock->sock_no, readcb, writecb, errcb, sock);
	if (!sock->event.buffered) {
		nlog(LOG_WARNING, "bufferevent_new() failed");
		del_sock(sock);
		return NULL;
	}	
	bufferevent_enable(sock->event.buffered, EV_READ|EV_WRITE);	
	bufferevent_setwatermark(sock->event.buffered, EV_READ|EV_WRITE, 0, 0);
	dlog(DEBUG3, "add_buffered_sock: Registered Module %s with Standard Socket functions %s", moduleptr?moduleptr->info->name:"core", sock->name);
	return sock;
}


/** @brief call the function to accept a new connection and handle it gracefully!
 *
 * This is for use only in the core, and not by modules
 *
 * @param fd the socket number
 * @param what ignored
 * @param arg is a pointer to the Sock structure for the listening socket
 * 
 * @return nothing
 */

static void
listen_accept_sock(int fd, short what, void *arg) {
	Sock *sock = (Sock *)arg;
	
	SET_SEGV_LOCATION();
	dlog(DEBUG1, "Got Activity on Listen Socket %d port %d (%s)", sock->sock_no, sock->sfunc.listenmode.port, sock->name);
	if (sock->sfunc.listenmode.funccb(sock->sock_no, sock->data) == NS_SUCCESS) {
		/* re-add this listen socket if the acceptcb succeeds */
		event_add(sock->event.event, NULL);
		sock->rmsgs++;
		return;
	} else {
		dlog(DEBUG1, "Deleting Listen Socket %d port %d (%s)", sock->sock_no, sock->sfunc.listenmode.port, sock->name);
		del_sock(sock);
	}
}


/** @brief create a new socket to listen on a port (with standard bindings)
 *
 * This creates a new socket that will listen on a standard port and will automatically 
 * bind to the local host if specified in the configuration
 *
 * @param sock_name the socket name
 * @param port a integer of the port to listen on.
 * @param acceptcb the function to call to accept a new connection. Responsible for actually accepting the connection
 * and createing the new Socket interface
 * @param data A void pointer that is passed back to the user via the acceptcb callback
 * 
 * @return A socket struct that represents the new Listening Socket
 */

Sock *
add_listen_sock(const char *sock_name, const int port, int type, sockcb acceptcb, void *data) {
	OS_SOCKET srvfd;      /* FD for our listen server socket */
	struct sockaddr_in srvskt;
	int      adrlen;
	Sock *sock;
	Module *moduleptr;
	
	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	
	adrlen = sizeof(struct sockaddr_in);
	(void) memset((void *) &srvskt, 0, (size_t) adrlen);
	srvskt.sin_family = AF_INET;
	/* bind to the local IP */
	if (me.dobind) {
		srvskt.sin_addr = me.lsa.sin_addr;
	} else {
		srvskt.sin_addr.s_addr = INADDR_ANY;
	}
	srvskt.sin_port = htons(port);
	if ((srvfd = socket(AF_INET, type, 0)) < 0)
	{
		nlog(LOG_CRITICAL, "Unable to get socket for port %d. (%s)", port, moduleptr->info->name);
		return NULL;
	}
	os_sock_set_nonblocking (srvfd);
	if (bind(srvfd, (struct sockaddr *) &srvskt, adrlen) < 0)
	{
		nlog(LOG_CRITICAL, "Unable to bind to port %d (%s)", port, moduleptr->info->name);
		return NULL;
	}
	if (listen(srvfd, 1) < 0)
	{
		nlog(LOG_CRITICAL, "Unable to listen on port %d (%s)", port, moduleptr->info->name);
		return NULL;
	}
	sock = new_sock(sock_name);
	sock->sock_no = srvfd;
	sock->moduleptr = moduleptr;
	sock->socktype = SOCK_LISTEN;
	sock->data = data;
	sock->sfunc.listenmode.port = port;
	sock->sfunc.listenmode.funccb = acceptcb;
	sock->event.event = os_malloc(sizeof(struct event));
	
	event_set(sock->event.event, sock->sock_no, EV_READ, listen_accept_sock, (void*) sock);
	event_add(sock->event.event, NULL);
	
	return (sock);

}

/** @brief Read Data from a "STANDARD" Socket
 *
 * This function reads data from a standard socket and will handle all necessary errors
 * When it has read the data, it calls the callback with the data that has been read from the socket 
 *
 * @param fd the socket number
 * @param what ignored
 * @param data pointer to the Sock Structure for the the socket thats being read currently
 * 
 * @return Nothing
 */

void
read_sock_activity(int fd, short what, void *data) {
	Sock *sock = (Sock *)data;
	u_char *p = NULL;
	int n;
	size_t howmuch = READBUFSIZE;
#ifdef WIN32
#error Mark, you need to write the win32 support functions for reading here. if any.
	DWORD dwBytesRead;
#endif

	if (what & EV_READ) { 	
#ifdef FIONREAD
			if (ioctl(sock->sock_no, FIONREAD, &howmuch) == -1)
				howmuch = READBUFSIZE;
#else 
#warning FIONREAD not available
#endif	
			if (howmuch > 0) {
				p = os_malloc(howmuch);
			}
#if SOCKDEBUG
printf("read called with %d bytes %d\n", howmuch, what);
#endif

#ifndef WIN32
			n = read(sock->sock_no, p, howmuch);
			if (n == -1) {
				dlog(DEBUG1, "sock_read: Read Failed with %s on fd %d (%s)", strerror(errno), sock->sock_no, sock->name);
				sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
				del_sock(sock);
				return;
			}
			if (n == 0) {
				dlog(DEBUG1, "sock_read: Read Failed with %s on fd %d (%s)", strerror(errno), sock->sock_no, sock->name);
				sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
				del_sock(sock);
				return;
			}
#else
			n = ReadFile((HANDLE)sock->sock_no, p, howmuch, &dwBytesRead, NULL);
			if (n == 0) {
				dlog(DEBUG1, "sock_read: Read Failed with %s on fd %d (%s)", strerror(errno), sock->sock_no, sock->name);
				sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
				del_sock(sock);
				return;
			}
			if (dwBytesRead == 0)
				return;
			n = dwBytesRead;
#endif
			dlog(DEBUG1, "sock_read: Read %d bytes from fd %d (%s)", n, sock->sock_no, sock->name);
			sock->rbytes += n;
			/* rmsgs is just a counter for how many times we read in the standard sockets */
			sock->rmsgs++;
			if (sock->sfunc.standmode.readfunc(sock->data, p, n) == NS_FAILURE) {
				dlog(DEBUG1, "sock_read_activity: Read Callback failed, Closing Socket on fd %d (%s)", sock->sock_no, sock->name);
				del_sock(sock);
				return;
			}			
	} else if (what & EV_WRITE) {
			if ((sock->sfunc.standmode.writefunc(fd, sock->data)) == NS_FAILURE) {
				dlog(DEBUG1, "sock_read: Write Callback Failed, Closing Socket on fd %d (%s)", sock->sock_no, sock->name);
				del_sock(sock);
				return;
			}			
	} else if (what & EV_TIMEOUT) {
			dlog(DEBUG1, "sock_read_activity: Timeout on %d (%s)", sock->sock_no, sock->name);
			if (sock->sfunc.standmode.readfunc(sock->data, NULL, -2) == NS_FAILURE) {
				dlog(DEBUG1, "sock_read: Timeout Read Callback Failed, Closing Socket on fd %d (%s)", sock->sock_no, sock->name);
				del_sock(sock);
				return;
			}			
	} else {
			nlog(LOG_WARNING, "Error, Unknown State in sock_read_activity %d", what);
			sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
			del_sock(sock);
	}							
			
}

/** @brief update the read/write calls for this socket
 *
 * Updates the event subsystem if we want to signal we are now interested
 * in new events 
 *
 * @param sock The socket structure we want to update
 * @param what the new events we want to monitor 
 * 
 * @return success or failure
*/
int 
update_sock(Sock *sock, short what, short reset, struct timeval *tv) {

	if (reset) {
		dlog(DEBUG1, "Event for fd %d (%s) has been reset", sock->sock_no, sock->name);
		event_del(sock->event.event);
	}
	if ((!sock->sfunc.standmode.readfunc) && (what & EV_READ)) {
		nlog (LOG_WARNING, "update_sock: read socket function doesn't exist = %s (%s)", sock->name, sock->moduleptr->info->name);
		return NS_FAILURE;
	}
	if ((!sock->sfunc.standmode.writefunc) && (what & EV_WRITE)) {
		nlog (LOG_WARNING, "update_sock: write socket function doesn't exist = %s (%s)", sock->name, sock->moduleptr->info->name);
		return NS_FAILURE;
	}
	if ((!tv) && (what & EV_TIMEOUT)) {
		nlog(LOG_WARNING, "update_sock: Timeout wanted but not specified on %s (%s)", sock->name, sock->moduleptr->info->name);
		return NS_FAILURE;
	}
	event_set(sock->event.event, sock->sock_no, what, read_sock_activity, (void*) sock);
	event_add(sock->event.event, tv);
	dlog(DEBUG1, "Updated Socket Info %s with new event %d", sock->name, what);
	return NS_SUCCESS;
}
/** @brief add a socket to the socket list as a "Standard" socket
 *
 * For core use. Adds a socket with the given functions to the socket list
 *
 * @param readfunc the name of read function to register with this socket
 * @param writefunc the name of write function to register with this socket
 * @param sock_name the name of socket to register
 * @param socknum the socket number to register with this socket
 * @param data User supplied data for tracking purposes.
 * 
 * @return pointer to socket if found, NULL if not found
*/
Sock *
add_sock (const char *sock_name, int socknum, sockfunccb readfunc, sockcb writefunc, short what, void *data, struct timeval *tv)
{
	Sock *sock;
	Module* moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if ((!readfunc) && (what & EV_READ)) {
		nlog (LOG_WARNING, "add_sock: read socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NULL;
	}
	if ((!writefunc) && (what & EV_WRITE)) {
		nlog (LOG_WARNING, "add_sock: write socket function doesn't exist = %s (%s)", sock_name, moduleptr->info->name);
		return NULL;
	}
	if ((!tv) && (what & EV_TIMEOUT)) {
		nlog(LOG_WARNING, "add_sock: Timeout wanted but not specified on %s (%s)", sock_name, moduleptr->info->name);
		return NULL;
	}
	sock = new_sock (sock_name);
	sock->sock_no = socknum;
	sock->moduleptr = moduleptr;
	sock->data = data;
	sock->sfunc.standmode.readfunc = readfunc;
	sock->sfunc.standmode.writefunc = writefunc;
	sock->socktype = SOCK_STANDARD;

	sock->event.event = os_malloc(sizeof(struct event));
	event_set(sock->event.event, sock->sock_no, what, read_sock_activity, (void*) sock);
	event_add(sock->event.event, tv);

	dlog(DEBUG2, "add_sock: Registered Module %s with Standard Socket functions %s", moduleptr->info->name, sock->name);
	return sock;
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
add_sockpoll (const char *sock_name, void *data, before_poll_func beforepoll, after_poll_func afterpoll)
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
del_sock (Sock *sock)
{
	hnode_t *sn;

	SET_SEGV_LOCATION();
	switch (sock->socktype) {
		case SOCK_STANDARD:
		case SOCK_LISTEN:
				event_del(sock->event.event);
				/* needed? */
				os_free(sock->event.event);
				os_sock_close (sock->sock_no);
				break;
		case SOCK_LINEMODE:
				os_free(sock->sfunc.linemode.readbuf);
		case SOCK_BUFFERED:
				bufferevent_free(sock->event.buffered);
				os_sock_close (sock->sock_no);
				break;
	}				
	
	sn = hash_lookup (sockethash, sock->name);
	if (sn) {
		sock = hnode_get (sn);
		dlog(DEBUG2, "del_sock: Unregistered Socket function %s from Module %s", sock->name, sock->moduleptr->info->name);
		hash_scan_delete (sockethash, sn);
		hnode_destroy (sn);
		ns_free (sock);
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
			del_sock (sock);
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
ns_cmd_socklist (CmdParams* cmdparams)
{
	Sock *sock = NULL;
	hscan_t ss;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("Sockets List: (%d)", cmdparams->source), (int)hash_count (sockethash));
	hash_scan_begin (&ss, sockethash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		sock = hnode_get (sn);
		irc_prefmsg (ns_botptr, cmdparams->source, "%s:--------------------------------", sock->moduleptr->info->name);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Socket Name: %s", cmdparams->source), sock->name);
		switch (sock->socktype) {
			case SOCK_STANDARD:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Standard Socket - fd: %d", cmdparams->source), sock->sock_no);
				break;
			case SOCK_POLL:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Poll Interface", cmdparams->source));
				break;
			case SOCK_BUFFERED:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Buffered Socket - fd: %d", cmdparams->source), sock->sock_no);
				break;
			case SOCK_LINEMODE:
				irc_prefmsg (ns_botptr, cmdparams->source, __("LineMode Socket - fd: %d", cmdparams->source), sock->sock_no);
				irc_prefmsg (ns_botptr, cmdparams->source, __("RecieveQ Set: %d Current: %d", cmdparams->source), sock->sfunc.linemode.recvq, sock->sfunc.linemode.readbufsize);	
				break;
			case SOCK_LISTEN:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Listen Socket - fd %d Port %d", cmdparams->source), sock->sock_no, sock->sfunc.listenmode.port);
				irc_prefmsg (ns_botptr, cmdparams->source, __("Accepted Connections: %ld", cmdparams->source), sock->rmsgs);
				break;
			default:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Uknown (Error!)", cmdparams->source));
				break;
		}
		irc_prefmsg (ns_botptr, cmdparams->source, __("Recieved Bytes: %ld, Recieved Messages: %ld", cmdparams->source), sock->rbytes, sock->rmsgs);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Sent Bytes: %ld, Sent Messages: %ld", cmdparams->source), sock->sbytes, sock->smsgs);
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("End of Socket List", cmdparams->source));
	return 0;
}

