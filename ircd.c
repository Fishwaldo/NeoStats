/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: ircd.c,v 1.7 2000/02/22 03:32:32 fishwaldo Exp $
*/
 
#include "stats.h"
#include "dl.h"

extern const char version_date[], version_time[];
extern const char protocol_version[];

void Usr_Version(char *, char *);
void Usr_ShowMOTD(char *, char *);
void Usr_Showcredits(char *, char *);
void Usr_AddServer(char *, char *);
void Usr_DelServer(char *, char *);
void Usr_DelUser(char *, char *);
void Usr_Mode(char *, char *);
void Usr_Kill(char *, char *);
void Usr_Pong(char *, char *);
void Usr_Away(char *, char *);
void Usr_Nick(char *, char *);
void Usr_Topic(char *, char *);
void Usr_Kick(char *, char *);
void Usr_Join(char *, char *);
void Usr_Part(char *, char *);
void Srv_Ping(char *, char *);
void Srv_Netinfo(char *, char *);
void Srv_Pass(char *, char *);
void Srv_Server(char *, char *);
void Srv_Squit(char *, char *);
void Srv_Nick(char *, char *);
void Srv_Svsnick(char *, char *);
void Srv_Kill(char *, char *);



static void ShowMOTD(char *);
static void Showcredits(char *);

struct int_commands {
	char *name;
	void (*function)(char *origin, char *coreLine);
	int srvmsg; /* Should this be a Server Message(1), or a User Message?(0) */
};

/* The Following Array is a list of Commands that are Handled Internally by NeoStats 
they have Nothing to do with the Array that the Modules use. These Functions are called before any
Module Functions to make sure the Internal Database is kept uptodate, as Some modules may utilize the Users or Servers
or Channel array.
if "srvmsg" is defined, it means that the command must be prefixed by ":" indicating a message from a Server (I think)

*/
typedef struct int_commands IntCommands;
 
IntCommands cmd_list[] = {
	/* Command	Function		srvmsg*/
	{MSG_VERSION,	Usr_Version,		1},
	{TOK_VERSION,	Usr_Version,		1},
	{MSG_MOTD,	Usr_ShowMOTD,		1},
	{TOK_MOTD,	Usr_ShowMOTD,		1},
	{MSG_CREDITS,	Usr_Showcredits,	1},
	{TOK_CREDITS,	Usr_Showcredits,	1},
	{MSG_SERVER,	Usr_AddServer,		1},
	{TOK_SERVER,	Usr_AddServer,		1},
	{MSG_SQUIT,	Usr_DelServer,		1},
	{TOK_SQUIT,	Usr_DelServer,		1},
	{MSG_QUIT,	Usr_DelUser,		1},
	{TOK_QUIT,	Usr_DelUser,		1},
	{MSG_MODE,	Usr_Mode,		1},
	{TOK_MODE,	Usr_Mode,		1},
	{MSG_SVSMODE,	Usr_Mode,		1},
	{TOK_SVSMODE,	Usr_Mode,		1},
	{MSG_SVS2MODE,	Usr_Mode,		1},
	{TOK_SVS2MODE,	Usr_Mode,		1},
	{MSG_KILL,	Usr_Kill,		1},
	{TOK_KILL,	Usr_Kill,		1},
	{MSG_PONG,	Usr_Pong,		1},
	{TOK_PONG,	Usr_Pong,		1},
	{MSG_AWAY,	Usr_Away,		1},
	{TOK_AWAY,	Usr_Away,		1},
	{MSG_NICK,	Usr_Nick,		1},
	{TOK_NICK,	Usr_Nick,		1},
	{MSG_TOPIC,	Usr_Topic,		1},
	{TOK_TOPIC,	Usr_Topic,		1},
	{MSG_KICK,	Usr_Kick,		1},
	{TOK_KICK,	Usr_Kick,		1},
	{MSG_JOIN,	Usr_Join,		1},
	{TOK_JOIN,	Usr_Join,		1},
	{MSG_PART,	Usr_Part,		1},
	{TOK_PART,	Usr_Part,		1},
	{MSG_PING,	Srv_Ping,		0},
	{TOK_PING,	Srv_Ping,		0},
	{MSG_NETINFO,	Srv_Netinfo,		0},
	{TOK_NETINFO,	Srv_Netinfo,		0},
	{MSG_PASS,	Srv_Pass,		0},
	{TOK_PASS,	Srv_Pass,		0},
	{MSG_SERVER,	Srv_Server,		0},
	{TOK_SERVER,	Srv_Server,		0},
	{MSG_SQUIT,	Srv_Squit,		0},
	{TOK_SQUIT,	Srv_Squit,		0},
	{MSG_NICK,	Srv_Nick,		0},
	{TOK_NICK,	Srv_Nick,		0},
	{MSG_SVSNICK,	Srv_Svsnick,		0},
	{TOK_SVSNICK,	Srv_Svsnick,		0},
	{MSG_KILL,	Srv_Kill,		0},
	{TOK_KILL,	Srv_Kill,		0},
	{NULL,		NULL,			0}
};

int init_bot(char *nick, char *user, char *host, char *rname, char *modes, char *mod_name)
{
	User *u;
	segv_location = "init_bot";
	u = finduser(nick);
	if (u) {
		log("Attempting to Login with a Nickname that already Exists: %s",nick);
		return -1;
	}
	log("about to addnick %s",nick);
	add_mod_user(nick, mod_name);
	sts("NICK %s 1 %d %s %s %s 0 :%s", nick, time(NULL), user, host, me.name, rname);
	AddUser(nick, user, host, me.name);
	sts(":%s MODE %s %s", nick, nick,modes);
	sts(":%s JOIN %s",nick ,me.chan);
	sts(":%s MODE %s +o %s",me.name,me.chan,nick);
	sts(":%s MODE %s +a %s",nick,me.chan,nick);
	return 1;
}

int del_bot(char *nick, char *reason)
{
	User *u;
	segv_location = "del_bot";
	u = finduser(nick);
	log("Killing %s for %s",nick,reason);
	if (!u) {
		log("Attempting to Logoff with a Nickname that does not Exists: %s",nick);
		return -1;
	}
	sts(":%s QUIT :%s",nick,reason);
	DelUser(nick);
	del_mod_user(nick);
	return 1;
}

		


void Module_Event(char *event, void *data) {
	Module *module_ptr;
	EventFnList *ev_list;


	segv_location = "Module_Event";
	module_ptr = module_list->next;
	while (module_ptr != NULL) {
		log("scanning Module %s for Events",module_ptr->info->module_name);
		/* this goes through each Module */
		ev_list = module_ptr->other_funcs;
		while (ev_list->cmd_name != NULL) {
			log("Scanning Module %s for Comamnd %s -> %s",module_ptr->info->module_name, event, ev_list->cmd_name);
			/* This goes through each Command */
			if (!strcasecmp(ev_list->cmd_name, event)) {
					segv_location = module_ptr->info->module_name;
					ev_list->function(data);			
					break;
			}
		ev_list++;
		}	
	module_ptr = module_ptr->next;
	}

}
void parse(char *line)
{
	char *origin, *cmd, *coreLine;
	int cmdptr = 0;
	int I = 0;
	Module *module_ptr;
	Functions *fn_list;
	Mod_User *list;
	
	segv_location = "parse";
	strip(line);
	recbuf = strdup(line);

	if (!(*line))
		return;

	coreLine = sstrdup(line);

#ifdef DEBUG
	log("R: %s", line);
#endif
	origin = strtok(line, " ");
	if (*origin == ':') {
		cmdptr = 1;
		origin++;
		cmd = strtok(NULL, " ");
	} else {
		cmd = origin;
	}
	coreLine = strtok(NULL, "");
        /* First, check if its a privmsg, and if so, handle it in the correct Function */
 	if (!strcasecmp("PRIVMSG",cmd)) {
 		/* its a privmsg, now lets see who too... */       
        	cmd = strtok(coreLine, " ");
		coreLine = strtok(NULL, "");
		coreLine++;
		/* coreLine contains the Actual Message now, cmd, is who its too */
		log("PRIVMSG TO: %s, coreline %s",cmd,coreLine);
		if (!strcasecmp(s_Services,cmd)) {
			/* its to the Internal Services Bot */
			segv_location = "servicesbot";
			servicesbot(origin,coreLine);
			return;
		} else {
			list = module_bot_lists;
			/* Check to see if any of the Modules have this nick Registered */
			while (list != NULL ) {
			log("nicks: %s", list->nick);
				if (!strcasecmp(list->nick,cmd)) {
					segv_location = list->modname;
					list->function(origin, coreLine);
					return;
				}
			list=list->next;
			}
			log("Recieved a Message for %s, but that user is not registered with us!!!", cmd);
		}
        }	
        	
        /* now, Parse the Command to the Internal Functions... */
	for (I=0; I < ((sizeof(cmd_list) / sizeof(cmd_list[0])) -1); I++) {
		if (!strcasecmp(cmd_list[I].name, cmd)) {
			if (cmd_list[I].srvmsg == cmdptr) {
				segv_location = cmd_list[I].name;
				cmd_list[I].function(origin, coreLine);
				break; log("should never get here-Parse");
			}	
		}
	}
	/* K, now Parse it to the Module functions */
	module_ptr = module_list->next;
	while (module_ptr != NULL) {
		/* this goes through each Module */
		fn_list = module_ptr->function_list;
		while (fn_list->cmd_name != NULL) {
			/* This goes through each Command */
			if (!strcasecmp(fn_list->cmd_name, cmd)) {
				if (fn_list->srvmsg == cmdptr) {
					segv_location = module_ptr->info->module_name;
					fn_list->function(origin, coreLine);			
					break;
					log("should never get here-Parse");
				}	
			}
		fn_list++;
		}	
	module_ptr = module_ptr->next;
	}
        	


}

/* Here are the Following Internal Functions.
they should update the internal Structures */

void Usr_Version(char *origin, char *coreLine) {
	sts(":%s 351 %s %s :%s -> %s %s", me.name, origin, version, me.name, version_date, version_time);
}
void Usr_ShowMOTD(char *origin, char *coreLine) {
	ShowMOTD(origin);
}
void Usr_Showcredits(char *origin, char *coreLine) {
	Showcredits(origin);
}
void Usr_AddServer(char *origin, char *coreLine){
	char *cmd;
	cmd = strtok(coreLine, " ");
	AddServer(cmd,1);
	Module_Event("NEWSERVER", findserver(cmd));
}
void Usr_DelServer(char *origin, char *coreLine){
	char *cmd;
	cmd = strtok(coreLine, " ");
	DelServer(cmd);
	Module_Event("DELSERVER", coreLine);
}
void Usr_DelUser(char *origin, char *coreLine) {
	DelUser(origin);
	Module_Event("SIGNOFF", coreLine);
}
void Usr_Mode(char *origin, char *coreLine) {
			char *rest, *cmd;
			cmd = strtok(coreLine, " ");
			if (!strchr(cmd, '#')) {
				log("Mode: UserMode: %s",cmd);
				cmd = strtok(NULL, "");
				UserMode(origin, cmd);
				Module_Event("UMODE", finduser(origin));
			} else {
/*
				log("Mode: ChanMode: %s",cmd);
				rest = strtok(NULL, "");
				ChanMode(cmd, rest);
				Module_Event("CMODE", coreLine);
*/
			}	
}	
void Usr_Kill(char *origin, char *coreLine) {
	char *cmd;
	cmd = strtok(coreLine, " ");
	DelUser(cmd);
	Module_Event("SIGNOFF", coreLine);
}
void Usr_Pong(char *origin, char *coreLine) {
			Server *s;
			char *cmd;
			cmd = strtok(coreLine, " ");
			s = findserver(cmd);
			if (s) {
				s->ping = time(NULL) - ping.last_sent;
				if (ping.ulag > 1)
					s->ping -= (float) ping.ulag;
				if (!strcmp(me.s->name, s->name))
					ping.ulag = me.s->ping;
				Module_Event("PONG", s);

			} else {
				log("Received PONG from unknown server: %s", cmd);
			}
}
void Usr_Away(char *origin, char *coreLine) {
			User *u = finduser(origin);
			if (u->is_away) {
				u->is_away = 0;
			} else {
				u->is_away = 1;
			}
			Module_Event("AWAY", u);
}	
void Usr_Nick(char *origin, char *coreLine) {
			char *cmd;
			User *u = finduser(origin);
			if (u) {
				cmd = strtok(coreLine, " ");
				Module_Event("NICK_CHANGE",coreLine);
				Change_User(u, cmd);
			}
}
void Usr_Topic(char *origin, char *coreLine) {
}
void Usr_Kick(char *origin, char *coreLine) {
}
void Usr_Join(char *origin, char *coreLine) {
/*	char *cmd, *temp = NULL;
			User *u = finduser(origin);
			cmd = strtok(coreLine, " ");
			if (*cmd == ':')
				cmd++;
			if (strcasecmp(",",cmd)) {
				log("double chan");
				temp = strtok(cmd,",");
			}
			if (u) {
				while (1) {
					Addchan(temp);
					temp = strtok(NULL,",");
					if (temp == NULL) { 
						log("Null: %s, %s",temp,cmd);
						temp = cmd;
						cmd = NULL;
					}
					if (temp == NULL) {
						log("break: %s",temp);
						break;
					}
				}
			}
*/
}
void Usr_Part(char *origin, char *coreLine) {
}
void Srv_Ping(char *origin, char *coreLine) {
			log("Ping: %s", coreLine);
			sts("PONG %s", coreLine);
}
void Srv_Netinfo(char *origin, char *coreLine) {
			char *cmd;
			cmd = strtok(coreLine, " ");
			cmd = strtok(NULL, " ");
			cmd = strtok(NULL, " ");
		        me.onchan = 1;
			sts(":%s NETINFO 0 %d %s 0 0 0 0 :%s",me.name,time(NULL),cmd,me.netname);
			globops(me.name,"Link with Network \2Complete!\2");
			#ifdef DEBUG
        			ns_debug_to_coders("By Compliation");
        		#endif
			if (!strcmp(cmd ,"2109")) {
				me.usesmo = 1;
			}
			Module_Event("NETINFO", coreLine); 
}
void Srv_Pass(char *origin, char *coreLine) {
}
void Srv_Server(char *origin, char *coreLine) {
			Server *s;
			AddServer(strtok(coreLine, " "), 1);
			s = findserver(coreLine);
			me.s = s;
			Module_Event("ONLINE", s);
			Module_Event("NEWSERVER", s);
}
void Srv_Squit(char *origin, char *coreLine) {
			Server *s;
			s = findserver(coreLine);
			if (s) {
				Module_Event("SQUIT", s);
				DelServer(coreLine);
			}
						
}
void Srv_Nick(char *origin, char *coreLine) {
			char *user, *host, *server, *cmd;
			log("start");
			cmd = strtok(coreLine, " ");
			user = strtok(NULL, " ");
			user = strtok(NULL, " ");
			user = strtok(NULL, " ");
			host = strtok(NULL, " ");
			server = strtok(NULL, " ");
			AddUser(cmd, user, host, server);
			Module_Event("SIGNON", finduser(cmd));
}
void Srv_Svsnick(char *origin, char *coreLine) {
			char *nnick;
			User *u;

			nnick = strtok(coreLine, " ");
			u = finduser(nnick);
			nnick = strtok(NULL, " ");
			Change_User(u, nnick);
			Module_Event("NICK_CHANGE",coreLine);

}		
void Srv_Kill(char *origin, char *coreLine) {
			char *user, *rest;
			user = strtok(coreLine, " ");
			rest = strtok(NULL, " ");

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
	sprintf(buf, ":%s GLOBOPS :%s", from, buf2);
	sts("%s", buf);
	va_end(ap);
}

int flood(User *u)
{
	time_t current = time(NULL);

	if (UserLevel(u) >= 40) /* locop or higher */
		return 0;
	if (current - u->t_flood > 10) {
		u->t_flood = time(NULL);
		u->flood = 0;
		return 0;
	}
	if (u->flood >= 5) {
		sts(":%s KILL %s :%s!%s (Flooding Services.)", s_Services,
			u->nick, Servbot.host, s_Services);
		log("FLOODING: %s!%s@%s", u->nick, u->username, u->hostname);
		DelUser(u->nick);
		return 1;
	} else {
		u->flood++;
	}
	return 0;
}

static void ShowMOTD(char *nick)
{
	static unsigned char copyright[38] = {  /* Can you tell I was bored? */
	0x47, 0x65, 0x6f, 0x53, 0x74, 0x61, 0x74, 0x73, 0x20, 0x61, 0x72, 0x65,
	0x20, 0x28, 0x63, 0x29, 0x20, 0x31, 0x39, 0x39, 0x39, 0x20, 0x4a, 0x6f,
	0x6e, 0x61, 0x74, 0x68, 0x61, 0x6e, 0x20, 0x47, 0x65, 0x6f, 0x72, 0x67,
	0x65, 0x0
	};

	/* Removal or modification of this function is against the law.
	** Be nice, and don't remove it or modify it.
	** -Jonathan 'netgod' George (net@lite.net) */

	sts(":%s 375 %s :- %s Message of the Day", me.name, nick, me.name);
	sts(":%s 372 %s :- ", me.name, nick);
	sts(":%s 372 %s :- Origonal Version Copyright:", me.name, nick);
	sts(":%s 372 %s :- %s", me.name, nick, copyright);
	sts(":%s 372 %s :- Grab your copy at codebase.kamserve.com", me.name, nick);
	sts(":%s 376 %s :End of /MOTD command.", me.name, nick);
}
static void Showcredits(char *nick)
{

	sts(":%s 351 %s :- %s Credits ", me.name, nick, version);
	sts(":%s 351 %s :- This Version of %s was based on GeoStats-1.1.0", me.name, nick, version);
	sts(":%s 351 %s :- Originally Written by Jonathan 'netgod' George (net@lite.net)", me.name, nick);
	sts(":%s 351 %s :- Now Maintained by Fish (fish@dynam.ac) and Shmad (Shmad@kamserve.com)", me.name, nick);
	sts(":%s 351 %s :- For Support, you can find Fish at irc.global-irc.net", me.name, nick);
	sts(":%s 351 %s :- or Shmad at irc.dreaming.org", me.name, nick);
	sts(":%s 351 %s :- Thanks to:", me.name, nick);
	sts(":%s 351 %s :- Stskeeps for Writting the best IRCD ever!", me.name, nick);
	sts(":%s 351 %s :- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)",me.name,nick);
	sts(":%s 351 %s :- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!", me.name, nick);
	sts(":%s 351 %s :- Andy For Ideas",me.name,nick);
	sts(":%s 351 %s :- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies",me.name,nick);
}
