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
#include "stats.h"
#include "dl.h"
#include <adns.h>
#include "conf.h"
#include "log.h"

void recvlog (char *line);

struct sockaddr_in lsa;
int dobind;

int
ConnectTo (char *host, int port)
{
	struct hostent *hp;
	struct sockaddr_in sa;
	int s;

	dobind = 0;
	/* bind to a local ip */
	memset (&lsa, 0, sizeof (lsa));
	if (strlen (me.local) > 1) {
		if ((hp = gethostbyname (me.local)) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "Warning, Couldn't bind to IP address %s", me.local);
		} else {
			memcpy ((char *) &lsa.sin_addr, hp->h_addr, hp->h_length);
			lsa.sin_family = hp->h_addrtype;
			dobind = 1;
		}
	}



	if ((hp = gethostbyname (host)) == NULL) {
		return (-1);
	}

	if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);
	if (dobind > 0) {
		if (bind (s, (struct sockaddr *) &lsa, sizeof (lsa)) < 0) {
			nlog (LOG_WARNING, LOG_CORE, "bind(): Warning, Couldn't bind to IP address %s", strerror (errno));
		}
	}


	bzero (&sa, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	bcopy (hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

	if (connect (s, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
		close (s);
		return (-1);
	}

	return s;
}

void
read_loop ()
{
	register int i, j, SelectResult;
	struct timeval *TimeOut, tvbuf;
	fd_set readfds, writefds, errfds;
	char c;
	char buf[BUFSIZE];
	Sock_List *mod_sock;
	hscan_t ss;
	hnode_t *sn;
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;

	TimeOut = malloc (sizeof (struct timeval));

	while (1) {
		SET_SEGV_LOCATION();
		memset (buf, '\0', BUFSIZE);
		chk ();
		SET_SEGV_LOCATION();
		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_ZERO (&errfds);
		FD_SET (servsock, &readfds);
		hash_scan_begin (&ss, sockh);
		me.cursocks = 1;	/* always one socket for ircd */
		while ((sn = hash_scan_next (&ss)) != NULL) {
			mod_sock = hnode_get (sn);
			if (mod_sock->readfnc)
				FD_SET (mod_sock->sock_no, &readfds);
			if (mod_sock->writefnc)
				FD_SET (mod_sock->sock_no, &writefds);
			if (mod_sock->errfnc)
				FD_SET (mod_sock->sock_no, &errfds);
			++me.cursocks;
		}
		/* adns stuff... whats its interested in */
		adns_beforeselect (ads, &me.maxsocks, &readfds, &writefds, &errfds, &TimeOut, &tvbuf, 0);
		/* adns may change this, but we tell it to go away!!! */
		TimeOut->tv_sec = 1;
		TimeOut->tv_usec = 0;
		SelectResult = select (FD_SETSIZE, &readfds, &writefds, &errfds, TimeOut);
		if (SelectResult > 0) {
			adns_afterselect (ads, me.maxsocks, &readfds, &writefds, &errfds, 0);

			/* do and dns related callbacks now */
			do_dns ();

			for (j = 0; j < BUFSIZE; j++) {
				if (FD_ISSET (servsock, &readfds)) {
					i = read (servsock, &c, 1);
					me.RcveBytes++;
					if (i >= 0) {
						buf[j] = c;
						if ((c == '\n')
						    || (c == '\r')) {
							me.RcveM++;
							me.lastmsg = time (NULL);
							if (config.recvlog)
								recvlog (buf);
							parse (buf);
							break;
						}
					} else {
						nlog (LOG_WARNING, LOG_CORE, "read returned a Error");
						return;
					}
				} else {
					/* this checks if there is any data waiting on a socket for a module */
					hash_scan_begin (&ss, sockh);
					while ((sn = hash_scan_next (&ss)) != NULL) {
						mod_sock = hnode_get (sn);
						SET_SEGV_INMODULE(mod_sock->modname);
						if (FD_ISSET (mod_sock->sock_no, &readfds)) {
							nlog (LOG_DEBUG3, LOG_CORE, "Running module %s readsock function for %s", mod_sock->modname, mod_sock->sockname);
							if (mod_sock->readfnc (mod_sock->sock_no, mod_sock->sockname) < 0)
								break;
						}
						if (FD_ISSET (mod_sock->sock_no, &writefds)) {
							nlog (LOG_DEBUG3, LOG_CORE, "Running module %s writesock function for %s", mod_sock->modname, mod_sock->sockname);
							if (mod_sock->writefnc (mod_sock->sock_no, mod_sock->sockname) < 0)
								break;
						}
						if (FD_ISSET (mod_sock->sock_no, &errfds)) {
							nlog (LOG_DEBUG3, LOG_CORE, "Running module %s errorsock function for %s", mod_sock->modname, mod_sock->sockname);
							if (mod_sock->errfnc (mod_sock->sock_no, mod_sock->sockname) < 0)
								break;
						}
					}
					CLEAR_SEGV_INMODULE();
					break;
				}
			}
		} else if (SelectResult == 0) {
			if ((time (NULL) - me.lastmsg) > 180) {
				/* if we havnt had a message for 3 minutes, more than likely, we are on a zombie server */
				/* disconnect and try to reconnect */
				/* Unload the Modules */
				hash_scan_begin (&ms, mh);
				while ((mn = hash_scan_next (&ms)) != NULL) {
					mod_ptr = hnode_get (mn);
					unload_module (mod_ptr->info->module_name, finduser (s_Services));
				}
				close (servsock);
				sleep (5);
				nlog (LOG_WARNING, LOG_CORE, "Eeek, Zombie Server, Reconnecting");
				do_exit (2);
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

extern int
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

void
recvlog (char *line)
{
	FILE *logfile;
	if ((logfile = fopen ("logs/recv.log", "a")) == NULL)
		return;
	if (logfile)
		fprintf (logfile, "%s", line);
	fclose (logfile);
}

char *
sctime (time_t stuff)
{
	char *s, *c;

	s = ctime (&stuff);
	if ((c = strchr (s, '\n')))
		*c = '\0';

	return s;
}

char fmtime[80];

char *
sftime (time_t stuff)
{
	struct tm *ltm = localtime (&stuff);

	strftime (fmtime, 80, "[%b (%a %d) %Y  %I:%M [%p/%Z]]", ltm);

	return fmtime;
}


int
sock_connect (int socktype, unsigned long ipaddr, int port, char *sockname, char *module, char *func_read, char *func_write, char *func_error)
{
	struct sockaddr_in sa;
	int s;
	int i;
/* socktype = SOCK_STREAM */

	if ((s = socket (AF_INET, socktype, 0)) < 0)
		return (-1);


	/* bind to a IP address */
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
		return (-1);
	}

	if ((i = connect (s, (struct sockaddr *) &sa, sizeof (sa))) < 0) {
		switch (errno) {
		case EINPROGRESS:
			break;
		default:
			nlog (LOG_WARNING, LOG_CORE, "Socket %s(%s) cant connect %s", sockname, module, strerror (errno), i);
			close (s);
			return (-1);
		}
	}

	add_socket (func_read, func_write, func_error, sockname, s, module);
	return s;
}

int
sock_disconnect (char *sockname)
{
	Sock_List *sock;
	fd_set fds;
	struct timeval tv;
	int i;

	sock = findsock (sockname);
	if (!sock) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Can not find Socket %s in list", sockname);
		return (-1);
	}

	/* the following code makes sure its a valid file descriptor */

	FD_ZERO (&fds);
	FD_SET (sock->sock_no, &fds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	i = select (1, &fds, NULL, NULL, &tv);
	if (!i && errno == EBADF) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Bad File Descriptor %s in list", sockname);
		return (-1);
	}
	nlog (LOG_DEBUG3, LOG_CORE, "Closing Socket %s with Number %d", sockname, sock->sock_no);
	close (sock->sock_no);
	del_socket (sockname);
	return (1);
}


void
sts (char *fmt, ...)
{
	va_list ap;
	char buf[512];
	int sent;
	va_start (ap, fmt);
	vsnprintf (buf, 512, fmt, ap);

	nlog (LOG_DEBUG2, LOG_CORE, "SENT: %s", buf);
	strcat (buf, "\n");
	sent = write (servsock, buf, strlen (buf));
	if (sent == -1) {
		nlog (LOG_CRITICAL, LOG_CORE, "Write error: %s", strerror(errno));
		do_exit (0);
	}
	me.SendM++;
	me.SendBytes = me.SendBytes + sent;
	va_end (ap);
}
