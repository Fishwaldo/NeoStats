/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: proxy.c,v 1.3 2002/08/22 13:53:08 fishwaldo Exp $
*/


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <fcntl.h>
#include "dl.h"
#include "stats.h"
#include "opsb.h"

int http_proxy(int sock);
int sock4_proxy(int sock);
int sock5_proxy(int sock);
int cisco_proxy(int sock);
int wingate_proxy(int sock);
int proxy_connect(unsigned long ipaddr, int port, char *who);

typedef struct proxy_types {
	char *type;
	int port;
	int (*scan)(int sock);
	int nofound;
	int noopen;
} proxy_types;

proxy_types proxy_list[] = {
	{"http", 	80, 	http_proxy, 	0,	0},
	{"http",	8080,	http_proxy,	0,	0},
	{"http",	3128,	http_proxy, 	0,	0},
	{"socks4",	1080,	sock4_proxy,	0,	0},
	{"socks5",	1080,	sock5_proxy,	0,	0},
	{"Cisco",	23,	cisco_proxy, 	0,	0},
	{"Wingate",	23,	wingate_proxy,	0,	0},
	{NULL,		0,	NULL,		0,	0}
};

#define NUM_PROXIES 7



scaninfo *find_scandata(char *sockname) {
	char *buf, *cmd;
	lnode_t *scannode;
	
	buf = sstrdup(sockname);
	cmd = strtok(buf, " ");

	scannode = list_find(opsbl, cmd, findscan);
	if (scannode)
		return lnode_get(scannode);
	else 
		return NULL;
}


void send_status(User *u) {
	int i;
	lnode_t *node, *socknode;
	scaninfo *scandata;
	socklist *sockinfo;
	prefmsg(u->nick, s_opsb, "Proxy Results:");
	prefmsg(u->nick, s_opsb, "Hosts Scanned: %d Hosts found Open: %d Exceptions %d", opsb.scanned, opsb.open, list_count(exempt));
	for (i = 0; i < NUM_PROXIES; i++) {
		prefmsg(u->nick, s_opsb, "Proxy %s (%d) Found %d Open %d", proxy_list[i].type, proxy_list[i].port, proxy_list[i].nofound, proxy_list[i].noopen);
	}
	prefmsg(u->nick, s_opsb, "Currently Scanning %d Proxies (%d in queue):", list_count(opsbl), list_count(opsbq));
	node = list_first(opsbl);
	while (node) {
		scandata = lnode_get(node);
		if (scandata->u) 
			prefmsg(u->nick, s_opsb, "Scanning %s by request of %s", scandata->lookup, scandata->u->nick);
		else 
			prefmsg(u->nick, s_opsb, "Scanning %s", scandata->lookup);
		
		switch(scandata->state) {
			case REPORT_DNS:
					prefmsg(u->nick, s_opsb, "Looking up IP Address");
					break;
			case GET_NICK_IP:
					prefmsg(u->nick, s_opsb, "Looking up IP address for Scan");
					break;
			case DO_OPM_LOOKUP:
					prefmsg(u->nick, s_opsb, "Looking up DNS blacklist");
					break;
			case DOING_SCAN:
					prefmsg(u->nick, s_opsb, "Scanning for Open Proxies");
					break;
			case GOTOPENPROXY:
					prefmsg(u->nick, s_opsb, "Contains a Open Proxy");
					break;
			default:
					prefmsg(u->nick, s_opsb, "Unknown State");
		}
		socknode = list_first(scandata->socks);
		while (socknode) {
			sockinfo = lnode_get(socknode);
			switch (sockinfo->flags) {
				case CONNECTING:
						prefmsg(u->nick, s_opsb, "    %s(%d) - Connecting", proxy_list[sockinfo->type].type, proxy_list[sockinfo->type].port);
						break;
				case SOCKCONNECTED:
						prefmsg(u->nick, s_opsb, "    %s(%d) - Connected", proxy_list[sockinfo->type].type, proxy_list[sockinfo->type].port);
						break;
				case UNCONNECTED:
						prefmsg(u->nick, s_opsb, "    %s(%d) - Disconnected", proxy_list[sockinfo->type].type, proxy_list[sockinfo->type].port);
						break;
				case OPENPROXY:
						prefmsg(u->nick, s_opsb, "    %s(%d) - Open Proxy", proxy_list[sockinfo->type].type, proxy_list[sockinfo->type].port);
						break;
				default:
						prefmsg(u->nick, s_opsb, "    %s(%d) - Unknown", proxy_list[sockinfo->type].type, proxy_list[sockinfo->type].port);
			}
			socknode = list_next(scandata->socks, socknode);
		}
	node = list_next(opsbl, node);
	}
}


void start_proxy_scan(lnode_t *scannode) {
	scaninfo *scandata;
	socklist *sockdata;
	lnode_t *socknode;
	int i, j;


	scandata = lnode_get(scannode);
	if (scandata->u) chanalert(s_opsb, "Starting proxy scan on %s (%s) by Request of %s", scandata->who, scandata->lookup, scandata->u->nick);
	scandata->socks = list_create(NUM_PROXIES);
	for (i = 0; i <  NUM_PROXIES; i++) {
		j = proxy_connect(scandata->ipaddr.s_addr, proxy_list[i].port, scandata->who);
		if (j > 0) {
			/* its ok */
			sockdata = malloc(sizeof(socklist));
			sockdata->sock = j;
			sockdata->flags = CONNECTING;
			sockdata->function = proxy_list[i].scan;
			sockdata->type = i;
			sockdata->bytes = 0;
			socknode = lnode_create(sockdata);
			list_append(scandata->socks, socknode);
		}
	}
	/* this is so we can timeout scans */
	scandata->started = time(NULL);
}

int http_proxy(int sock) {
	char *buf;
	int i;
	buf = malloc(512);
	i = snprintf(buf, 512, "CONNECT %s:%d HTTP/1.0\r\n\r\n", opsb.targethost, opsb.targetport);
#ifdef DEBUG
	log("sending http request");
#endif
	i= send(sock, buf, i, MSG_NOSIGNAL);
	free(buf);
	return i;
}


int sock4_proxy(int sock) {
	struct in_addr addr;
	unsigned long laddr;
	char *buf;
	int len;
 
	if (inet_aton(opsb.targethost, &addr) == 0) {
		log("OPSB socks4_proxy() : %s is not a valid IP",
		    opsb.targethost);
	    	return 0;
	}
    
	laddr = htonl(addr.s_addr);
 	buf = malloc(512);
	len = snprintf(buf, 512, "%c%c%c%c%c%c%c%c%c",  4, 1,
	    (((unsigned short) opsb.targetport) >> 8) & 0xFF,
	    (((unsigned short) opsb.targetport) & 0xff),
	    (char) (laddr >> 24) & 0xFF, (char) (laddr >> 16) & 0xFF,
	    (char) (laddr >> 8) & 0xFF, (char) laddr & 0xFF, 0);
	
	len = send(sock, buf, len, MSG_NOSIGNAL);
	free(buf);
	return(len);
}

int sock5_proxy(int sock) {
        struct in_addr addr;
        unsigned long laddr;
        int len;
        char *buf;

        if (inet_aton(opsb.targethost, &addr) == 0) {
                log("OPSB socks5_proxy() : %s is not a valid IP",
                    opsb.targethost);
        }

        laddr = htonl(addr.s_addr);
	buf = malloc(512);
        /* Form authentication string */
        /* Version 5, 1 number of methods, 0 method (no auth). */
        len = snprintf(buf, 512, "%c%c%c", 5, 1, 0);
        len = send(sock, buf, len, MSG_NOSIGNAL);
	if (len < 0) {
		free(buf);
		return len;
	}
        /* Form request string */

        len = snprintf(buf, 512, "%c%c%c%c%c%c%c%c%c%c", 5, 1, 0, 1,
            (char) (laddr >> 24) & 0xFF, (char) (laddr >> 16) & 0xFF,
            (char) (laddr >> 8) & 0xFF, (char) laddr & 0xFF,
            (((unsigned short) opsb.targetport) >> 8) & 0xFF,
            (((unsigned short) opsb.targetport) & 0xFF)
                      );

        len = send(sock, buf, len, MSG_NOSIGNAL);
        return(len);



}


int cisco_proxy(int sock) {
	char *buf;
	int i;
	buf = malloc(512);
	i = snprintf(buf, 512, "cisco\r\n");
	i = send(sock, buf, i, MSG_NOSIGNAL);
	if (i < 0)
		return i;
	i = snprintf(buf, 512, "telnet %s %d\r\n", opsb.targethost, opsb.targetport);
	i = send(sock, buf, i, MSG_NOSIGNAL);
	free(buf);
	return i;
}

int wingate_proxy(int sock) {
	char *buf;
	int i;
	buf = malloc(512);
	i = snprintf(buf, 512, "%s:%d\r\n", opsb.targethost, opsb.targetport);
	i = send(sock, buf, i, MSG_NOSIGNAL);
	free(buf);
	return i;
}



/* proxy read function */

int proxy_read(int socknum, char *sockname) {
	char *buf;
	int i = 0;
	scaninfo *scandata;
	lnode_t	*socknode;
	socklist *sockdata = NULL;
	

	scandata = find_scandata(sockname);
	if (!scandata) {
		log("ehh, wtf, can find scan data");
		return 1;
	}
	socknode = list_first(scandata->socks);
	while (socknode) {
		sockdata = lnode_get(socknode);
		if (sockdata->sock == socknum) {
			i = 1;
			break;
		}
		socknode = list_next(scandata->socks, socknode);
	}		
	if (i == 0) {
		log("ehh can't find socket info for proxy_read()");
		return 1;
	}
	buf = malloc(512);
	bzero(buf, 512);
	i = recv(socknum, buf, 512, 0);
	if (i < 0) {
#ifdef DEBUG
		log("OPSB proxy_read(): %d has the following error: %s", socknum, strerror(errno));
#endif
		if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "No %s Proxy Server on port %d", proxy_list[sockdata->type].type, proxy_list[sockdata->type].port);
		close(socknum);
		del_socket(sockname);
		sockdata->flags = UNCONNECTED;
		free(buf);
		return -1;
	} else {
		if (i > 0) {
#ifdef DEBUG
			log("OPSB proxy_read(): Got this: %s (%d)",buf, i);
#endif
			/* we check if this might be a normal http server */
			if (strstr(buf, "Method Not Allowed")) {
#ifdef DEBUG
				log("closing socket %d due to ok HTTP server", socknum);
#endif
				if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "No Open %s Proxy Server on port %d", proxy_list[sockdata->type].type, proxy_list[sockdata->type].port);
				sockdata->flags = UNCONNECTED;
				close(socknum);
				del_socket(sockname);
				free(buf);
				return -1;
			}
	
			/* this looks for the ban string */
			if (strstr(buf, opsb.lookforstring)) {
				if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "Open %s Proxy Server on port %d", proxy_list[sockdata->type].type, proxy_list[sockdata->type].port);
				++proxy_list[sockdata->type].noopen;
				scandata->state = GOTOPENPROXY;
				sockdata->flags = OPENPROXY;
				do_ban(scandata);
				close(socknum);
				del_socket(sockname);
				free(buf);
				return -1;
			}
			sockdata->bytes += i;
			/* avoid reading too much data */
			if (sockdata->bytes > opsb.maxbytes) {
#ifdef DEBUG
				log("OPSB proxy_read(): Closing %d due to too much data", socknum);
#endif
				if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "No Open %s Proxy Server on port %d", proxy_list[sockdata->type].type, proxy_list[sockdata->type].port);
				close(socknum);
				del_socket(sockname);
				sockdata->flags = UNCONNECTED;
				free(buf);
				return -1;
			}
		}
	}
	free(buf);
	return 1;

}

/* proxy write function */

int proxy_write(int socknum, char *sockname) {
	int i = 0;
	scaninfo *scandata;
	lnode_t	*socknode;
	socklist *sockdata = NULL;

	scandata = find_scandata(sockname);
	if (!scandata) {
		log("ehh, wtf, can find scan data");
		return 1;
	}
	socknode = list_first(scandata->socks);
	while (socknode) {
		sockdata = lnode_get(socknode);
		if (sockdata->sock == socknum) {
			i = 1;
			break;
		}
		socknode = list_next(scandata->socks, socknode);
	}		
	if (i == 0) {
		log("ehhh, can't find socket for proxy_write()");
		return 1;
	}			
	if (sockdata->flags == CONNECTING || sockdata->flags == SOCKCONNECTED) {
	
		if (sockdata->flags == CONNECTING) 
			i = (int)sockdata->function(socknum);
		else 
			i = send(socknum, "", 1, MSG_NOSIGNAL);
		if (i < 0) {
#ifdef DEBUG
			log("OPSB proxy_write(): %d has the following error: %s", socknum, strerror(errno));
#endif
			if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "No %s Proxy Server on port %d", proxy_list[sockdata->type].type, proxy_list[sockdata->type].port);
			close(socknum);
			del_socket(sockname);
			sockdata->flags = UNCONNECTED;
			return -1;
		} else {
			if (sockdata->flags != SOCKCONNECTED) ++proxy_list[sockdata->type].nofound;
			sockdata->flags = SOCKCONNECTED;
		}
	}
	return 1;
}

/* proxy error function */

int proxy_err(int socknum, char *sockname) {
return 1;
}


/* proxy connect function trys to connect a socket to a remote proxy 
*  its set non blocking, so both the send and recieve functions must be used
*  to tell if the connection is successfull or not
*  it also registers the socket with the core neostats socket functions
*/

int proxy_connect(unsigned long ipaddr, int port, char *who)
{
	struct sockaddr_in sa;
	char *sockname;
	int s;
	int i;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);
log("host %d, port %d", ipaddr, port);
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	sa.sin_addr.s_addr = ipaddr;

	/* set non blocking */
	
	if ((i = fcntl(s, F_SETFL, O_NONBLOCK)) < 0) {
		log("can't fcntl %s", strerror(i));
		return (-1);
	}

	if ((i = connect (s, (struct sockaddr *) &sa, sizeof (sa))) < 0) {
		switch (errno) {
			case EINPROGRESS:
					log("in progress");
					break;
			default:
					log("cant connect %s", strerror(errno), i);
					close (s);
					return (-1);
		}
	}

	sockname = malloc(64);
	snprintf(sockname, 63, "%s %d", who, s);

	add_socket("proxy_read", "proxy_write", "proxy_err", sockname, s, "opsb");

	free(sockname);
	return s;
}
