/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: sock.c,v 1.3 2000/02/23 05:39:24 fishwaldo Exp $
*/

#include "stats.h"

fd_set readfds, nullfds;

int ConnectTo(char *host, int port)
{
	struct hostent *hp;
	struct sockaddr_in sa;
	int s;

	if ((hp = gethostbyname (host)) == NULL) {
		return (-1);
	}

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);

	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
    bcopy(hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

	if (connect (s, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
		close (s);
		return (-1);
	}

	return s;
}

void read_loop()
{
	register int i, j, SelectResult;
	struct timeval TimeOut;
	char c;
	char buf[BUFSIZE];

	while (1) {
		segv_location = "Read_Loop";
		memset(buf, '\0', BUFSIZE);
		chk();
		FD_ZERO(&readfds);
		TimeOut.tv_sec = 1;
		TimeOut.tv_usec = 0;
		FD_SET(servsock, &readfds);
		SelectResult = select(FD_SETSIZE, &readfds, &nullfds, &nullfds, &TimeOut);
		if (SelectResult > 0) {
			for (j = 0; j < BUFSIZE; j++) {
				if (FD_ISSET(servsock, &readfds)) {
					i = read(servsock, &c, 1);
					if (i >= 0) {
						buf[j] = c;
						if ((c == '\n') || (c == '\r')) {
							parse(buf);
							break;
						}
					} else {
						log("read returned a Error");
						return;
					}
				} else {
					log("FDISSET failed");
					break;
				}
			}
		} else {
			if (SelectResult < 0) {
				log("Lost connection to server."); 
				return; 
			}
		}
	}
 log("hu, how did we get here");
}
void notice(char *who, char *buf,...)
{
	va_list ap;
	char tmp[512];
	char out[512];
	va_start (ap, buf);
	vsnprintf (tmp, 512, buf, ap);

	if (me.onchan) {
		sprintf(out,":%s PRIVMSG %s :%s",who, me.chan, tmp);
#ifdef DEBUG
		log("SENT: %s", out);
#endif

		strcat (out, "\n");

		if ((write (servsock, out, strlen (out))) == -1) {
			me.onchan = 0;
			log("Write error.");
			exit(0);
		}
	}
	va_end (ap);

}

void sts(char *fmt,...)
{
	va_list ap;
	char buf[512];

	va_start (ap, fmt);
	vsnprintf (buf, 512, fmt, ap);

#ifdef DEBUG
	log("SENT: %s", buf);
#endif
	strcat (buf, "\n");

	if ((write (servsock, buf, strlen (buf))) == -1) {
		log("Write error.");
		exit(0);
	}

	va_end (ap);
}

void log(char *fmt, ...)
{
	va_list ap;
	FILE *logfile;
	char buf[512], fmtime[80];
	time_t tmp = time(NULL);

	va_start(ap, fmt);
	vsnprintf(buf, 512, fmt, ap);

	strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));


	if ((logfile = fopen("stats.log", "a")) == NULL) return;

	if (logfile)
		fprintf(logfile, "(%s) %s\n", fmtime, buf);

#ifdef DEBUG
	fprintf(stderr, "(%s) %s\n", fmtime, buf);
#endif

	fclose(logfile);

	va_end(ap);
}

void ResetLogs()
{
	char tmp[25];
	time_t t = time(NULL);
	
	segv_location = "ResetLogs";
	strftime(tmp, 25, "logs/stats-%m-%d.log", localtime(&t));
	rename("stats.log", tmp);
	log("Started fresh logfile.");
}

char *sctime(time_t stuff)
{
	char *s, *c;
	
	s = ctime(&stuff);
	if ((c = strchr(s, '\n'))) *c = '\0';

	return s;
}

char fmtime[80];

char *sftime(time_t stuff)
{
	struct tm *ltm = localtime(&stuff);

	strftime(fmtime, 80, "[%b (%a %d) %Y  %I:%M [%p/%Z]]", ltm);

	return fmtime;
}
