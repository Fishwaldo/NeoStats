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

#include "neostats.h"
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

static void recvlog (char *line);

/* @brief Module Socket List hash */
static hash_t *sockethash;
/* @brief server socket */
static int servsock;
char recbuf[BUFSIZE];
static struct timeval *TimeOut;
static struct pollfd *ufds;

rta_hook_func rta_hook_1 = NULL;
rta_hook_func rta_hook_2 = NULL;

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
		if (rta_hook_1) {
			rta_hook_1 (&readfds, &writefds);
		}  
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
			if (rta_hook_2) {
				rta_hook_2 (&readfds, &writefds);
			}
			if (FD_ISSET (servsock, &readfds)) {
				for (j = 0; j < BUFSIZE; j++) {
					i = os_sock_read (servsock, &c, 1);
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
	nlog (LOG_NOTICE, "Connecting to %s:%d", me.uplink, me.port);
	servsock = ConnectTo (me.uplink, me.port);
	if (servsock <= 0) {
		nlog (LOG_WARNING, "Unable to connect to %s", me.uplink);
	} else {
		/* Call the IRC specific function send_server_connect to login as a server to IRC */
		irc_connect (me.name, me.numeric, me.infoline, config.pass, (unsigned long)me.t_start, (unsigned long)me.now);
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

	lim = ns_calloc (sizeof (struct rlimit));
	getrlimit (RLIMIT_NOFILE, lim);
	ret = lim->rlim_max;
	ns_free (lim);
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

	add_sock (name, s, func_read, func_write, func_error);
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
	os_sock_close (sock->sock_no);
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
send_to_socket (const char *buf, const int buflen)
{
	int sent;

	if (servsock == -1) {
		nlog(LOG_WARNING, "Not sending to server as we have a invalid socket");
		return;
	}
	sent = os_sock_write (servsock, buf, buflen);
	if (sent == -1) {
		nlog (LOG_CRITICAL, "Write error: %s", strerror(errno));
		/* Try to close socket then reset the servsock value to avoid cyclic calls */
		os_sock_close (servsock);
		servsock = -1;
		do_exit (NS_EXIT_ERROR, NULL);
	}
	me.SendM++;
	me.SendBytes = me.SendBytes + sent;
}


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

void FiniSocks (void) 
{
	ns_free(TimeOut);
	ns_free(ufds);
	if (servsock > 0)
		os_sock_close (servsock);
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
	Sock *sock;

	sock = (Sock *)hnode_find (sockethash, sock_name);
	if (!sock) {
		dlog (DEBUG3, "find_sock: %s not found!", sock_name);		
	}
	return sock;
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
add_sock (const char *sock_name, int socknum, sock_func readfunc, sock_func writefunc, sock_func errfunc)
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
		if (sock->socktype == SOCK_STANDARD) {
			irc_prefmsg (ns_botptr, cmdparams->source, __("Socket Number: %d", cmdparams->source), sock->sock_no);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, __("Poll Interface", cmdparams->source));
		}
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("End of Socket List", cmdparams->source));
	return 0;
}

