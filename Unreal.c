/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      Unreal.c, 
** Version: 1.5
** Date:    17/11/2001
*/
 
#include "stats.h"
#include "Unreal.h"

void sts(char *fmt,...);

void init_ircd() {
	if (usr_mds);
};


int sserver_cmd(const char *name, const int numeric, const char *infoline) {
	sts("%s %s %s %d :%s", me.name, (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
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
	sts("%s %s :%s", who, (me.token ? TOK_QUIT : MSG_QUIT), quitmsg);
	DelUser(who);
	return 1;
}

int spart_cmd(const char *who, const char *chan) {
	sts("%s %s %s", who, (me.token ? TOK_PART : MSG_PART), chan);
	return 1;
}

int sjoin_cmd(const char *who, const char *chan) {
	sts(":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
	return 1;
}

int schmode_cmd(const char *who, const char *chan, const char *mode, const char *args) {
	sts(":%s %s %s %s %s %lu", who, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, time(NULL));
	return 1;
}

int snick_cmd(const char *nick, const char *ident, const char *host, const char *realname) {
	sts("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time(NULL), ident, host, me.name, realname);
	AddUser(nick,ident, host, me.name);
	return 1;
}  

int sping_cmd(const char *from, const char *reply, const char *to) {
	sts(":%s %s %s :%s", from, (me.token ? TOK_NICK : MSG_NICK), reply, to);
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
	sts(":%s %s 0 %d %d %s 0 0 0 %s",me.name,(me.token ? TOK_NETINFO : MSG_NETINFO), time(NULL),ircd_srv.uprot, ircd_srv.cloak,me.netname);
	return 1;
}

int skill_cmd(const char *from, const char *target, const char *reason,...) {
	va_list ap;
	char buf[512];
	va_start(ap, reason);
	vsnprintf(buf, 512, reason, ap);
	sts(":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, buf);
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

void notice(char *who, char *buf,...)
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
void privmsg(char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	va_start(ap, fmt);
	vsnprintf(buf2, sizeof(buf2), fmt, ap);
	if (me.want_privmsg)
		sprintf(buf, ":%s PRIVMSG %s :%s", from, to, buf2);
	else
		sprintf(buf, ":%s NOTICE %s :%s", from, to, buf2);
	sts("%s", buf);
	va_end(ap);
}

void privmsg_list(char *to, char *from, const char **text)
{
	while (*text) {
		if (**text)
			privmsg(to, from, "%s", *text);
		else
			privmsg(to, from, " ");
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
