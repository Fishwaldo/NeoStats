/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      ircd.c, 
** Version: 1.11
** Date:    17/11/2001
*/
 
#include <setjmp.h>
#include "stats.h"
#include "dl.h"


extern const char version_date[], version_time[];
extern const char protocol_version[];

void Usr_Version(char *, char *);
void Usr_ShowMOTD(char *, char *);
void Usr_ShowADMIN(char *, char *);
void Usr_Showcredits(char *, char *);
void Usr_AddServer(char *, char *);
void Usr_DelServer(char *, char *);
void Usr_DelUser(char *, char *);
void Usr_Mode(char *, char *);
void Usr_Smode(char *, char *);
void Usr_Kill(char *, char *);
void Usr_Pong(char *, char *);
void Usr_Away(char *, char *);
void Usr_Nick(char *, char *);
void Usr_Topic(char *, char *);
void Usr_Kick(char *, char *);
void Usr_Join(char *, char *);
void Usr_Part(char *, char *);
void Usr_Stats(char *, char *);
void Usr_Vhost(char *, char *);
void Srv_Ping(char *, char *);
void Srv_Netinfo(char *, char *);
void Srv_Pass(char *, char *);
void Srv_Server(char *, char *);
void Srv_Squit(char *, char *);
void Srv_Nick(char *, char *);
void Srv_Svsnick(char *, char *);
void Srv_Kill(char *, char *);
void Srv_Connect();


static void ShowMOTD(char *);
static void ShowADMIN(char *);
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
 
#ifdef UNREAL
IntCommands cmd_list[] = {
	/* Command	Function		srvmsg*/
	{MSG_STATS,	Usr_Stats, 		1},
	{TOK_STATS,	Usr_Stats,		1},
	{MSG_SETHOST, 	Usr_Vhost,		1},
	{TOK_SETHOST, 	Usr_Vhost, 		1},
	{MSG_VERSION,	Usr_Version,		1},
	{TOK_VERSION,	Usr_Version,		1},
	{MSG_MOTD,	Usr_ShowMOTD,		1},
	{TOK_MOTD,	Usr_ShowMOTD,		1},
	{MSG_ADMIN,	Usr_ShowADMIN,		1},
	{TOK_ADMIN,	Usr_ShowADMIN,		1},
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
	{MSG_SVSMODE,	Usr_Smode,		1},
	{TOK_SVSMODE,	Usr_Smode,		1},
	{MSG_SVS2MODE,	Usr_Smode,		1},
	{TOK_SVS2MODE,	Usr_Smode,		1},
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
	{MSG_PROTOCTL, 	Srv_Connect, 		0},
	{TOK_PROTOCTL,	Srv_Connect,		0},
	{NULL,		NULL,			0}
};
#endif
#ifdef ULTIMATE
IntCommands cmd_list[] = {
	/* Command	Function		srvmsg*/
	{MSG_STATS,	Usr_Stats, 		1},
	{TOK_STATS,	Usr_Stats,		1},
	{MSG_SETHOST, 	Usr_Vhost,		1},
	{TOK_SETHOST, 	Usr_Vhost, 		1},
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
	{MSG_SVSMODE,	Usr_Smode,		1},
	{TOK_SVSMODE,	Usr_Smode,		1},
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
	{MSG_PROTOCTL, 	Srv_Connect, 		0},
	{TOK_PROTOCTL,	Srv_Connect,		0},
	{NULL,		NULL,			0}
};
#endif

int init_bot(char *nick, char *user, char *host, char *rname, char *modes, char *mod_name)
{
	User *u;
	char cmd[63];
	segv_location = sstrdup("init_bot");
	u = finduser(nick);
	if (u) {
		log("Attempting to Login with a Nickname that already Exists: %s",nick);
		return -1;
	}
	add_mod_user(nick, mod_name);
	snick_cmd(nick, user, host, rname);
	sumode_cmd(nick, nick, UMODE_SERVICES | UMODE_DEAF | UMODE_KIX);
	sjoin_cmd(nick, me.chan);
	sprintf(cmd, "%s %s", nick, nick);
	schmode_cmd(me.name, me.chan, "+oa", cmd);
	Module_Event("SIGNON", finduser(nick));
	return 1;
}

int del_bot(char *nick, char *reason)
{
	User *u;
	segv_location = sstrdup("del_bot");
	u = finduser(nick);
#ifdef DEBUG
	log("Killing %s for %s",nick,reason);
#endif
	if (!u) {
		log("Attempting to Logoff with a Nickname that does not Exists: %s",nick);
		return -1;
	}
	Module_Event("SIGNOFF", finduser(nick));
	squit_cmd(nick, reason);
	del_mod_user(nick);
	return 1;
}

		


void Module_Event(char *event, void *data) {
	Module *module_ptr;
	EventFnList *ev_list;
	hscan_t ms;
	hnode_t *mn;

	segv_location = sstrdup("Module_Event");
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		module_ptr = hnode_get(mn);
		ev_list = module_ptr->other_funcs;
		while (ev_list->cmd_name != NULL) {
			/* This goes through each Command */
			if (!strcasecmp(ev_list->cmd_name, event)) {
#ifdef DEBUG
					log("Running Module %s for Comamnd %s -> %s",module_ptr->info->module_name, event, ev_list->cmd_name);
#endif
					segv_location = sstrdup(module_ptr->info->module_name);
					strcpy(segvinmodule, module_ptr->info->module_name);
					if (setjmp(sigvbuf) == 0) {
						ev_list->function(data);			
					} else {
						log("setjmp() Failed, Can't call Module %s\n", module_ptr->info->module_name);
					}
					strcpy(segvinmodule, "");
					segv_location = sstrdup("Module_Event_Return");
					break;
			}
		ev_list++;
		}	
	}

}
void parse(char *line)
{
	char *origin, *cmd, *coreLine, *corelen;
	int cmdptr = 0;
	int I = 0;
	Module *module_ptr;
	Functions *fn_list;
	Mod_User *list;
	hscan_t ms;
	hnode_t *mn;
		
	segv_location = sstrdup("parse");
	strip(line);
	strcpy(recbuf, line);
	if (!(*line))
		return;

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
 	if (!strcasecmp("PRIVMSG",cmd) || (!strcasecmp("!",cmd))) {
 		/* its a privmsg, now lets see who too... */       
        	cmd = strtok(coreLine, " ");
		coreLine = strtok(NULL, "");
		coreLine++;
		/* coreLine contains the Actual Message now, cmd, is who its too */
                if (findbot(origin))
                return;

		if (!strcasecmp(s_Services,cmd)) {
			/* its to the Internal Services Bot */
			segv_location = sstrdup("servicesbot");
			servicesbot(origin,coreLine);
			segv_location = sstrdup("ServicesBot_return");
			return;
		} else {
			list = findbot(cmd);
			/* Check to see if any of the Modules have this nick Registered */
			if (list) {
#ifdef DEBUG
				log("nicks: %s", list->nick);
#endif
				/* if the message is from one of our bots, silently drop it, to stop floods */
				if (findbot(origin)) return;


				/* Check to make sure there are no blank spaces so we dont crash */
				corelen = smalloc(strlen(coreLine));
			        if (strlen(coreLine) >= 350) {
			                privmsg(origin, s_Services, "command line too long!");
			                notice (s_Services,"%s tried to send a very LARGE command, we told them to shove it!", origin);
			                return;
			        }

				strcpy(corelen, coreLine);
				if ((strtok(corelen, " ")) == NULL) {
					return;
				}
                                segv_location = sstrdup(list->modname);
				strcpy(segvinmodule, list->modname);
				if (setjmp(sigvbuf) == 0) {
					list->function(origin, coreLine);
				}
				strcpy(segvinmodule, "");
				segv_location = sstrdup("Return from Module Message");
				free(corelen);
				return;
			}
			log("Recieved a Message for %s, but that user is not registered with us!!! buf: %s", cmd, coreLine);
		}
        }	
        	
        /* now, Parse the Command to the Internal Functions... */
	segv_location = sstrdup("Parse - Internal Functions");
	for (I=0; I < ((sizeof(cmd_list) / sizeof(cmd_list[0])) -1); I++) {
		if (!strcasecmp(cmd_list[I].name, cmd)) {
			if (cmd_list[I].srvmsg == cmdptr) {
				segv_location = sstrdup(cmd_list[I].name);
				cmd_list[I].function(origin, coreLine);
				break; log("should never get here-Parse");
			}	
		}
	}
	/* K, now Parse it to the Module functions */
	segv_location = sstrdup("Parse - Module Functions");
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		module_ptr = hnode_get(mn);
		fn_list = module_ptr->function_list;
		while (fn_list->cmd_name != NULL) {
			/* This goes through each Command */
			if (!strcasecmp(fn_list->cmd_name, cmd)) {
				if (fn_list->srvmsg == cmdptr) {
#ifdef DEBUG
					log("Running Module %s for Function %s", module_ptr->info->module_name, fn_list->cmd_name);
#endif
					segv_location = sstrdup(module_ptr->info->module_name);
					strcpy(segvinmodule, module_ptr->info->module_name);
					if (setjmp(sigvbuf) == 0) {
						fn_list->function(origin, coreLine);			
					}
					strcpy(segvinmodule, "");
					segv_location = sstrdup("Parse_Return_Module");
					break;
					log("Should never get here-Parse");
				}	
			}
		fn_list++;
		}	
	}
        	

}




/* Here are the Following Internal Functions.
they should update the internal Structures */

void init_ServBot()
{
	char rname[63];
	segv_location = sstrdup("init_ServBot");

	sprintf(rname, "/msg %s \2HELP\2", s_Services);
	snick_cmd(s_Services, Servbot.user, Servbot.host, rname);
	sumode_cmd(s_Services, s_Services, UMODE_SERVICES | UMODE_DEAF | UMODE_KIX);
	sjoin_cmd(s_Services, me.chan);
	sprintf(rname, "%s %s", s_Services, s_Services);
	schmode_cmd(me.name, me.chan, "+oa", rname);
	me.onchan = 1;
	Module_Event("SIGNON", finduser(s_Services));
}




void Srv_Connect() {
	init_ServBot();
}





void Usr_Stats(char *origin, char *coreLine) {
	User *u;
	char *stats;
	time_t tmp;
	time_t tmp2;
		
	u=finduser(origin);
	if (!u) {
		log("Recieved a Message from a Unknown User! (%s)", origin);
	}
	stats = strtok(coreLine, " ");
	if (!strcasecmp(stats, "u")) {
		/* server uptime - Shmad */ 
                int uptime = time (NULL) - me.t_start;
                sts(":%s 242 %s :Statistical Server Up %d days, %d:%02d:%02d", me.name,
                u->nick, uptime/86400, (uptime/3600) % 24, (uptime/60) % 60,
                uptime % 60);
	} else if (!strcasecmp(stats, "c")) {
		/* Connections */
		sts(":%s 214 %s N *@%s * * %d 50", me.name, u->nick, me.uplink, me.port);
		sts(":%s 213 %s C *@%s * * %d 50", me.name, u->nick, me.uplink, me.port);
	} else if (!strcasecmp(stats, "o")) {
		/* Operators */
		sts(":%s 243 %s %s :Operators think they are God, but you and I know they are not!", me.name, u->nick, stats);
	} else if (!strcasecmp(stats, "l")) {
		/* Port Lists */
		tmp = time(NULL) - me.lastmsg; 
		tmp2 = time(NULL) - me.t_start;
		sts(":%s 211 %s l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE", me.name, u->nick);
		sts(":%s 241 %s %s 0 %d %d %d %d %d 0 :%d", me.name, u->nick, me.uplink, me.SendM, me.SendBytes,me.RcveM , me.RcveBytes, tmp2, tmp);  	
	}
	sts(":%s 219 %s %s :End of /STATS report", me.name, u->nick, stats);
	notice(s_Services,"%s Requested Stats %s", u->nick, stats);
}

void Usr_Version(char *origin, char *coreLine) {
	sts(":%s 351 %s %s :%s -> %s %s", me.name, origin, version, me.name, version_date, version_time);
}
void Usr_ShowMOTD(char *origin, char *coreLine) {
	ShowMOTD(origin);
}
void Usr_ShowADMIN(char *origin, char *coreLine) {
	ShowADMIN(origin);
}
void Usr_Showcredits(char *origin, char *coreLine) {
	Showcredits(origin);
}
void Usr_AddServer(char *origin, char *coreLine){
	char *cmd;
	cmd = strtok(coreLine, " ");
	AddServer(cmd,origin,1);
	Module_Event("NEWSERVER", findserver(cmd));
}
void Usr_DelServer(char *origin, char *coreLine){
	char *cmd;
	cmd = strtok(coreLine, " ");
	Module_Event("DELSERVER", coreLine);
	DelServer(cmd);
}
void Usr_DelUser(char *origin, char *coreLine) {
	Module_Event("SIGNOFF", finduser(origin));
	DelUser(origin);
}
void Usr_Smode(char *origin, char *coreLine) {
	char *cmd, *mode, tmp[25];
	cmd = strtok(coreLine, " ");
	mode = strtok(NULL, "");
	/* this is a hack */
	sprintf(tmp, ":%s", mode); 
	UserMode(cmd, tmp);
	Module_Event("UMODE", finduser(cmd));

}
void Usr_Mode(char *origin, char *coreLine) {
			char *cmd;
			cmd = strtok(coreLine, " ");
			if (!strchr(cmd, '#')) {
#ifdef DEBUG
				log("Mode: UserMode: %s",cmd);
#endif
				cmd = strtok(NULL, "");
				UserMode(origin, cmd);
				Module_Event("UMODE", finduser(origin));
			} else {
/* Shmad */
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
	User *u;
	Mod_User *mod_ptr;
	
	cmd = strtok(coreLine, " ");
	mod_ptr = findbot(cmd);
	if (mod_ptr) { /* Oh Oh, one of our Bots has been Killed off! */
		Module_Event("BOTKILL", cmd);
		DelUser(cmd);
		return;
	}
	u = finduser(cmd);
	if (u) {
		Module_Event("KILL", u);
		DelUser(cmd);
	}
}
void Usr_Vhost(char *origin, char *coreLine) {
	char *cmd;
	User *u;
	cmd = strtok(coreLine, " ");
	u = finduser(origin);
	if (u) {
		cmd = strtok(coreLine, " ");
		strcpy(u->vhost, cmd);
	}
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
			if (u) {
				if (u->is_away) {
					u->is_away = 0;
				} else {
					u->is_away = 1;
				}
				Module_Event("AWAY", u);
			} else {
				log("Warning, Unable to find User %s for Away", origin);
			}
}	
void Usr_Nick(char *origin, char *coreLine) {
			char *cmd, *cmd2;
			User *u = finduser(origin);
			if (u) {
				cmd = smalloc(255);
				snprintf(cmd, 255, "%s %s", u->nick, coreLine);
				cmd2 = strtok(coreLine, " ");
				Change_User(u, cmd2);
				Module_Event("NICK_CHANGE",cmd);
				free(cmd);

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

			sts("PONG %s", coreLine);
}
void Srv_Netinfo(char *origin, char *coreLine) {
			char *cmd;
			cmd = strtok(coreLine, " ");
			cmd = strtok(NULL, " ");
			cmd = strtok(NULL, " ");
		        me.onchan = 1;
#ifdef UNREAL
			ircd_srv.uprot = atoi(cmd);
			strcpy(ircd_srv.cloak, strtok(NULL, " "));
			cmd = strtok(NULL, " ");
			cmd = strtok(NULL, " ");
			cmd = strtok(NULL, " ");
			strcpy(me.netname, strtok(NULL, " "));
#endif

			sts(":%s NETINFO 0 %d %d %s 0 0 0 %s",me.name,time(NULL),ircd_srv.uprot, ircd_srv.cloak,me.netname);
			globops(me.name,"Link with Network \2Complete!\2");
			#ifdef DEBUG
        			ns_debug_to_coders("");
        		#endif
			if (ircd_srv.uprot == 2109) {
				me.usesmo = 1;
			} 
			Module_Event("NETINFO", coreLine); 
}
void Srv_Pass(char *origin, char *coreLine) {
}
void Srv_Server(char *origin, char *coreLine) {
			Server *s;
			AddServer(strtok(coreLine, " "),origin, 1);
			s = findserver(coreLine);
			me.s = s;
			Module_Event("ONLINE", s);
			Module_Event("NEWSERVER", s);
}
void Srv_Squit(char *origin, char *coreLine) {
			Server *s;
			s = findserver(strtok(coreLine, " "));
			if (s) {
				Module_Event("SQUIT", s);
				DelServer(strtok(coreLine, " "));
			} else {
				log("Waring, Squit from Unknown Server %s", strtok(coreLine, " "));
			}
						
}
void Srv_Nick(char *origin, char *coreLine) {
			char *user, *host, *server, *cmd;
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

/* Display our MOTD Message of the Day from the external stats.motd file */
static void ShowMOTD(char *nick)
{
    FILE *fp;
    char buf[BUFSIZE];

    sts(":%s 375 %s :- %s Message of the Day -", me.name, nick, me.name);
    sts(":%s 372 %s :- %s.  Copyright (c) 1999 - 2002 The NeoStats Group", me.name, nick, version);
    sts(":%s 372 %s :-", me.name, nick);

    fp = fopen ("stats.motd", "r");

    if (fp)
    {
	while (fgets (buf, sizeof (buf), fp))
	{
	    buf[strlen (buf) - 1] = 0;
	    sts(":%s 372 %s :- %s", me.name, nick, buf);
	}
	fclose (fp);
    }
	sts(":%s 376 %s :End of /MOTD command.", me.name, nick);
}


/* Display the ADMIN Message from the external stats.admin file */
static void ShowADMIN(char *nick)
{
    FILE *fp;
    char buf[BUFSIZE];

    sts(":%s 375 %s :- %s NeoStats Admins -", me.name, nick, me.name);
    sts(":%s 372 %s :- %s.  Copyright (c) 1999 - 2002 The NeoStats Group", me.name, nick, version);
    sts(":%s 372 %s :-", me.name, nick);

    fp = fopen ("stats.admin", "r");

    if (fp)
    {
	while (fgets (buf, sizeof (buf), fp))
	{
	    buf[strlen (buf) - 1] = 0;
	    sts(":%s 372 %s :- %s", me.name, nick, buf);
	}
	fclose (fp);
    }
	sts(":%s 376 %s :End of /ADMIN command.", me.name, nick);
}


static void Showcredits(char *nick)
{
	sts(":%s 351 %s :- %s Credits ", me.name, nick, version);
	sts(":%s 351 %s :- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)", me.name,  nick);
	sts(":%s 351 %s :- For Support, you can find ^Enigma^ or Shmad at", me.name, nick);
	sts(":%s 351 %s :- irc.irc-chat.net #NeoStats", me.name, nick);
	sts(":%s 351 %s :- Thanks to:", me.name, nick);
	sts(":%s 351 %s :- \2Fish\2 still part of the team with patch submissions.", me.name, nick);
	sts(":%s 351 %s :- Stskeeps for Writting the best IRCD ever!", me.name, nick);
	sts(":%s 351 %s :- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)",me.name,nick);
	sts(":%s 351 %s :- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!", me.name, nick);
	sts(":%s 351 %s :- Andy For Ideas",me.name,nick);
	sts(":%s 351 %s :- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies",me.name,nick);
	sts(":%s 351 %s :- sre and Jacob for development systems and access",me.name, nick);
}

