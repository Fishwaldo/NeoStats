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
** $Id: netinfo.c,v 1.10 2003/01/21 13:15:34 fishwaldo Exp $
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"

#include "ni_help.c"

const char netversion_date[] = __DATE__;
const char netversion_time[] = __TIME__;

extern const char *ni_help[];
extern const char *ni_help_set[];
extern const char *ni_help_oset[];
extern const char *ni_help_show[];
extern const char *ni_help_showo[];

char *s_Netinfo;
typedef struct ni_map_ NI_Map;
typedef struct ni_ignore_ NI_Ignore;
struct ni_map_ {
	NI_Map *next, *prev;
	char tld[MAXHOST];
	char channel[CHANLEN];
	char Message[255];
};
struct ni_ignore_ {
	NI_Ignore *next, *prev;
	char host[MAXHOST+9]; /* we need host and Ident :) */
	char nick[MAXNICK]; 
};

NI_Map *tldmap;
NI_Ignore *nickignore;

char Nchannel[25];
char Ochannel[25];
char Message[255];

void Loadconfig();
void Saveconfig();
static int s_sign_on(char **av, int ac);
static int s_mode(char **av, int ac);
int checktld(char *data);
static int new_m_version(char *origin, char **av, int ac);


Module_Info my_info[] = { {
	"NetInfo",
	"A Network Information Service",
	"1.0"
} };

/* S_sign_on 
** Run when someone signs onto the network 
** and it was a simple hack to put in the channel support :)
*/
static int s_sign_on(char **av, int ac) {
	NI_Map *map;
	NI_Ignore *ignore;
	char *tmp = NULL;
	char *tmpident = NULL;
	char *tmphost = NULL;
    User *u;
    /* Approximate Segfault Location */
    strcpy(segv_location, "Netinfo-s_sign_on");
    u = finduser(av[0]);

	if (findbot(u->nick)) return 1;
	/* make tmphost contain ident and host */
	/* first look for Ignores */
	for (ignore = nickignore; ignore; ignore = ignore->next) {
		if (fnmatch(ignore->nick, strlower(u->nick),0) == 0) {
			/* got a nick Match */
			tmp = sstrdup(ignore->host);
			tmpident = strtok(tmp, "@");
			tmphost = strtok(NULL, "");
			if (fnmatch(tmpident, strlower(u->username), 0) == 0) {
				if (fnmatch(tmphost, strlower(u->hostname), 0) == 0) {
					return 1;
				}
			}
		}
	}
	/* Now lets figure out what channel to join them */		
	for (map = tldmap; map; map = map->next) {
		if (fnmatch(map->tld, strlower(u->hostname), 0) == 0) {
			ssvsjoin_cmd(u->nick, map->channel);
			prefmsg(u->nick, s_Netinfo, "%s", map->Message);
			return 1;
		}
	}

	/* ok, the didnt fall into any defined tld, join them to the Default channel. */
	
	ssvsjoin_cmd(u->nick, Nchannel);
	prefmsg(u->nick, s_Netinfo, "%s", Message);
	return 1;
}

static int s_mode(char **av, int ac) {
	char *modes;
	int add = 0;
	User *u;

	/* Approximate Segfault Location */
	strcpy(segv_location, "Netinfo-s_mode");
	u = finduser(av[0]);


	if (findbot(u->nick)) return 1;
	modes = u->modes;
	while(*modes++) {
		switch(*modes) {
			case '+': add = 1;	break;
			case 'O':
			case 'o':
				if (add) {
					ssvsjoin_cmd(u->nick, Ochannel);
				}
		}
	}
	return 1;
}

int new_m_version(char *origin, char **av, int ac) {
        snumeric_cmd(351, origin, "Module Netinfo Loaded, Version: %s %s %s", my_info[0].module_version,netversion_date,netversion_time);
	return 0;
}

Functions my_fn_list[] = {
        { MSG_VERSION,  new_m_version,  1 },
#ifdef HAVE_TOKEN_SUP
        { TOK_VERSION,  new_m_version,  1 },
#endif
	{ NULL,		NULL,		0 }
};

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
	char *cmd, *tmp = NULL,*tmp2 = NULL;
	char *chan, *message;
	NI_Map *map;
	NI_Ignore *ignore;
	char *tmpcoreLine;
	
	u = finduser(origin);
	if (findbot(u->nick)) return 1;
	if (!u) {
		log("Unable to find user %s (NetInfo)", origin);
		return -1;
	}
	tmpcoreLine = sstrdup(coreLine);
	cmd = strtok(tmpcoreLine, " ");
	if (!strcasecmp(cmd, "SHOW")) {
		if (UserLevel(u) >= 50) {
			tmp = strtok(NULL, "");
			if (!tmp) { 
				/* show Settings */
#ifndef HYBRID7
				if (u->Umode & UMODE_REGNICK) {
#else
				if (1) {
#endif
					for (ignore = nickignore; ignore; ignore = ignore->next) {
						if (fnmatch(ignore->nick, strlower(u->nick),0) == 0) {
							/* got a nick Match */
							prefmsg(u->nick, s_Netinfo, "Your NickName will not be AutoJoined");
							break;
						} else {
							prefmsg(u->nick, s_Netinfo, "Your Nickname will be AutoJoined to a Channel");
						}
					}
				} else {
					prefmsg(u->nick, s_Netinfo, "You must Either Register your Nick with Nickserv, or Register your Nick with Services (/msg nickserv help register");
				}
				if (UserLevel(u) >= 150) {
					prefmsg(u->nick, s_Netinfo, "/2Also available to Net Administrators:/2");
					prefmsg(u->nick, s_Netinfo, "/msg %s show map -> Shows Current Domain to Channel Mapping", s_Netinfo);
					prefmsg(u->nick, s_Netinfo, "/msg %s show ignore -> Shows Current Nicks/Hosts to Not AutoJoin", s_Netinfo);
				} else {
			prefmsg(u->nick, s_Netinfo, "Access Denied");
		}
			} else if (!strcasecmp(tmp, "MAP")) {
				if (UserLevel(u) >= 50) {
					prefmsg(u->nick, s_Netinfo, "Domain to Channel Mapping:");
					for (map = tldmap; map; map = map->next) {
						prefmsg(u->nick, s_Netinfo, "Hosts *%s will be joined to Channel %s", map->tld, map->channel);
						prefmsg(u->nick, s_Netinfo, "Greeting Message: %s", map->Message);
					}
					prefmsg(u->nick, s_Netinfo, "End of List.");
				}  else {
			prefmsg(u->nick, s_Netinfo, "Access Denied");
		}
			} else if (!strcasecmp(tmp, "IGNORE")) {
				if (UserLevel(u) >= 50) {
					prefmsg(u->nick, s_Netinfo, "Nicks Hosts to not Autojoin:");
					for (ignore = nickignore; ignore; ignore = ignore->next) 
						prefmsg(u->nick, s_Netinfo, "Nick: %s Host: %s", ignore->nick, ignore->host);		
					prefmsg(u->nick, s_Netinfo, "End of List.");
				} else {
			prefmsg(u->nick, s_Netinfo, "Access Denied");
		}
			}
		} else {
			prefmsg(u->nick, s_Netinfo, "Access Denied");
		}
		return 1;
        } else if (!strcasecmp(cmd, "VERSION")) {
               prefmsg(u->nick, s_Netinfo,"\2NetInfo Version Information\2");
               prefmsg(u->nick, s_Netinfo,"%s - %s",me.name, my_info[0].module_version);
               return 1; 
	} else if (!strcasecmp(cmd, "SET")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			prefmsg(u->nick, s_Netinfo, "Incorrect Syntax");
			return 1;
		} else if (!strcasecmp(tmp, "AUTOJOIN")) {
			tmp2 = strtok(NULL, " ");
			if (!tmp2) {
				prefmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
#ifndef HYBRID7
			} else if (u->Umode & UMODE_REGNICK) {
#else
			} else if (1) {
#endif
				if (!strcasecmp(tmp2, "ON")) {
					prefmsg(u->nick, s_Netinfo, "Not Done yet");
				} else if (!strcasecmp(tmp2, "OFF")) {
					prefmsg(u->nick, s_Netinfo, "Not Done yet");
				} else {
					prefmsg(u->nick, s_Netinfo, "Incorrect Syntax. /msg %s SET AUTOJOIN ON/OFF", s_Netinfo);
				}	
			} else {
				prefmsg(u->nick, s_Netinfo, "You need to register your nick to use this service");
			}
			return 1;
		} else if (!strcasecmp(tmp, "MAP")) {
			if (UserLevel(u) < 150) {
				prefmsg(u->nick, s_Netinfo, "Access Denined");
				return 1;
			}
			tmp2 = strtok(NULL, " ");
			if (!tmp2) {
				prefmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
			} else if (!strcasecmp(tmp2, "ADD")) {
				tmp = strtok(NULL, " ");
				if (checktld(tmp) == 1) {
					chan = strtok(NULL, " ");
					if (!chan) {
						prefmsg(u->nick, s_Netinfo, "Incorrect Syntax");
						return 1;		
					}		
					message = strtok(NULL, "");
					if (!message) {
						prefmsg(u->nick, s_Netinfo, "Incorrect Syntax");
					}	
					map = smalloc(sizeof(NI_Map));
					strcpy(map->tld, tmp);
					strcpy(map->channel, chan);
					strcpy(map->Message, message);
					if (!tldmap) {
						tldmap = map;
						tldmap->next = NULL;
					} else {
						map->next = tldmap;
						tldmap = map;
					}
					prefmsg(u->nick, s_Netinfo, "Added Mapping: %s ==> %s with Message %s", tmp, chan, message);
					return 1;
				} else {
					prefmsg(u->nick, s_Netinfo, "The TLD Syntax is Incorrect eg: *@*.ac");
					return -1;
				}				
			} else if (!strcasecmp(tmp2, "DEL")) {
			
			} else {
				prefmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
			}	
			
			prefmsg(u->nick, s_Netinfo, "I'm lazy and havn't done this yet, so shoot me - Fish");
			return 1;
		}	
	} else if (!strcasecmp(cmd, "HELP")) {
		tmp = strtok(NULL, "");
		if (!tmp) { 
			privmsg_list(u->nick, s_Netinfo, ni_help);
		} else if (!strcasecmp(tmp, "SET")) {
			privmsg_list(u->nick, s_Netinfo, ni_help_set);
			if (UserLevel(u) >= 150) privmsg_list(u->nick, s_Netinfo, ni_help_seto);
		} else if (!strcasecmp(tmp, "SHOW")) {
			privmsg_list(u->nick, s_Netinfo, ni_help_show);
			if (UserLevel(u) >= 150) privmsg_list(u->nick, s_Netinfo, ni_help_showo);
		} 
	} 			
				
	chanalert(s_Netinfo, "%s Sent me this: %s", u->nick, coreLine);
	return 1;
}
int checktld(char *data) {
/*	if (strcasecmp(data, "@")) {
**		strtok(data, "@");
**		if (fnmatch(data, me.name, 1) > 0) return -1;
**	} else 	{
**		return -1;
**	} */
	return 1;	
}

int Online(char **av, int ac) {
	if (init_bot(s_Netinfo,"Service",me.name,"Network Information Service", "+SA", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_Netinfo = strcat(s_Netinfo, "_");
		init_bot(s_Netinfo,"Service",me.name,"Network Information Service", "+SA", my_info[0].module_name);
	}
	return 1;
};


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
	{ "SIGNON", 	s_sign_on},
	{ "UMODE", 	s_mode},
	{ NULL, 	NULL}
};



Module_Info *__module_get_info() {
	return my_info;
};

Functions *__module_get_functions() {
	return my_fn_list;
};

EventFnList *__module_get_events() {
	return my_event_list;
};

void _init() {
	s_Netinfo = "NetInfo";
	Loadconfig();


}


void _fini() {
	Saveconfig();

};
void Saveconfig()
{
	FILE *fp = fopen("data/niconfig.db", "w");
	NI_Map *map;
	NI_Ignore *ignore;

	if (!fp) {
		log("Unable to open niconfig.db for writing.");
		return;
	}

	for (map = tldmap; map; map = map->next) {
#ifdef DEBUG
		log("Writing netinfo database for %s", map->tld);
#endif
		fprintf(fp, "%s %s %s\n", map->tld, map->channel, map->Message);
	}
	fprintf(fp, "DEFAULT %s %s\n", Nchannel, Message);
	fprintf(fp, "OPER %s\n", Ochannel);
	fprintf(fp, "%s", "#IGNORE\n");
	for (ignore = nickignore; ignore; ignore = ignore->next) {
		fprintf(fp, "%s %s\n",ignore->nick, ignore->host);
#ifdef DEBUG
		log("Writing netinfo database for %s", ignore->nick);
#endif
	}
	fclose(fp);
}
void Loadconfig()
{
	FILE *fp = fopen("data/niconfig.db", "r");
	NI_Map *map;
	NI_Ignore *ignore;
	char buf[BUFSIZE];
	char *tmp;

	if (fp) {
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			/* log("Buf: %s", buf); */
			if (!strcasecmp(buf, "#IGNORE")) { 
				break;
			}
			map = smalloc(sizeof(NI_Map));
			tmp = strtok(buf, " ");
			if (!strcasecmp(tmp, "DEFAULT")) {
				memcpy(Nchannel, strtok(NULL, " "), 25);
				memcpy(Message, strtok(NULL, ""), 255);
			} else if (!strcasecmp(tmp, "OPER")) {
				memcpy(Ochannel, strtok(NULL, " "), 25);
			} else {
				strcpy(map->tld, tmp);
				strcpy(map->channel, strtok(NULL, " "));
				strcpy(map->Message, strtok(NULL, ""));
				if (!tldmap) {
					tldmap = map;
					tldmap->next = NULL;
				} else {
					map->next = tldmap;
					tldmap = map;
				}
			}			
#ifdef DEBUG
		log("Got Channel Mapping for %s", map->channel);
#endif
		}
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			/* log("Buf: %s", buf); */
			ignore = smalloc(sizeof(NI_Ignore));
			strcpy(ignore->nick,strtok(buf, " "));
			strcpy(ignore->host,strtok(NULL, " "));		
			if (!nickignore) {
				nickignore = ignore;
				nickignore->next = NULL;
			} else {
				ignore->next = nickignore;
				nickignore = ignore;
			}
#ifdef DEBUG
			log("Got ignore Mapping for %s", ignore->host);
#endif
		}
		fclose(fp);
	} else {
		chanalert(s_Services, "No Database Found! AutoJoin is Disabled!");
	}
}
