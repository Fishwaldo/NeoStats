/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
**
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

void Usr_Version(EvntMsg *EM);
void Usr_ShowMOTD(EvntMsg *EM);
void Usr_ShowADMIN(EvntMsg *EM);
void Usr_Showcredits(EvntMsg *EM);
void Usr_AddServer(EvntMsg *EM);
void Usr_DelServer(EvntMsg *EM);
void Usr_DelUser(EvntMsg *EM);
void Usr_Mode(EvntMsg *EM);
void Usr_Smode(EvntMsg *EM);
void Usr_Kill(EvntMsg *EM);
void Usr_Pong(EvntMsg *EM);
void Usr_Away(EvntMsg *EM);
void Usr_Nick(EvntMsg *EM);
void Usr_Topic(EvntMsg *EM);
void Usr_Kick(EvntMsg *EM);
void Usr_Join(EvntMsg *EM);
void Usr_Part(EvntMsg *EM);
void Usr_Stats(EvntMsg *EM);
void Usr_Vhost(EvntMsg *EM);
void Srv_Topic(EvntMsg *EM);
void Srv_Ping(EvntMsg *EM);
void Srv_Netinfo(EvntMsg *EM);
void Srv_Pass(EvntMsg *EM);
void Srv_Server(EvntMsg *EM);
void Srv_Squit(EvntMsg *EM);
void Srv_Nick(EvntMsg *EM);
void Srv_Svsnick(EvntMsg *EM);
void Srv_Kill(EvntMsg *EM);
void Srv_Connect(EvntMsg *EM);
#ifdef ULTIMATE
void Srv_Vctrl(EvntMsg *EM);
#endif
#ifdef ULTIMATE3
void Srv_Svinfo(EvntMsg *EM);
void Srv_Burst(EvntMsg *EM);
void Srv_Sjoin(EvntMsg *EM);
#endif

void AddStringToList(char ***List,char S[],int *C);
void FreeList(char **List,int C);


static void ShowMOTD(char *);
static void ShowADMIN(char *);
static void Showcredits(char *);



struct int_commands {
	char *name;
	void (*function)();
	int srvmsg; /* Should this be a Server Message(1), or a User Message?(0) */
	int exec;
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
	{MSG_STATS,	Usr_Stats, 		1, 	0},
	{TOK_STATS,	Usr_Stats,		1, 	0},
	{MSG_SETHOST, 	Usr_Vhost,		1, 	0},
	{TOK_SETHOST, 	Usr_Vhost, 		1, 	0},
	{MSG_VERSION,	Usr_Version,		1, 	0},
	{TOK_VERSION,	Usr_Version,		1, 	0},
	{MSG_MOTD,	Usr_ShowMOTD,		1, 	0},
	{TOK_MOTD,	Usr_ShowMOTD,		1, 	0},
	{MSG_ADMIN,	Usr_ShowADMIN,		1, 	0},
	{TOK_ADMIN,	Usr_ShowADMIN,		1, 	0},
	{MSG_CREDITS,	Usr_Showcredits,	1, 	0},
	{TOK_CREDITS,	Usr_Showcredits,	1, 	0},
	{MSG_SERVER,	Usr_AddServer,		1, 	0},
	{TOK_SERVER,	Usr_AddServer,		1, 	0},
	{MSG_SQUIT,	Usr_DelServer,		1, 	0},
	{TOK_SQUIT,	Usr_DelServer,		1, 	0},
	{MSG_QUIT,	Usr_DelUser,		1, 	0},
	{TOK_QUIT,	Usr_DelUser,		1, 	0},
	{MSG_MODE,	Usr_Mode,		1, 	0},
	{TOK_MODE,	Usr_Mode,		1, 	0},
	{MSG_SVSMODE,	Usr_Smode,		1, 	0},
	{TOK_SVSMODE,	Usr_Smode,		1, 	0},
	{MSG_SVS2MODE,	Usr_Smode,		1, 	0},
	{TOK_SVS2MODE,	Usr_Smode,		1, 	0},
	{MSG_KILL,	Usr_Kill,		1, 	0},
	{TOK_KILL,	Usr_Kill,		1, 	0},
	{MSG_PONG,	Usr_Pong,		1, 	0},
	{TOK_PONG,	Usr_Pong,		1, 	0},
	{MSG_AWAY,	Usr_Away,		1, 	0},
	{TOK_AWAY,	Usr_Away,		1, 	0},
	{MSG_NICK,	Usr_Nick,		1, 	0},
	{TOK_NICK,	Usr_Nick,		1, 	0},
	{MSG_TOPIC,	Usr_Topic,		1, 	0},
	{TOK_TOPIC,	Usr_Topic,		1, 	0},
	{MSG_TOPIC, 	Usr_Topic, 		0, 	0},
	{TOK_TOPIC, 	Usr_Topic, 		0, 	0},
	{MSG_KICK,	Usr_Kick,		1, 	0},
	{TOK_KICK,	Usr_Kick,		1, 	0},
	{MSG_JOIN,	Usr_Join,		1, 	0},
	{TOK_JOIN,	Usr_Join,		1, 	0},
	{MSG_PART,	Usr_Part,		1, 	0},
	{TOK_PART,	Usr_Part,		1, 	0},
	{MSG_PING,	Srv_Ping,		0, 	0},
	{TOK_PING,	Srv_Ping,		0, 	0},
	{MSG_NETINFO,	Srv_Netinfo,		0, 	0},
	{TOK_NETINFO,	Srv_Netinfo,		0, 	0},
	{MSG_PASS,	Srv_Pass,		0, 	0},
	{TOK_PASS,	Srv_Pass,		0, 	0},
	{MSG_SERVER,	Srv_Server,		0, 	0},
	{TOK_SERVER,	Srv_Server,		0, 	0},
	{MSG_SQUIT,	Srv_Squit,		0, 	0},
	{TOK_SQUIT,	Srv_Squit,		0, 	0},
	{MSG_NICK,	Srv_Nick,		0, 	0},
	{TOK_NICK,	Srv_Nick,		0, 	0},
	{MSG_SVSNICK,	Srv_Svsnick,		0, 	0},
	{TOK_SVSNICK,	Srv_Svsnick,		0, 	0},
	{MSG_KILL,	Srv_Kill,		0, 	0},
	{TOK_KILL,	Srv_Kill,		0, 	0},
	{MSG_PROTOCTL, 	Srv_Connect, 		0, 	0},
	{TOK_PROTOCTL,	Srv_Connect,		0, 	0},
	{NULL,		NULL,			0, 	0}
};
#endif
#ifdef ULTIMATE
IntCommands cmd_list[] = {
	/* Command	Function		srvmsg*/
	{MSG_STATS,	Usr_Stats, 		1, 	0},
	{TOK_STATS,	Usr_Stats,		1, 	0},
	{MSG_SETHOST, 	Usr_Vhost,		1, 	0},
	{TOK_SETHOST, 	Usr_Vhost, 		1, 	0},
	{MSG_VERSION,	Usr_Version,		1, 	0},
	{TOK_VERSION,	Usr_Version,		1, 	0},
	{MSG_MOTD,	Usr_ShowMOTD,		1, 	0},
	{TOK_MOTD,	Usr_ShowMOTD,		1, 	0},
	{MSG_CREDITS,	Usr_Showcredits,	1, 	0},
	{TOK_CREDITS,	Usr_Showcredits,	1, 	0},
	{MSG_SERVER,	Usr_AddServer,		1, 	0},
	{TOK_SERVER,	Usr_AddServer,		1, 	0},
	{MSG_SQUIT,	Usr_DelServer,		1, 	0},
	{TOK_SQUIT,	Usr_DelServer,		1, 	0},
	{MSG_QUIT,	Usr_DelUser,		1, 	0},
	{TOK_QUIT,	Usr_DelUser,		1, 	0},
	{MSG_MODE,	Usr_Mode,		1, 	0},
	{TOK_MODE,	Usr_Mode,		1, 	0},
	{MSG_SVSMODE,	Usr_Smode,		1, 	0},
	{TOK_SVSMODE,	Usr_Smode,		1, 	0},
	{MSG_KILL,	Usr_Kill,		1, 	0},
	{TOK_KILL,	Usr_Kill,		1, 	0},
	{MSG_PONG,	Usr_Pong,		1, 	0},
	{TOK_PONG,	Usr_Pong,		1, 	0},
	{MSG_AWAY,	Usr_Away,		1, 	0},
	{TOK_AWAY,	Usr_Away,		1, 	0},
	{MSG_NICK,	Usr_Nick,		1, 	0},
	{TOK_NICK,	Usr_Nick,		1, 	0},
	{MSG_TOPIC,	Usr_Topic,		1, 	0},
	{TOK_TOPIC,	Usr_Topic,		1, 	0},
	{MSG_KICK,	Usr_Kick,		1, 	0},
	{TOK_KICK,	Usr_Kick,		1, 	0},
	{MSG_JOIN,	Usr_Join,		1, 	0},
	{TOK_JOIN,	Usr_Join,		1, 	0},
	{MSG_PART,	Usr_Part,		1, 	0},
	{TOK_PART,	Usr_Part,		1, 	0},
	{MSG_PING,	Srv_Ping,		0, 	0},
	{TOK_PING,	Srv_Ping,		0, 	0},
#ifndef ULTIMATE3
	{MSG_SNETINFO,	Srv_Netinfo,		0, 	0},
	{TOK_SNETINFO,	Srv_Netinfo,		0, 	0},
#endif
#ifdef ULTIMATE3
	{MSG_SVINFO,	Srv_Svinfo,		0, 	0},
	{MSG_CAPAB,	Srv_Connect, 		0, 	0},
	{MSG_BURST,	Srv_Burst, 		0, 	0},
	{MSG_SJOIN,	Srv_Sjoin,		1, 	0},
#endif
	{MSG_VCTRL,	Srv_Vctrl, 		0, 	0},
	{TOK_VCTRL,	Srv_Vctrl, 		0, 	0},
	{MSG_PASS,	Srv_Pass,		0, 	0},
	{TOK_PASS,	Srv_Pass,		0, 	0},
	{MSG_SERVER,	Srv_Server,		0, 	0},
	{TOK_SERVER,	Srv_Server,		0, 	0},
	{MSG_SQUIT,	Srv_Squit,		0, 	0},
	{TOK_SQUIT,	Srv_Squit,		0, 	0},
	{MSG_NICK,	Srv_Nick,		0, 	0},
	{TOK_NICK,	Srv_Nick,		0, 	0},
	{MSG_SVSNICK,	Srv_Svsnick,		0, 	0},
	{TOK_SVSNICK,	Srv_Svsnick,		0, 	0},
	{MSG_KILL,	Srv_Kill,		0, 	0},
	{TOK_KILL,	Srv_Kill,		0, 	0},
	{MSG_PROTOCTL, 	Srv_Connect, 		0, 	0},
	{TOK_PROTOCTL,	Srv_Connect,		0, 	0},
	{NULL,		NULL,			0, 	0}
};
#endif

void init_main() {
	if (usr_mds);
}


int init_bot(char *nick, char *user, char *host, char *rname, char *modes, char *mod_name)
{
	User *u;
	char cmd[63];
	EvntMsg *EM;
	strcpy(segv_location, "init_bot");
	u = finduser(nick);
	if (u) {
		log("Attempting to Login with a Nickname that already Exists: %s",nick);
		return -1;
	}
	add_mod_user(nick, mod_name);
#ifdef UNREAL
	snewnick_cmd(nick, user, host, rname);
	sumode_cmd(nick, nick, UMODE_SERVICES | UMODE_DEAF | UMODE_KIX);
#elif ULTIMATE
#ifdef ULTIMATE3
	snewnick_cmd(nick, user, host, rname, UMODE_SERVICES | UMODE_DEAF | UMODE_SBOT);
#elif
	snewnick_cmd(nick, user, host, rname);
	sumode_cmd(nick, nick, UMODE_SERVICES | UMODE_DEAF | UMODE_SBOT);
#endif
#endif
	sjoin_cmd(nick, me.chan);
	sprintf(cmd, "%s %s", nick, nick);
	schmode_cmd(nick, me.chan, "+oa", cmd);
//	Module_Event("SIGNON", EM);
	return 1;
}

int del_bot(char *nick, char *reason)
{
	User *u;
	EvntMsg *EM;
	strcpy(segv_location, "del_bot");
	u = finduser(nick);
#ifdef DEBUG
	log("Killing %s for %s",nick,reason);
#endif
	if (!u) {
		log("Attempting to Logoff with a Nickname that does not Exists: %s",nick);
		return -1;
	}
//	Module_Event("SIGNOFF", EM);
	squit_cmd(nick, reason);
	del_mod_user(nick);
	return 1;
}

		


void Module_Event(char *event, EvntMsg *EM) {
	Module *module_ptr;
	EventFnList *ev_list;
	hscan_t ms;
	hnode_t *mn;

	strcpy(segv_location, "Module_Event");
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		module_ptr = hnode_get(mn);
		ev_list = module_ptr->other_funcs;
		if (ev_list) {
			while (ev_list->cmd_name != NULL) {
				/* This goes through each Command */
				if (!strcasecmp(ev_list->cmd_name, event)) {
#ifdef DEBUG	
						log("Running Module %s for Comamnd %s -> %s",module_ptr->info->module_name, event, ev_list->cmd_name);
#endif	
						strcpy(segv_location, module_ptr->info->module_name);
						strcpy(segvinmodule, module_ptr->info->module_name);
						if (setjmp(sigvbuf) == 0) {
							ev_list->function(EM);			
						} else {
							log("setjmp() Failed, Can't call Module %s\n", module_ptr->info->module_name);
						}
						strcpy(segvinmodule, "");
						strcpy(segv_location, "Module_Event_Return");
						break;
				}
			ev_list++;
			}
		}	
	}

}


/* Taken from Epona - Thanks! */

/*************************************************************************/

/* split_buf:  Split a buffer into arguments and store the arguments in an
 *             argument vector pointed to by argv (which will be malloc'd
 *             as necessary); return the argument count.  If colon_special
 *             is non-zero, then treat a parameter with a leading ':' as
 *             the last parameter of the line, per the IRC RFC.  Destroys
 *             the buffer by side effect.
 */

EvntMsg *split_buf(char *buf, int colon_special)
{
    char *s;
    int flag = 0;
    EvntMsg *EM;

    EM = malloc(sizeof(EvntMsg));
    EM->ac = 0;
    EM->fc = 0;
    EM->cmdptr = 1;
    if (*buf == ':') {
    	/* its a origin */
	buf++;
	EM->origin = strtok(buf, " ");
	EM->cmd = strtok(NULL, " ");
    } else {
    	EM->origin = NULL;
	EM->cmd = strtok(buf, " ");
	EM->cmdptr = 0;
    }	      
    while ((s = strtok(NULL, " ")) != NULL) {
	if (*s == ':') {
		s++;
		flag = 1;
	}
	AddStringToList(&EM->av, s, &EM->ac);
	if (flag) strncat(EM->data, s, sizeof(EM->data));
	if (flag) strncat(EM->data, " ", sizeof(EM->data));
    }
    return EM;
}

extern char *joinbuf(char **av, int ac, int from) {
	int i;
	char *buf;

	buf = malloc(512);
	sprintf(buf, "%s", av[from]);
	for (i = from+1; i < ac; i++) {
		sprintf(buf, "%s %s", buf, av[i]);
	}
	return (char *)buf;
}


void parse(char *line)
{
	int I = 0;
	char *tonick;
	Module *module_ptr;
	Functions *fn_list;
	Mod_User *list;
	hscan_t ms;
	hnode_t *mn;
	EvntMsg *EM;
		
	strcpy(segv_location, "parse");
	strip(line);
	strcpy(recbuf, line);
	if (!(*line))
		return;
#ifdef DEBUG
	log("R: %s", line);
#endif

	EM = split_buf(line, 0);	
	if (EM->cmdptr == 1) {
		if (findserver(EM->origin)) {
			EM->s = findserver(EM->origin);
			EM->u = NULL;
			EM->isserv = 1;
		} else if (finduser(EM->origin)) {
			EM->u = finduser(EM->origin);
			EM->s = NULL;
			EM->u->ulevel = _UserLevel(EM->u);
			EM->isserv = 0;
		} else if (me.onchan) {
			log("Got Message from Unknown Origin, Ignoring!!!");
			log("Message was: %s", recbuf);
			EM->isserv = -1;
			EM->u = NULL;
			goto parend;
		}
	}
        /* First, check if its a privmsg, and if so, handle it in the correct Function */
 	if (!strcasecmp("PRIVMSG",EM->cmd) || (!strcasecmp("!",EM->cmd))) {
 		/* its a privmsg, now lets see who too... */       
		/* if its a message from our own internal bots, silently drop it */
                if (findbot(EM->origin)) {
			goto parend;
		}

		tonick = strtok(EM->av[0], "@");
		/* this handles PRIVMSG neostats@stats.neostat.net messages, and ordinary messages */
// TODO: still channel support for bots here

		if (*tonick == '#') {
			log("Channel Message to one of our Bots, on TODO list");
			goto parend;
		}
		if (!strcasecmp(s_Services,tonick)) {
			/* its to the Internal Services Bot */
			strcpy(segv_location, "servicesbot");
			servicesbot(EM);
			strcpy(segv_location, "ServicesBot_return");
			goto parend;
		} else {
			list = findbot(tonick);
			/* Check to see if any of the Modules have this nick Registered */
			if (list) {
#ifdef DEBUG
				log("nicks: %s", list->nick);
#endif

				/* Check to make sure there are no blank spaces so we dont crash */
			        if (strlen(EM->av[1]) >= 350) {
			                privmsg(EM->origin, s_Services, "command line too long!");
			                notice (s_Services,"%s tried to send a very LARGE command, we told them to shove it!", EM->origin);
					goto parend;
			        }

                                strcpy(segv_location, list->modname);
				strcpy(segvinmodule, list->modname);
				if (setjmp(sigvbuf) == 0) {
					list->function(EM);
				}
				strcpy(segvinmodule, "");
				strcpy(segv_location, "Return from Module Message");
				goto parend;
			}
			log("Recieved a Message for %s, but that user is not registered with us!!! buf: %s", EM->av[0], EM->av[1]);
		}
        }	
        	
        /* now, Parse the Command to the Internal Functions... */
	strcpy(segv_location, "Parse - Internal Functions");
	for (I=0; I < ((sizeof(cmd_list) / sizeof(cmd_list[0])) -1); I++) {
		if (!strcmp(cmd_list[I].name, EM->cmd)) {
			if (cmd_list[I].srvmsg == EM->cmdptr) {
				cmd_list[I].exec++;
				strcpy(segv_location, cmd_list[I].name);
				cmd_list[I].function(EM);
				break; log("should never get here-Parse");
			}	
		}
	}
	/* K, now Parse it to the Module functions */
	strcpy(segv_location, "Parse - Module Functions");
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		module_ptr = hnode_get(mn);
		fn_list = module_ptr->function_list;
		while (fn_list->cmd_name != NULL) {
			/* This goes through each Command */
			if (!strcasecmp(fn_list->cmd_name, EM->cmd)) {
				if (fn_list->srvmsg == EM->cmdptr) {
#ifdef DEBUG
					log("Running Module %s for Function %s", module_ptr->info->module_name, fn_list->cmd_name);
#endif
					strcpy(segv_location, module_ptr->info->module_name);
					strcpy(segvinmodule, module_ptr->info->module_name);
					if (setjmp(sigvbuf) == 0) {
						fn_list->function(EM);			
					}
					strcpy(segvinmodule, "");
					strcpy(segv_location, "Parse_Return_Module");
					break;
					log("Should never get here-Parse");
				}	
			}
		fn_list++;
		}	
	}
	parend:
	FreeList(EM->av, EM->ac);
/* its upto the calling function of EM->fndata to free it, as we don't know if we are allowed to free some function data */
//	for (I = 0; I< EM->fc; I++) {
//		if (EM->canfree[I] == 1) {
//			free(EM->fndata[I]);
//			EM->canfree[I] = 0;
//		}
//	}
	EM->fc = 0;
	free(EM);
}









/* Here are the Following Internal Functions.
they should update the internal Structures */

void init_ServBot()
{
	char rname[63];
	EvntMsg *EM;
	strcpy(segv_location, "init_ServBot");
	sprintf(rname, "/msg %s \2HELP\2", s_Services);
#ifdef ULTIMATE3
	sburst_cmd(1);
	snewnick_cmd(s_Services, Servbot.user, Servbot.host, rname, UMODE_SERVICES | UMODE_DEAF | UMODE_SBOT);
#else 
	snewnick_cmd(s_Services, Servbot.user, Servbot.host, rname);
#endif
#ifdef UNREAL
	sumode_cmd(s_Services, s_Services, UMODE_SERVICES | UMODE_DEAF | UMODE_KIX);
	sjoin_cmd(s_Services, me.chan);
	sprintf(rname, "%s %s", s_Services, s_Services);
	schmode_cmd(me.name, me.chan, "+oa", rname);
#elif !ULTIMATE
	sumode_cmd(s_Services, s_Services, UMODE_SERVICES | UMODE_DEAF | UMODE_SBOT);
#endif
	sjoin_cmd(s_Services, me.chan);
	sprintf(rname, "%s %s", s_Services, s_Services);
	schmode_cmd(s_Services, me.chan, "+oa", rname);
	me.onchan = 1;
//	Module_Event("SIGNON", EM);
}

#ifdef ULTIMATE3
void Srv_Sjoin(EvntMsg *EM) {
	long mode = 0;
	long mode1 = 0;
	char *modes;
	int ok = 1, i, j = 3;
	ModesParm *m;
	Chans *c;
	lnode_t *mn = NULL;
	list_t *tl;
	if (EM->ac <= 2) {
		modes = EM->av[1];
	} else {
		modes = EM->av[2];
	}
	if (*modes == '#') {
		join_chan(EM->u, modes);
		return;
	}		
	if (*modes != '+') {
		return;
	}
	tl = list_create(10);
	while (*modes) {
		for (i=0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) -1);i++) {
			if (*modes == cFlagTab[i].flag) {
				if (cFlagTab[i].parameters) {
					m = malloc(sizeof(ModesParm));
					m->mode = cFlagTab[i].mode;
					strcpy(m->param, EM->av[j]);										
					mn = lnode_create(m);
					if (!list_isfull(tl)) {
						list_append(tl, mn);
					} else {
						log("Eeeek, tl list is full in Svr_Sjoin(ircd.c)");
						assert(0);
					}
					j++;
				} else {
					mode1 |= cFlagTab[i].mode;
				}
			}
		}
	modes++;
	}	

	while (EM->ac > j) {
		modes = EM->av[j];
		while (ok == 1) {
			if (*modes == '@') {
				mode |= MODE_CHANOP;
				modes++;
			} else if (*modes == '*') {
				mode |= MODE_CHANADMIN;
				modes++;
			} else if (*modes == '%') {
				mode |= MODE_HALFOP;
				modes++;
			} else if (*modes == '+') {
				mode |= MODE_VOICE;
				modes++;
			} else {
				EM->fndata[0] = finduser(modes);
				EM->fc = 1;
				ok = 0;
			}
		}
		join_chan(EM->fndata[0], EM->av[1]);
		EM->fndata[1] = findchan(EM->av[1]);
		EM->fc = 2;
		ChangeChanUserMode(EM->fndata[1], EM->fndata[0], 1, mode);
		j++;
		ok = 1;
	}
	c = EM->fndata[1];
	c->modes = mode1;
	if (!list_isempty(tl)) {
		if (!list_isfull(c->modeparms)) {
			list_transfer(c->modeparms, tl, list_first(tl));
		} else {
			/* eeeeeeek, list is full! */
			log("Eeeek, c->modeparms list is full in Svr_Sjoin(ircd.c)");
			assert(0);
		}
	}
	list_destroy(tl);
}
void Srv_Burst(EvntMsg *EM) {
	if (EM->ac > 0) {
		if (ircd_srv.burst == 1) {
			sburst_cmd(0);
			ircd_srv.burst = 0;
		}
	} else {
		ircd_srv.burst = 1;
		init_ServBot();
	}
	
}
#endif
void Srv_Connect(EvntMsg *EM) {
	int i;

	for (i = 0; i < EM->ac; i++) {
		if (!strcasecmp("TOKEN", EM->av[i])) {
			me.token = 1;
		}
	}
#ifndef ULTIMATE3
	init_ServBot();
#endif
}


void Usr_Stats(EvntMsg *EM) {
	time_t tmp;
	time_t tmp2;
	int I;
#ifdef EXTAUTH
	int dl;
	int (*listauth)(User *u);
#endif
		
	if (EM->isserv != 0) {
		log("Recieved a Message from a Unknown User! (%s)", EM->origin);
	}
	if (!strcasecmp(EM->av[0], "u")) {
		/* server uptime - Shmad */ 
                int uptime = time (NULL) - me.t_start;
                snumeric_cmd(242, EM->u->nick, "Statistical Server up %d days, %d:%02d:%02d", uptime/86400, (uptime/3600) % 24, (uptime/60) % 60,
                uptime % 60);
	} else if (!strcasecmp(EM->av[0], "c")) {
		/* Connections */
		snumeric_cmd(214, EM->u->nick, "N *@%s * * %d 50", me.uplink, me.port);
		snumeric_cmd(213, EM->u->nick, "C *@%s * * %d 50", me.uplink, me.port);
	} else if (!strcasecmp(EM->av[0], "o")) {
		/* Operators */
#ifdef EXTAUTH
		dl = get_dl_handle("extauth");
		if (dl > 0) {
			listauth = dlsym((int *)dl, "__list_auth");
			if (listauth)  
				(*listauth)(EM->u);
		} else
#endif
		snumeric_cmd(243, EM->u->nick, "Operators think they are God, but you and I know they are not!");
	} else if (!strcasecmp(EM->av[0], "l")) {
		/* Port Lists */
		tmp = time(NULL) - me.lastmsg; 
		tmp2 = time(NULL) - me.t_start;
		snumeric_cmd(211, EM->u->nick, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		snumeric_cmd(241, EM->u->nick, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, me.SendM, me.SendBytes,me.RcveM , me.RcveBytes, tmp2, tmp);  	
	} else if (!strcasecmp(EM->av[0], "m")) {
		for (I=0; I < ((sizeof(cmd_list) / sizeof(cmd_list[0])) -1); I++) {
			if (cmd_list[I].exec > 0) {
			snumeric_cmd(212, EM->u->nick, "%s - %d", cmd_list[I].name, cmd_list[I].exec);
			}
		}
	}

	
	snumeric_cmd(219, EM->u->nick, "%s :End of /STATS report", EM->av[0]);
	notice(s_Services,"%s Requested Stats %s", EM->u->nick, EM->av[0]);
}

void Usr_Version(EvntMsg *EM) {
	snumeric_cmd(351, EM->u->nick, "%s :%s -> %s %s", version, me.name, version_date, version_time); 
}
void Usr_ShowMOTD(EvntMsg *EM) {
	ShowMOTD(EM->u->nick);
}
void Usr_ShowADMIN(EvntMsg *EM) {
	ShowADMIN(EM->u->nick);
}
void Usr_Showcredits(EvntMsg *EM) {
	Showcredits(EM->u->nick);
}
void Usr_AddServer(EvntMsg *EM){
	AddServer(EM->av[0],EM->u->nick,atoi(EM->av[1]));
	Module_Event("NEWSERVER", EM);
}
void Usr_DelServer(EvntMsg *EM){
	Module_Event("DELSERVER", EM);
	DelServer(EM->av[0]);
}
void Usr_DelUser(EvntMsg *EM) {
	Module_Event("SIGNOFF", EM);
	DelUser(EM->u->nick);
}
void Usr_Smode(EvntMsg *EM) {
#ifdef ULTIMATE3
	UserMode(EM->av[0], EM->av[2]);
#else
	UserMode(EM->av[0], EM->av[1]);
#endif
	Module_Event("UMODE", EM);
}
void Usr_Mode(EvntMsg *EM) {
			if (!strchr(EM->av[0], '#')) {
#ifdef DEBUG
				log("Mode: UserMode: %s",EM->av[0]);
#endif
				UserMode(EM->av[0], EM->av[1]);
				Module_Event("UMODE", EM);
			} else {
				ChanMode(EM->u->nick, EM->av, EM->ac);
			}	
}	
void Usr_Kill(EvntMsg *EM) {
	User *u;
	Mod_User *mod_ptr;
	
	mod_ptr = findbot(EM->av[0]);
	if (mod_ptr) { /* Oh Oh, one of our Bots has been Killed off! */
		Module_Event("BOTKILL", EM);
		DelUser(EM->av[0]);
		return;
	}
	u = finduser(EM->av[0]);
	if (u) {
		Module_Event("KILL", EM);
		DelUser(EM->av[0]);
	}
}
void Usr_Vhost(EvntMsg *EM) {
	User *u;
#ifndef ULTIMATE3
	u = finduser(EM->u->nick);
#else 
	u = finduser(EM->av[0]);
#endif
	if (u) {
#ifndef ULTIMATE3
		strcpy(u->vhost, EM->av[0]);
#else
		strcpy(u->vhost, EM->av[1]);
#endif
	}
}
void Usr_Pong(EvntMsg *EM) {
			Server *s;
			s = findserver(EM->av[0]);
			if (s) {
				s->ping = time(NULL) - ping.last_sent;
				if (ping.ulag > 1)
					s->ping -= (float) ping.ulag;
				if (!strcmp(me.s->name, s->name))
					ping.ulag = me.s->ping;
				Module_Event("PONG", EM);

			} else {
				log("Received PONG from unknown server: %s", EM->av[0]);
			}
}
void Usr_Away(EvntMsg *EM) {
			if (EM->u) {
				if (EM->u->is_away) {
					EM->u->is_away = 0;
				} else {
					EM->u->is_away = 1;
				}
				Module_Event("AWAY", EM);
			} else {
				log("Warning, Unable to find User %s for Away", EM->u->nick);
			}
}	
void Usr_Nick(EvntMsg *EM) {
			if (EM->u) {
				Change_User(EM->u, EM->av[0]);
				Module_Event("NICK_CHANGE", EM);
			}
}
void Usr_Topic(EvntMsg *EM) {
	char *buf;
	Chans *c;
	c = findchan(EM->av[0]);
	if (c) {
		buf = joinbuf(EM->av, EM->ac, 3);
		Change_Topic(EM->av[1], c, atoi(EM->av[2]), buf);
		free(buf);
		Module_Event("TOPICCHANGE", EM);
	} else {
		log("Ehhh, Can't find Channel %s", EM->av[0]);
	}
	

}

void Usr_Kick(EvntMsg *EM) {
	Module_Event("KICK", EM);
	part_chan(finduser(EM->av[1]), EM->av[0]);
	
}
void Usr_Join(EvntMsg *EM) {
	char *s, *t;
	t = EM->av[0];
	while (*(s=t)) {
		t = s + strcspn(s, ",");
                if (*t)
                	*t++ = 0;
		join_chan(EM->u, s);
//		EM->fndata[0] = malloc(strlen(s));
//		strncpy(EM->fndata[0], s, strlen(s));
//		EM->fc = 1;
		Module_Event("JOINCHAN", EM);
//		free(EM->fndata[0]);
	}
}
void Usr_Part(EvntMsg *EM) {
	part_chan(EM->u, EM->av[0]);
	Module_Event("PARTCHAN", EM);

}
void Srv_Ping(EvntMsg *EM) {
			spong_cmd(EM->av[0]);
#ifdef ULTIMATE3
			if (ircd_srv.burst) {
				sping_cmd(me.name, EM->av[0], EM->av[0]);
			}
#endif
}
#ifdef ULTIMATE
void Srv_Vctrl(EvntMsg *EM) {
		ircd_srv.uprot = atoi(EM->av[0]);
		ircd_srv.nicklg = atoi(EM->av[1]);
		ircd_srv.modex = atoi(EM->av[2]);
		ircd_srv.gc = atoi(EM->av[3]);
		strcpy(me.netname, EM->av[14]);
		vctrl_cmd();

}
#endif
#ifdef ULTIMATE3
void Srv_Svinfo(EvntMsg *EM) {
	ssvinfo_cmd();
}
#endif
#ifndef ULTIMATE3
void Srv_Netinfo(EvntMsg *EM) {
		        me.onchan = 1;
			ircd_srv.uprot = atoi(EM->av[2]);
			strcpy(ircd_srv.cloak, EM->av[3]);
			strcpy(me.netname, EM->av[7]);

			snetinfo_cmd();
			globops(me.name,"Link with Network \2Complete!\2");
#ifdef DEBUG
       			ns_debug_to_coders("");
#endif
			if (ircd_srv.uprot == 2109) {
				me.usesmo = 1;
			} 
			Module_Event("NETINFO", NULL); 
}
#endif

void Srv_Pass(EvntMsg *EM) {
}
void Srv_Server(EvntMsg *EM) {
			Server *s;
			if (!EM->origin) {
				AddServer(EM->av[0],me.name, atoi(EM->av[1]));
			} else {
				AddServer(EM->av[0],EM->origin, atoi(EM->av[1]));
			}
			s = findserver(EM->av[0]);
			me.s = s;
			Module_Event("ONLINE", EM);
			Module_Event("NEWSERVER", EM);
}
void Srv_Squit(EvntMsg *EM) {
			if (EM->s) {
				Module_Event("SQUIT", EM);
				DelServer(EM->av[0]);
			} else {
				log("Waring, Squit from Unknown Server %s", EM->av[0]);
			}
						
}
void Srv_Nick(EvntMsg *EM) {
#ifndef ULTIMATE3
			AddUser(EM->av[0], EM->av[3], EM->av[4], EM->av[5]);
			EM->fndata[0] = finduser(EM->av[0]);
			EM->fc = 1;
			Module_Event("SIGNON", EM);
#else
			AddUser(EM->av[0], EM->av[4], EM->av[5], EM->av[6]);
#ifdef DEBUG
			log("Mode: UserMode: %s",EM->av[3]);
#endif
			UserMode(EM->av[0], EM->av[3]);
			EM->fndata[0] = finduser(EM->av[0]);
			EM->fc = 1;
			Module_Event("SIGNON", EM);
			Module_Event("UMODE", EM);
			
#endif
}
void Srv_Svsnick(EvntMsg *EM) {
			User *u;

			u = finduser(EM->av[0]);
			Change_User(u, EM->av[1]);
			Module_Event("NICK_CHANGE",EM);

}		
void Srv_Kill(EvntMsg *EM) {
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
		skill_cmd(s_Services, u->nick, "%s!%s (Flooding Services.)", Servbot.host, s_Services);
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

    snumeric_cmd(375, nick, ":- %s Message of the Day -", me.name);
    snumeric_cmd(372, nick, ":- %s. Copyright (c) 1999 - 2002 The NeoStats Group", version);
    snumeric_cmd(372, nick, ":-");

    fp = fopen ("stats.motd", "r");

    if (fp)
    {
	while (fgets (buf, sizeof (buf), fp))
	{
	    buf[strlen (buf) - 1] = 0;
	    snumeric_cmd(372, nick, ":- %s", buf);
	}
	fclose (fp);
    }
	snumeric_cmd(376, nick, ":End of /MOTD command.");
}


/* Display the ADMIN Message from the external stats.admin file */
static void ShowADMIN(char *nick)
{
    FILE *fp;
    char buf[BUFSIZE];

    snumeric_cmd(256, nick, ":- %s NeoStats Admins -", me.name);
    snumeric_cmd(256, nick, ":- %s.  Copyright (c) 1999 - 2002 The NeoStats Group", version);

    fp = fopen ("stats.admin", "r");

    if (fp)
    {
	while (fgets (buf, sizeof (buf), fp))
	{
	    buf[strlen (buf) - 1] = 0;
	    snumeric_cmd(257, nick, ":- %s", buf);
	}
	fclose (fp);
    }
	snumeric_cmd(258, nick, ":End of /ADMIN command.");
}


static void Showcredits(char *nick)
{
	snumeric_cmd(351, nick, ":- %s Credits ",version);
	snumeric_cmd(351, nick, ":- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	snumeric_cmd(351, nick, ":- For Support, you can find ^Enigma^ or Shmad at");
	snumeric_cmd(351, nick, ":- irc.irc-chat.net #NeoStats");
	snumeric_cmd(351, nick, ":- Thanks to:");
	snumeric_cmd(351, nick, ":- \2Fish\2 still part of the team with patch submissions.");
	snumeric_cmd(351, nick, ":- Stskeeps for Writting the best IRCD ever!");
	snumeric_cmd(351, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)");
	snumeric_cmd(351, nick, ":- monkeyIRCD for the Module Segv Catching code");
	snumeric_cmd(351, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!");
	snumeric_cmd(351, nick, ":- Andy For Ideas");
	snumeric_cmd(351, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies");
	snumeric_cmd(351, nick, ":- sre and Jacob for development systems and access");
}

void AddStringToList(char ***List,char S[],int *C)
{
  int c;
  char **l;

  c = *C;
  l = *List;

  c++;
  if(c == 1)
    l = (char **)malloc(sizeof(char *));
  else
    l = (char **)realloc(l,c*sizeof(char *));
  l[c-1] = strdup(S);

  *C = c;
  *List = l;
}

void FreeList(char **List,int C)
{
  int i;
  for(i=0;i<C;i++)
    free(List[i]);
  if(C != 0)
    free(List);
}
