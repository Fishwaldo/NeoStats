/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
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
** $Id: hybrid7.c,v 1.7 2002/09/16 03:31:46 fishwaldo Exp $
*/
 
#include "stats.h"
#include "hybrid7.h"
#include "dl.h"
void sts(char *fmt,...);

aCtab cFlagTab[] = {
		{MODE_SECRET,		's', 0, 0},
		{MODE_PRIVATE,		'p', 0, 0},
		{MODE_MODERATED,	'm', 0, 0},
		{MODE_TOPICLIMIT,	't', 0, 0},
		{MODE_INVITEONLY,	'i', 0, 0},
		{MODE_NOPRIVMSGS,	'n', 0, 0},
		{MODE_HIDEOPS,		'a', 0, 0},
		{MODE_LIMIT,		'l', 0, 1},
		{MODE_KEY,		'k', 0, 1},
		{MODE_BAN,		'b', 0, 1},
		{MODE_EXCEPT,		'e', 0, 1},
		{MODE_HALFOP,		'h', 1, 0},
		{MODE_CHANOP,		'o', 1, 0},
		{MODE_VOICE,		'v', 1, 0},
		{MODE_INVEX,		'I', 0, 1},	
		{0x0, 0x0, 0x0, 0x0}
};


Oper_Modes usr_mds[] = {
				{UMODE_OPER, 'o', 50},
				{UMODE_ADMIN, 'A', 190},
				{UMODE_BOTS, 'b', 0},
				{UMODE_CCONN, 'c', 0},
		    		{UMODE_DEBUG, 'd', 200},
	      			{UMODE_FULL,  'f', 0},
  				{UMODE_CALLERID, 'g', 0},
    				{UMODE_INVISIBLE, 'i', 0},
      				{UMODE_SKILL, 'k', 0},
        			{UMODE_LOCOPS, 'l', 40},
          			{UMODE_NCHANGE, 'n', 0},
              			{UMODE_REJ, 'r', 0},
                		{UMODE_SERVNOTICE, 's', 0},
                  		{UMODE_UNAUTH, 'u', 0},
                    		{UMODE_WALLOP, 'w', 0},
                      		{UMODE_EXTERNAL, 'x', 0},
                        	{UMODE_SPY, 'y', 0},
                          	{UMODE_OPERWALL, 'z', 0},
				{UMODE_SERVICES, 'S', 0},
                                {0, 0, 0 }
};

void init_ircd() {
	if (usr_mds);
};

int seob_cmd(const char *server) {
	sts(":%s %s", server,  MSG_EOB);
	return 1;
}


int sserver_cmd(const char *name, const int numeric, const char *infoline) {
	sts("%s %s %s %d :%s", me.name, MSG_SERVER, name, numeric, infoline);
	return 1;
}

int slogin_cmd(const char *name, const int numeric, const char *infoline, const char *pass) {
	sts("%s %s :TS", MSG_PASS, pass);
	sts("CAPAB :TS EX CHW IE EOB KLN GLN KNOCK HOPS HUB AOPS MX");
	sts("%s %s %d :%s", MSG_SERVER, name, numeric, infoline);
	return 1;
}

int ssquit_cmd(const char *server) {
	sts("%s %s", MSG_SQUIT, server);
	return 1;
}

int sprotocol_cmd(const char *option) {
	return 1;
}

int squit_cmd(const char *who, const char *quitmsg) {
	sts(":%s %s :%s", who, MSG_QUIT, quitmsg);
	DelUser(who);
	return 1;
}

int spart_cmd(const char *who, const char *chan) {
	sts(":%s %s %s", who, MSG_PART, chan);
	part_chan(finduser(who), (char *)chan);
	return 1;
}

int sjoin_cmd(const char *who, const char *chan) {
	sts(":%s %s 0 %s + :%s", me.name, MSG_SJOIN, chan, who);
	join_chan(finduser(who), (char *)chan);
	return 1;
}

int schmode_cmd(const char *who, const char *chan, const char *mode, const char *args) {
	char **av;
	int ac;	
	char tmp[512];

	sts(":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, time(NULL));
	sprintf(tmp, "%s %s %s", chan, mode, args);
	ac = split_buf(tmp, &av, 0);
	ChanMode("", av, ac);
	free(av);
	return 1;
}
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
	sts("%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, time(NULL), newmode, ident, host, me.name, realname);
	AddUser(nick,ident, host, me.name, 0, time(NULL));
	UserMode(nick, newmode);
	return 1;
}  

int sping_cmd(const char *from, const char *reply, const char *to) {
	sts(":%s %s %s :%s", from, MSG_PING, reply, to);
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
	sts(":%s %s %s :%s", who, MSG_MODE, target, newmode);
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
	sts("%s %s", MSG_PONG, reply);
	return 1;
}


int skill_cmd(const char *from, const char *target, const char *reason,...) {
	va_list ap;
	char buf[512];
	va_start(ap, reason);
	vsnprintf(buf, 512, reason, ap);
	sts(":%s %s %s :%s", from, MSG_KILL, target, buf);
	va_end(ap);
	DelUser(target);
	return 1;
}

int ssmo_cmd(const char *from, const char *umodetarget, const char *msg) {
	notice(s_Services, "Warning, Module %s tried to SMO, which is not supported in Hybrid", segvinmodule);
	log("Warning, Module %s tried to SMO, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int snick_cmd(const char *oldnick, const char *newnick) {
	Change_User(finduser(oldnick), newnick);
	sts(":%s %s %s %d", oldnick, MSG_NICK, newnick, time(NULL));
	return 1;
}
int sswhois_cmd(const char *target, const char *swhois) {
	notice(s_Services, "Warning Module %s tried to SWHOIS, which is not supported in Hybrid", segvinmodule);
	log("Warning. Module %s tried to SWHOIS, which is not supported in Hybrid", segvinmodule);
	return 1;
}
int ssvsnick_cmd(const char *target, const char *newnick) {
	notice(s_Services, "Warning Module %s tried to SVSNICK, which is not supported in Hybrid", segvinmodule);
	log("Warning. Module %s tried to SVSNICK, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int ssvsjoin_cmd(const char *target, const char *chan) {
	notice(s_Services, "Warning Module %s tried to SJOIN, which is not supported in Hybrid", segvinmodule);
	log("Warning. Module %s tried to SJOIN, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int ssvspart_cmd(const char *target, const char *chan) {
	notice(s_Services, "Warning Module %s tried to SVSPART, which is not supported in Hybrid", segvinmodule);
	log("Warning. Module %s tried to SVSPART, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int skick_cmd(const char *who, const char *target, const char *chan, const char *reason) {
	sts(":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
	part_chan(finduser(target), (char *)chan); 
	return 1;
}
int swallops_cmd(const char *who, const char *msg,...) {
	va_list ap;
	char buf[512];
	va_start(ap, msg);
	vsnprintf(buf, 512, msg, ap);
	sts(":%s %s :%s", who, MSG_WALLOPS, buf);
	va_end(ap);
	return 1;
}

int ssvshost_cmd(const char *who, const char *vhost) {
	notice(s_Services, "Warning Module %s tried to SVSHOST, which is not supported in Hybrid", segvinmodule);
	log("Warning. Module %s tried to SVSHOST, which is not supported in Hybrid", segvinmodule);
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

int sakill_cmd(const char *host, const char *ident, const char *setby, const int length, const char *reason,...) {
	/* there isn't a akill on Hybrid, so we send a kline to all servers! */
	hscan_t ss;
	hnode_t *sn;
	Server *s;

	va_list ap;
	char buf[512];
	va_start(ap, reason);
	vsnprintf(buf, 512, reason, ap);
	
        hash_scan_begin(&ss, sh);
        while ((sn = hash_scan_next(&ss)) != NULL) {
        	s = hnode_get(sn);
		sts(":%s %s %s %lu %s %s :%s", setby, MSG_KLINE, s->name, length, ident, host, buf);
        }	
	va_end(ap);
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
