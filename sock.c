/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: sock.c,v 1.16 2002/03/25 08:13:52 fishwaldo Exp $
*/

#include "stats.h"
#include "dl.h"

fd_set readfds, nullfds;

#ifdef RECVLOG
void recvlog(char *line);
#endif

void init_sock() {
	if (usr_mds);
}
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
	Sock_List *mod_sock;
	hscan_t ss;
	hnode_t *sn;
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;

	while (1) {
		strcpy(segv_location, "Read_Loop");
		memset(buf, '\0', BUFSIZE);
		chk();
		strcpy(segv_location, "Read_Loop2");
		FD_ZERO(&readfds);
		TimeOut.tv_sec = 1;
		TimeOut.tv_usec = 0;
		FD_SET(servsock, &readfds);
		hash_scan_begin(&ss, sockh);
		while ((sn = hash_scan_next(&ss)) != NULL) {
			mod_sock = hnode_get(sn);
			FD_SET(mod_sock->sock_no, &readfds);
		}
		SelectResult = select(FD_SETSIZE, &readfds, &nullfds, &nullfds, &TimeOut);
		if (SelectResult > 0) {
			for (j = 0; j < BUFSIZE; j++) {
				if (FD_ISSET(servsock, &readfds)) {
					i = read(servsock, &c, 1);
					me.RcveBytes++;
					if (i >= 0) {
						buf[j] = c;
						if ((c == '\n') || (c == '\r')) {
							me.RcveM++;
							me.lastmsg = time(NULL);
#ifdef RECVLOG
							recvlog(buf);
#endif
							parse(buf);
							break;
						}
					} else {
						log("read returned a Error");
						return;
					}
				} else {
				/* this checks if there is any data waiting on a socket for a module */
					hash_scan_begin(&ss, sockh);
					while ((sn = hash_scan_next(&ss)) != NULL) {
						mod_sock = hnode_get(sn);
						if (FD_ISSET(mod_sock->sock_no, &readfds)) {
							mod_sock->function();
						}
					}
					break;
				}
			}
		} else if (SelectResult == 0) {
			if ((time(NULL) - me.lastmsg) >	180) {
				/* if we havnt had a message for 3 minutes, more than likely, we are on a zombie server */
				/* disconnect and try to reconnect */
				/* Unload the Modules */
				hash_scan_begin(&ms, mh);
				while ((mn = hash_scan_next(&ms)) != NULL) {
					mod_ptr = hnode_get(mn);
					unload_module(mod_ptr->info->module_name, finduser(s_Services));
				}
				close(servsock);
				sleep(5);
				log("Eeek, Zombie Server, Reconnecting");
				execve("./stats", NULL, NULL);
			}
		} else if (SelectResult == -1) {
				log("Lost connection to server."); 
				return; 
		}
	}
 log("hu, how did we get here");
}
#ifdef RECVLOG
void recvlog(char *line)
{
	FILE *logfile;
	if ((logfile = fopen("logs/recv.log", "a")) == NULL) return;
	if (logfile)
		fprintf(logfile, "%s", line);
	fclose(logfile);
}
#endif

void log(char *fmt, ...)
{
	va_list ap;
	FILE *logfile;
	char buf[512], fmtime[80];
	time_t tmp = time(NULL);

	va_start(ap, fmt);
	vsnprintf(buf, 512, fmt, ap);

	strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));


	if ((logfile = fopen("logs/stats.log", "a")) == NULL) return;

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
	
	strcpy(segv_location, "ResetLogs");
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
