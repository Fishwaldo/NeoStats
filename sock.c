/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
#include "stats.h"
#include "dl.h"
#include <adns.h>
#include "conf.h"
#include "log.h"
#include "timer.h"
#include "dns.h"
#include "transfer.h"
#include "curl.h"

static void recvlog (char *line);

static struct sockaddr_in lsa;
static int dobind;

int servsock;
char recbuf[BUFSIZE];

/** @brief Connect to a server
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

	TimeOut = malloc (sizeof (struct timeval));
	ufds = malloc((sizeof *ufds) *  me.maxsocks);

	while (1) {
		SET_SEGV_LOCATION();
		memset (buf, '\0', BUFSIZE);
		me.now = time(NULL);
		CheckTimers ();
		SET_SEGV_LOCATION();
		me.now = time(NULL);
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

		SelectResult = select (FD_SETSIZE, &readfds, &writefds, &errfds, TimeOut);
		me.now = time(NULL);
		if (SelectResult > 0) {
			/* check ADNS fds */
			adns_afterselect (ads, me.maxsocks, &readfds, &writefds, &errfds, 0);

			/* do any dns related callbacks now */
			do_dns ();

			/* check CURL fds */
			while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curlmultihandle, &maxfdsunused)) {
			}
			transfer_status();

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
		fprintf (logfile, line);
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
sts (char *fmt, ...)
{
	va_list ap;
	char buf[BUFSIZE];
	int sent;
	int buflen;
	
	if (servsock == -1) {
		nlog(LOG_WARNING, LOG_CORE, "Not Sending to Server as we have a invaild Socket");
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (buf, BUFSIZE, fmt, ap);
	va_end (ap);

	nlog (LOG_DEBUG2, LOG_CORE, "SENT: %s", buf);
	if(strnlen (buf, BUFSIZE) < BUFSIZE - 2) {
		strlcat (buf, "\n", BUFSIZE);
	} else {
		buf[BUFSIZE - 1] = 0;
		buf[BUFSIZE - 2] = '\n';
	}
	buflen = strnlen (buf, BUFSIZE);
	sent = write (servsock, buf, buflen);
	if (sent == -1) {
		nlog (LOG_CRITICAL, LOG_CORE, "Write error: %s", strerror(errno));
		do_exit (NS_EXIT_ERROR, NULL);
	}
	me.SendM++;
	me.SendBytes = me.SendBytes + sent;
}
