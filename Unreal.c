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
** $Id: Unreal.c,v 1.41 2003/05/22 13:51:54 fishwaldo Exp $
*/
 
#include "stats.h"
#include "Unreal.h"
#include "dl.h"
#include "log.h"
void sts(char *fmt,...);


aCtab cFlagTab[] = {
	{MODE_LIMIT, 'l', 0, 1},
	{MODE_VOICE, 'v', 1, 0},
	{MODE_HALFOP, 'h', 1, 0},
	{MODE_CHANOP, 'o', 1, 0},
	{MODE_PRIVATE, 'p', 0, 0},
	{MODE_SECRET, 's', 0, 0},
	{MODE_MODERATED, 'm', 0, 0},
	{MODE_NOPRIVMSGS, 'n', 0, 0},
	{MODE_TOPICLIMIT, 't', 0, 0},
	{MODE_INVITEONLY, 'i', 0, 0},
	{MODE_KEY, 'k', 0, 1},
	{MODE_RGSTR, 'r', 0, 0},
	{MODE_RGSTRONLY, 'R', 0, 0},
	{MODE_NOCOLOR, 'c', 0, 0},
	{MODE_CHANPROT, 'a', 1, 0},
	{MODE_CHANOWNER, 'q', 1, 0},
	{MODE_OPERONLY, 'O', 0, 0},
	{MODE_ADMONLY, 'A', 0, 0},
	{MODE_LINK, 'L', 0, 1},
	{MODE_NOKICKS, 'Q', 0, 0},
	{MODE_BAN, 'b', 0, 1},
	{MODE_STRIP, 'S', 0, 0},	/* works? */
	{MODE_EXCEPT, 'e', 0, 1},	/* exception ban */
	{MODE_NOKNOCK, 'K', 0, 0},	/* knock knock (no way!) */
	{MODE_NOINVITE, 'V', 0, 0},	/* no invites */
	{MODE_FLOODLIMIT, 'f', 0, 1},	/* flood limiter */
	{MODE_NOHIDING, 'H', 0, 0},	/* no +I joiners */
	{MODE_STRIPBADWORDS, 'G', 0, 0},	/* no badwords */
	{MODE_NOCTCP, 'C', 0, 0},	/* no CTCPs */
	{MODE_AUDITORIUM, 'u', 0, 0},
	{MODE_ONLYSECURE, 'z', 0, 0},
	{MODE_NONICKCHANGE, 'N', 0, 0},
	{0x0, 0x0, 0x0}
};


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
                                 {UMODE_SADMIN, 'a',100},
				 {UMODE_COADMIN, 'C',60},
				 {UMODE_EYES,	'e',0},
				 {UMODE_KIX, 'q',0},
				 {UMODE_BOT, 'B',0},
				 {UMODE_FCLIENT, 'F',0},
	   			 {UMODE_DEAF,    'd',0},
   				 {UMODE_HIDING,  'I',0},
                                 {UMODE_ADMIN, 'A',70},
                                 {UMODE_NETADMIN, 'N',185},
				 {UMODE_TECHADMIN, 'T',190},
                                 {UMODE_CLIENT, 'c',0},
                                 {UMODE_FLOOD, 'f',0},
                                 {UMODE_REGNICK, 'r',0},
                                 {UMODE_HIDE,    'x',0},
                                 {UMODE_CHATOP, 'b',0},
				 {UMODE_WHOIS, 'W',0},
                                 {0, 0, 0 }
};












void init_ircd() {
	if (usr_mds);
};


int sserver_cmd(const char *name, const int numeric, const char *infoline) {
	sts(":%s %s %s %d :%s", me.name, (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	return 1;
}

int slogin_cmd(const char *name, const int numeric, const char *infoline, const char *pass) {
	sts("%s %s", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts("%s %s %d :%s", (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	return 1;
}

int ssquit_cmd(const char *server) {
	sts("%s %s", (me.token ? TOK_SQUIT : MSG_SQUIT), server);
	return 1;
}

int sprotocol_cmd(const char *option) {
	sts("%s %s", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL), option);
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

int sjoin_cmd(const char *who, const char *chan) {
	sts(":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
	join_chan(finduser(who), (char *)chan);
	return 1;
}

int schmode_cmd(const char *who, const char *chan, const char *mode, const char *args) {
	char **av;
	int ac;	
	char tmp[512];

	sts(":%s %s %s %s %s %lu", who, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, time(NULL));
	snprintf(tmp, 512, "%s %s %s", chan, mode, args);
	ac = split_buf(tmp, &av, 0);
	ChanMode("", av, ac);
	return 1;
}

int snewnick_cmd(const char *nick, const char *ident, const char *host, const char *realname) {
	sts("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time(NULL), ident, host, me.name, realname);
	AddUser(nick,ident, host, me.name, 0, time(NULL));
	return 1;
}  

int sping_cmd(const char *from, const char *reply, const char *to) {
	sts(":%s %s %s :%s", from, (me.token ? TOK_PING : MSG_PING), reply, to);
	return 1;
}

int sumode_cmd(const char *who, const char *target, long mode) {
	int i, j;
	char newmode[20];
	newmode[0] = '+';
	j=1;
	for (i = 0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1); i++) {
		if (mode & usr_mds[i].umodes) {
			newmode[j]= usr_mds[i].mode;
			j++;
		}
		
	}
	newmode[j] = '\0';
	sts(":%s %s %s :%s", who, (me.token ? TOK_MODE : MSG_MODE), target, newmode);
	UserMode(target, newmode);
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
	sts(":%s %s 0 %d %d %s 0 0 0 :%s",me.name,(me.token ? TOK_NETINFO : MSG_NETINFO), time(NULL),ircd_srv.uprot, ircd_srv.cloak,me.netname);
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
	sts(":%s %s %s :%s", from, (me.token ? TOK_SMO : MSG_SMO), umodetarget, msg);
	return 1;
}

int snick_cmd(const char *oldnick, const char *newnick) {
	Change_User(finduser(oldnick), newnick);
	sts(":%s %s %s %d", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, time(NULL));
	return 1;
}
int sswhois_cmd(const char *target, const char *swhois) {
	sts("%s %s :%s", (me.token ? TOK_SWHOIS : MSG_SWHOIS), target, swhois);
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
		nlog(LOG_WARNING, LOG_CORE, "Can't Find user %s for ssvshost_cmd", who);
		return 0;
	} else {
		strcpy(u->vhost, vhost);
		sts(":%s %s %s %s", me.name, (me.token ? TOK_CHGHOST : MSG_CHGHOST), who, vhost);
		return 1;
	}
}

int ssvsmode_cmd(const char *target, const char *modes) {
	User *u;
	u = finduser(target);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE, "Can't find user %s for ssvsmode_cmd", target);
		return 0;
	} else {
		sts(":%s %s %s %s", me.name, (me.token ? TOK_SVSMODE : MSG_SVSMODE), target, modes);
		UserMode(target, modes);
	}
	return 1;
}

int ssvskill_cmd(const char *target, const char *reason, ...) {
	User *u;
	char **av;
	int ac = 0;
	va_list ap;
	char buf[512];
	u = finduser(target);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE, "Cant find user %s for ssvskill_cmd", target);
		return 0;
	} else {
		va_start(ap, reason);
		vsnprintf(buf, 512, reason, ap);
		sts(":%s %s %s :%s", me.name, (me.token ? TOK_SVSKILL : MSG_SVSKILL), target, buf);
		AddStringToList(&av, u->nick, &ac);
		Module_Event("KILL", av, ac);
		free(av);
		DelUser(target);
		va_end(ap);
		return 1;
	}
}

/* akill is gone in the latest Unreals, so we set Glines instead */

int sakill_cmd(const char *host, const char *ident, const char *setby, const int length, const char *reason,...) {
	va_list ap;
	char buf[512];
	va_start(ap, reason);
	vsnprintf(buf, 512, reason, ap);
	sts(":%s %s + G %s %s %s %d %d :%s", me.name, (me.token ? TOK_TKL : MSG_TKL), ident, host, setby, time(NULL) + length, time(NULL), buf);
	va_end(ap);
	return 1;
}

int srakill_cmd(const char *host, const char *ident) {
	sts(":%s %s - G %s %s %s", me.name, (me.token ? TOK_TKL : MSG_TKL), ident, host, me.name);
	return 1;
}

void sts(char *fmt,...)
{
	va_list ap;
	char buf[512];
	int sent;
	va_start (ap, fmt);
	vsnprintf (buf, 512, fmt, ap);

	nlog(LOG_DEBUG2, LOG_CORE, "SENT: %s", buf);
	strcat (buf, "\n");
	sent = write (servsock, buf, strlen (buf));
	if (sent == -1) {
		nlog(LOG_CRITICAL, LOG_CORE, "Write error.");
		do_exit(0);
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
		snprintf(out,512, ":%s PRIVMSG %s :%s",who, me.chan, tmp);
		nlog(LOG_DEBUG3, LOG_CORE, "SENT: %s", out);
		strcat (out, "\n");
		sent = write(servsock, out, strlen (out));
		if (sent == -1) {
			me.onchan = 0;
			nlog(LOG_DEBUG3, LOG_CORE, "Write error.");
			do_exit(0);
		}
		me.SendM++;
		me.SendBytes = me.SendBytes + sent;
	}
	va_end (ap);
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
	snprintf(buf, 512, ":%s NOTICE %s :%s", from, to, buf2);
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
	snprintf(buf, 512, ":%s PRIVMSG %s :%s", from, to, buf2);
	sts("%s", buf);
	va_end(ap);
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
		snprintf(buf, 512, ":%s PRIVMSG %s :%s", from, to, buf2);
	} else {
		snprintf(buf, 512, ":%s NOTICE %s :%s", from, to, buf2);
	}
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
		snprintf(buf, 512, ":%s GLOBOPS :%s", from, buf2);
		sts("%s", buf);
	} else {
		nlog(LOG_NORMAL, LOG_CORE, "%s", buf2);
	}
	va_end(ap);
}
