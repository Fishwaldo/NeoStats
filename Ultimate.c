/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
** $Id: Ultimate.c,v 1.34 2003/01/06 12:07:25 fishwaldo Exp $
*/
 
#include "stats.h"
#include "Ultimate.h"
#include "dl.h"
void sts(char *fmt,...);

aCtab cFlagTab[] = {
		{MODE_BAN,		'b', 0, 1},
		{MODE_EXCEPT,		'e', 0, 1},
		{MODE_FLOODLIMIT,	'f', 0, 1}, /* Flood limiter */
		{MODE_HALFOP,		'h', 1, 0},
		{MODE_CHANADMIN, 	'a', 1, 0},
		{MODE_INVITEONLY,	'i', 0, 0},
		{MODE_KEY,		'k', 0, 1},
		{MODE_LIMIT,		'l', 0, 1},
		{MODE_MODERATED,	'm', 0, 0},
		{MODE_NOPRIVMSGS,	'n', 0, 0},
		{MODE_CHANOP,		'o', 1, 0},
		{MODE_PRIVATE,		'p', 0, 0},
		{MODE_RGSTR,		'r', 0, 0},
		{MODE_SECRET,		's', 0, 0},
		{MODE_TOPICLIMIT,	't', 0, 0},
		{MODE_VOICE,		'v', 1, 0},
		{MODE_NOCOLOR,		'x', 0, 0},
		{MODE_ADMONLY,		'A', 0, 0},
		{MODE_NOINVITE,		'I', 0, 0}, /* no invites */	
		{MODE_NOKNOCK,	 	'K', 0, 0}, /* knock knock (no way!) */
		{MODE_LINK,		'L', 0, 1},
		{MODE_OPERONLY,		'O', 0, 0},
		{MODE_RGSTRONLY,	'R', 0, 0},
		{MODE_STRIP,		'S', 0, 0}, /* works? */
		{0x0, 0x0, 0x0, 0x0}
};


#ifdef ULTIMATE3
Oper_Modes usr_mds[] = {
				 {UMODE_OPER, 'O', 50},
                                 {UMODE_LOCOP, 'o', 40},
                                 {UMODE_INVISIBLE, 'i', 0},
                                 {UMODE_WALLOP, 'w', 0},
                                 {UMODE_SERVNOTICE, 's', 0},
                                 {UMODE_CLIENT, 'c', 0},
                                 {UMODE_REGNICK, 'r',10},
                                 {UMODE_KILLS, 'k',0},
                                 {UMODE_FAILOP, 'g', 0},
                                 {UMODE_HELPOP, 'h', 30},
				 {UMODE_FLOOD, 'f', 0},
		 		 {UMODE_SPY, 'y', 0},
				 {UMODE_DCC, 'D', 0},
		 		 {UMODE_GLOBOPS, 'g', 0},
	 	 		 {UMODE_CHATOP, 'c', 0},
		 		 {UMODE_SERVICESOPER, 'a', 100},
				 {UMODE_REJ, 'j', 0},
		 		 {UMODE_ROUTE, 'n', 0},
				 {UMODE_SPAM, 'm', 0},
                                 {UMODE_HIDE,    'x',0},
				 {UMODE_IRCADMIN, 'Z', 200},
                                 {UMODE_SERVICESADMIN, 'P',200},
                                 {UMODE_SERVICES, 'S',200},
				 {UMODE_PROT, 'p', 0},
		 		 {UMODE_GLOBCON, 'F', 0},
	   			 {UMODE_DEBUG,    'd',0},
				 {UMODE_DCCWARN, 'd', 0},
		 		 {UMODE_WHOIS, 'W', 0},
	 	 		 {0, 0, 0}
};

Oper_Modes susr_mds[] = {
				 {SMODE_SSL, 's', 0},
		 		 {SMODE_COADMIN, 'a', 75},
	 	 		 {SMODE_SERVADMIN, 'A', 100},
				 {SMODE_COTECH, 't', 125},
		 		 {SMODE_TECHADMIN, 'T', 150},
	 	 		 {SMODE_CONET, 'n', 175},
				 {SMODE_NETADMIN, 'N', 190},
		 		 {SMODE_GUEST, 'G', 100},
				 {0, 0, 0}
};				
						
#elif ULTIMATE
Oper_Modes usr_mds[] = {
                                 {UMODE_OPER, 'o', 50},
                                 {UMODE_LOCOP, 'O', 40},
                                 {UMODE_INVISIBLE, 'i', 0},
                                 {UMODE_WALLOP, 'w', 0},
                                 {UMODE_FAILOP, 'g', 0},
                                 {UMODE_HELPOP, 'h', 30},
                                 {UMODE_SERVNOTICE, 's',0},
                                 {UMODE_KILLS, 'k',0},
                                 {UMODE_SERVICES, 'S',200},
                                 {UMODE_SERVICESADMIN, 'P',200},
                                 {UMODE_RBOT, 'B',0},
                                 {UMODE_SBOT, 'b', 0},
                                 {UMODE_DEAF,    'd',0},
                                 {UMODE_ADMIN, 'z',70},
                                 {UMODE_NETADMIN, 'N',185},
                                 {UMODE_TECHADMIN, 'T',190},
                                 {UMODE_CLIENT, 'c',0},
                                 {UMODE_FLOOD, 'f',0},
                                 {UMODE_REGNICK, 'r',0},
                                 {UMODE_HIDE,    'x',0},
                                 {UMODE_WATCHER, 'W',0},
                                 {UMODE_SERVICESOPER, 'a', 100},
                                 {UMODE_SUPER, 'p', 40},
                                 {UMODE_IRCADMIN, 'Z', 100},
                                 {UMODE_DEAF, 'd', 0},
                                 {0, 0, 0 }
};
#endif












int sserver_cmd(const char *name, const int numeric, const char *infoline) {
	sts("%s %s %s %d :%s", me.name, (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	return 1;
}

int slogin_cmd(const char *name, const int numeric, const char *infoline, const char *pass) {
#ifndef ULTIMATE3
	sts("%s %s", (me.token ? TOK_PASS : MSG_PASS), pass);
#else
	sts("%s %s :TS", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts("CAPAB TS5 BURST SSJ5 NICKIP CLIENT");
#endif
	sts("%s %s %d :%s", (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	return 1;
}

int ssquit_cmd(const char *server) {
	sts("%s %s", (me.token ? TOK_SQUIT : MSG_SQUIT), server);
	return 1;
}

int sprotocol_cmd(const char *option) {
#ifndef ULTIMATE3
	sts("%s %s", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL), option);
#endif	
	return 1;
}

int squit_cmd(const char *who, const char *quitmsg) {
	sts(":%s %s :%s", who, (me.token ? TOK_QUIT : MSG_QUIT), quitmsg);
	DelUser(who);
	return 1;
}

int spart_cmd(const char *who, const char *chan) {
	sts(":%s %s %s", who, (me.token ? TOK_PART : MSG_PART), chan);
	part_chan(finduser(who), (char *)chan);
	return 1;
}
#ifdef ULTIMATE3
int sjoin_cmd(const char *who, const char *chan, unsigned long chflag) {
	char flag;
	char mode[2]; 
	char **av;
	int ac;
	char tmp[512];
	switch (chflag) {
		case MODE_CHANOP:	flag = '@';
					strcpy(mode, "0");
					break;
		case MODE_HALFOP:	flag = '%';
					strcpy(mode, "h");
					break;
		case MODE_VOICE:	flag = '+';
					strcpy(mode, "v");
					break;
		case MODE_CHANADMIN:	flag = '!';
					strcpy(mode, "a");
					break;
		default:		flag = ' ';
					strcpy(mode, "");
	}
	sts(":%s %s 0 %s + :%c%s", me.name, MSG_SJOIN, chan, flag, who);
	join_chan(finduser(who), (char *)chan);
	sprintf(tmp, "%s +%s %s", chan, mode, who);
	ac = split_buf(tmp, &av, 0);
	ChanMode(me.name, av, ac);
	free(av);

#else 
int sjoin_cmd(const char *who, const char *chan) {
	sts(":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
	join_chan(finduser(who), (char *)chan);
#endif
	return 1;
}

int schmode_cmd(const char *who, const char *chan, const char *mode, const char *args) {
	char **av;
	int ac;	
	char tmp[512];

	sts(":%s %s %s %s %s %lu", me.name, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, time(NULL));
	sprintf(tmp, "%s %s %s", chan, mode, args);
	ac = split_buf(tmp, &av, 0);
	ChanMode("", av, ac);
	free(av);
	return 1;
}
#ifndef ULTIMATE3
int snewnick_cmd(const char *nick, const char *ident, const char *host, const char *realname) {
	sts("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time(NULL), ident, host, me.name, realname);
	AddUser(nick,ident, host, me.name, 0, time(NULL));
#else 
int snewnick_cmd(const char *nick, const char *ident, const char *host, const char *realname, long mode) {
	int i;
	char newmode[20];
	newmode[0] = '+';
	newmode[1] = '\0';
	for (i = 0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1); i++) {
		if (mode & usr_mds[i].umodes) {
			sprintf(newmode, "%s%c", newmode, usr_mds[i].mode);
		}
	}
	sts("%s %s 1 %lu %s %s %s %s 0 %lu :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time(NULL), newmode, ident, host, me.name, time(NULL), realname);
	AddUser(nick,ident, host, me.name, 0, time(NULL));
#ifdef ULTIMATE3
	UserMode(nick, newmode, 0);
#else
	UserMode(nick, newmode);
#endif
#endif
	return 1;
}  

int sping_cmd(const char *from, const char *reply, const char *to) {
	sts(":%s %s %s :%s", from, (me.token ? TOK_PING : MSG_PING), reply, to);
	return 1;
}

int sumode_cmd(const char *who, const char *target, long mode) {
	int i;
	char newmode[20];
	newmode[0] = '+';
	newmode[1] = '\0';
	for (i = 0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1); i++) {
		if (mode & usr_mds[i].umodes) {
			sprintf(newmode, "%s%c", newmode, usr_mds[i].mode);
		}
	}
	sts(":%s %s %s :%s", who, (me.token ? TOK_MODE : MSG_MODE), target, newmode);
#ifdef ULTIMATE3
	UserMode(target, newmode, 0);
#else
	UserMode(target, newmode);
#endif
	return 1;
}

int snumeric_cmd(const int numeric, const char *target, const char *data,...) {
	va_list ap;
	char buf[512];
	va_start(ap, data);
	vsnprintf(buf, 512, data, ap);
	sts(":%s %d %s :%s", me.name, numeric, target, buf);
	va_end(ap);
	return 1;	
}

int  spong_cmd(const char *reply) {
	sts("%s %s", (me.token ? TOK_PONG : MSG_PONG), reply);
	return 1;
}

int snetinfo_cmd() {
	sts(":%s %s 0 %d %d %s 0 0 0 :%s",me.name,MSG_SNETINFO, time(NULL),ircd_srv.uprot, ircd_srv.cloak,me.netname);
	return 1;
}

int vctrl_cmd() {
	sts("%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, ircd_srv.uprot, ircd_srv.nicklg, ircd_srv.modex, ircd_srv.gc, me.netname);
	return 1;
}
int skill_cmd(const char *from, const char *target, const char *reason,...) {
	va_list ap;
	char buf[512];
	va_start(ap, reason);
	vsnprintf(buf, 512, reason, ap);
	sts(":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, buf);
	va_end(ap);
	DelUser(target);
	return 1;
}

int ssmo_cmd(const char *from, const char *umodetarget, const char *msg) {
	notice(s_Services, "Warning, Module %s tried to SMO, which is not supported in Ultimate", segvinmodule);
	log("Warning, Module %s tried to SMO, which is not supported in Ultimate", segvinmodule);
	return 1;
}

int snick_cmd(const char *oldnick, const char *newnick) {
	Change_User(finduser(oldnick), newnick);
	sts(":%s %s %s %d", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, time(NULL));
	return 1;
}
int sswhois_cmd(const char *target, const char *swhois) {
	notice(s_Services, "Warning Module %s tried to SWHOIS, which is not supported in Ultimate", segvinmodule);
	log("Warning. Module %s tried to SWHOIS, which is not supported in Ultimate", segvinmodule);
	return 1;
}
int ssvsnick_cmd(const char *target, const char *newnick) {
	sts("%s %s %s :%d", (me.token ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, time(NULL));
	return 1;
}

int ssvsjoin_cmd(const char *target, const char *chan) {
	sts("%s %s %s", (me.token ? TOK_SVSJOIN : MSG_SVSJOIN), target, chan);
	return 1;
}

int ssvspart_cmd(const char *target, const char *chan) {
	sts("%s %s %s", (me.token ? TOK_SVSPART : MSG_SVSPART), target, chan);
	return 1;
}

int skick_cmd(const char *who, const char *target, const char *chan, const char *reason) {
	sts(":%s %s %s %s :%s", who, (me.token ? TOK_KICK : MSG_KICK), chan, target, (reason ? reason : "No Reason Given"));
	part_chan(finduser(target), (char *)chan); 
	return 1;
}
int swallops_cmd(const char *who, const char *msg,...) {
	va_list ap;
	char buf[512];
	va_start(ap, msg);
	vsnprintf(buf, 512, msg, ap);
	sts(":%s %s :%s", who, (me.token ? TOK_WALLOPS : MSG_WALLOPS), buf);
	va_end(ap);
	return 1;
}

int ssvshost_cmd(const char *who, const char *vhost) {
	User *u;
	u = finduser(who);
	if (!u) {
		log("Can't Find user %s for ssvshost_cmd", who);
		return 0;
	} else {
		strcpy(u->vhost, vhost);
#ifdef ULTIMATE3
		sts(":%s %s %s %s", me.name, (me.token ? TOK_SETHOST : MSG_SETHOST), who, vhost);
#elif ULTIMATE
                sts(":%s CHGHOST %s %s", me.name, who, vhost);
#endif
		return 1;
	}
}
int sakill_cmd(const char *host, const char *ident, const char *setby, const int length, const char *reason,...) {
	va_list ap;
	char buf[512];
	va_start(ap, reason);
	vsnprintf(buf, 512, reason, ap);
	sts(":%s %s %s %s %d %s %d :%s", me.name, (me.token ? TOK_AKILL : MSG_AKILL), host, ident, length, setby, time(NULL), buf);
	va_end(ap);
	return 1;
}


int ssvinfo_cmd() {
	sts("SVINFO 5 3 0 :%d", time(NULL));
	return 1;
}
int sburst_cmd(int b) {
	if (b == 0) {
		sts("BURST 0");
	} else {
		sts("BURST");
	}
	return 1;
}



void sts(char *fmt,...)
{
	va_list ap;
	char buf[512];
	int sent;
	va_start (ap, fmt);
	vsnprintf (buf, 512, fmt, ap);

#ifdef DEBUG
	log("SENT: %s", buf);
#endif
	strcat (buf, "\n");
	sent = write (servsock, buf, strlen (buf));
	if (sent == -1) {
		log("Write error.");
		exit(0);
	}
	me.SendM++;
	me.SendBytes = me.SendBytes + sent;
	va_end (ap);
}

void chanalert(char *who, char *buf,...)
{
	va_list ap;
	char tmp[512];
	char out[512];
	int sent;
	va_start (ap, buf);
	vsnprintf (tmp, 512, buf, ap);

	if (me.onchan) {
		sprintf(out,":%s PRIVMSG %s :%s",who, me.chan, tmp);
#ifdef DEBUG
		log("SENT: %s", out);
#endif

		strcat (out, "\n");
		sent = write(servsock, out, strlen (out));
		if (sent == -1) {
			me.onchan = 0;
			log("Write error.");
			exit(0);
		}
		me.SendM++;
		me.SendBytes = me.SendBytes + sent;
	}
	va_end (ap);
}
void prefmsg(char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	va_start(ap, fmt);
	vsnprintf(buf2, sizeof(buf2), fmt, ap);
        if (findbot(to)) {
	        chanalert(s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
                return;
        }
	if (me.want_privmsg) {
		sprintf(buf, ":%s PRIVMSG %s :%s", from, to, buf2);
	} else {
		sprintf(buf, ":%s NOTICE %s :%s", from, to, buf2);
	}
	sts("%s", buf);
	va_end(ap);
}
void privmsg(char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

        if (findbot(to)) {
                chanalert(s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
                return;
        }

	va_start(ap, fmt);
	vsnprintf(buf2, sizeof(buf2), fmt, ap);
	sprintf(buf, ":%s PRIVMSG %s :%s", from, to, buf2);
	sts("%s", buf);
	va_end(ap);
}

void notice(char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

        if (findbot(to)) {
                chanalert(s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
                return;
        }

	va_start(ap, fmt);
	vsnprintf(buf2, sizeof(buf2), fmt, ap);
	sprintf(buf, ":%s NOTICE %s :%s", from, to, buf2);
	sts("%s", buf);
	va_end(ap);
}


void privmsg_list(char *to, char *from, const char **text)
{
	while (*text) {
		if (**text)
			prefmsg(to, from, "%s", *text);
		else
			prefmsg(to, from, " ");
		text++;
	}	
}


void globops(char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	va_start(ap, fmt);
	vsnprintf(buf2, sizeof(buf2), fmt, ap);

/* Shmad - have to get rid of nasty term echos :-) */

/* Fish - now that was crackhead coding! */
	if (me.onchan) { 
		sprintf(buf, ":%s GLOBOPS :%s", from, buf2);
		sts("%s", buf);
	} else {
		log("%s", buf2);
	}
	va_end(ap);
}
