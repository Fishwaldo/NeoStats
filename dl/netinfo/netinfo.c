/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: netinfo.c,v 1.2 2000/06/10 08:48:54 fishwaldo Exp $
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
typedef struct ni_vhelper_ NI_Vhelper;
typedef struct ni_dccdeny_ NI_Dccdeny;
struct ni_map_ {
	NI_Map *next, *prev;
	long hash;
	char tld[MAXHOST];
	char channel[CHANLEN];
	char Message[255];
};
struct ni_ignore_ {
	NI_Ignore *next, *prev;
	long hash;
	char host[MAXHOST+9]; /* we need host and Ident :) */
	char nick[MAXNICK]; 
};
struct ni_vhelper_ {
	NI_Vhelper *next, *prev;
	long hash;
	char nick[MAXNICK];
	int level;
};
struct ni_dccdeny_ {
	NI_Dccdeny *next, *prev;
	long hash;
	char file[255];
	char message[255];
	char who[MAXNICK];
};

NI_Map *tldmap;
NI_Ignore *nickignore;
NI_Vhelper *vhelper;
NI_Dccdeny *dccdeny;

char Nchannel[25];
char Ochannel[25];
char Message[255];

#define M_TABLE_SIZE 255
#define DC_TABLE_SIZE 255
#define V_TABLE_SIZE 255
#define I_TABLE_SIZE 255


NI_Map *maplist[M_TABLE_SIZE];
NI_Ignore *ignorelist[I_TABLE_SIZE];
NI_Vhelper *helperlist[V_TABLE_SIZE];
NI_Dccdeny *dcclist[DC_TABLE_SIZE];

void Loadconfig();
void Saveconfig();
static int s_sign_on(User *);
static int s_mode(User *);
static int s_new_server(Server *);
int checktld(char *data);

static void add_map_to_hash_table(char *, NI_Map *);
static void del_map_from_hash_table(char *, NI_Map *);
static void add_ignore_to_hash_table(char *, NI_Ignore *);
static void del_ignore_from_hash_table(char *, NI_Ignore *);
static void add_vhelper_to_hash_table(char *, NI_Vhelper *);
static void del_vhelper_from_hash_table(char *, NI_Vhelper *);
static void add_dccdeny_to_hash_table(char *, NI_Dccdeny *);
static void del_dccdeny_from_hash_table(char *, NI_Dccdeny *);

static NI_Map *new_map(char *name) {
	NI_Map *m;
	m = smalloc(sizeof(NI_Map));
	if (!name) 
		name = "";
	memcpy(m->tld, name, MAXHOST);
	add_map_to_hash_table(name, m);
	return m;
}
static NI_Ignore *new_ignore(char *name) {
	NI_Ignore *i;
	i = smalloc(sizeof(NI_Ignore));
	if (!name)
		name = "";
	memcpy(i->nick, name, 255);
	add_ignore_to_hash_table(name, i);
	return i;
}
static NI_Vhelper *new_helper(char *name) {
	NI_Vhelper *v;
	v = smalloc(sizeof(NI_Vhelper));
	if (!name)
		name = "";
	memcpy(v->nick, name, 255);
	add_vhelper_to_hash_table(name, v);
	return v;
}
static NI_Dccdeny *new_dcc(char *name) {
	NI_Dccdeny *d;
	d = smalloc(sizeof(NI_Dccdeny));
	if (!name)
		name = "";
	memcpy(d->file, name, 255);
	add_dccdeny_to_hash_table(name, d);
	return d;
}
static void add_map_to_hash_table(char *name, NI_Map *m) {
	m->hash = HASH(name, M_TABLE_SIZE);
	m->next = maplist[m->hash];
	maplist[m->hash] = (void *)m;
}
static void add_ignore_to_hash_table(char *name, NI_Ignore *i) {
	i->hash = HASH(name, I_TABLE_SIZE);
	i->next = ignorelist[i->hash];
	ignorelist[i->hash] = (void *)i;
}
static void add_vhelper_to_hash_table(char *name, NI_Vhelper *v) {
	v->hash = HASH(name, V_TABLE_SIZE);
	v->next = helperlist[v->hash];
	helperlist[v->hash] = (void *)v;
}
static void add_dccdeny_to_hash_table(char *name, NI_Dccdeny *d) {
	d->hash = HASH(name, DC_TABLE_SIZE);
	d->next = dcclist[d->hash];
	dcclist[d->hash] = (void *)d;
}
static void del_dccdeny_from_hash_table(char *name, NI_Dccdeny *d) {
	NI_Dccdeny *tmp, *prev = NULL;
	
	for (tmp = dcclist[d->hash]; tmp; tmp = tmp->next) {
		if (tmp == d) {
			if (prev)
				prev->next = tmp->next;
			else
				dcclist[d->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

static void del_vhelper_from_hash_table(char *name, NI_Vhelper *v) {
	NI_Vhelper *tmp, *prev = NULL;
	
	for (tmp = helperlist[v->hash]; tmp; tmp = tmp->next) {
		if (tmp == v) {
			if (prev)
				prev->next = tmp->next;
			else
				helperlist[v->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}
static void del_map_from_hash_table(char *name, NI_Map *m) {
	NI_Map *tmp, *prev = NULL;
	
	for (tmp = maplist[m->hash]; tmp; tmp = tmp->next) {
		if (tmp == m) {
			if (prev)
				prev->next = tmp->next;
			else
				maplist[m->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}
static void del_ignore_from_hash_table(char *name, NI_Ignore *i) {
	NI_Ignore *tmp, *prev = NULL;
	
	for (tmp = ignorelist[i->hash]; tmp; tmp = tmp->next) {
		if (tmp == i) {
			if (prev)
				prev->next = tmp->next;
			else
				ignorelist[i->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

Module_Info my_info[] = { {
	"NetInfo",
	"A Network Information Service",
	"1.0"
} };

static int s_new_server(Server *s) {
	int i;
	NI_Dccdeny *dcc;

	sts(":%s SVSFLINE *", me.name);
	for (i = 0; i < I_TABLE_SIZE; i++) {
		for (dcc = dcclist[i]; dcc; dcc = dcc->next) {
			sts(":%s SVSFLINE + %s :%s", me.name, dcc->file, dcc->message);
		}
	}
	return 1;
}

/* S_sign_on 
** Run when someone signs onto the network 
** and it was a simple hack to put in the channel support :)
*/
static int s_sign_on(User *u) {
	NI_Map *map;
	NI_Ignore *ignore;
	char *tmp = NULL;
	char *tmpident = NULL;
	char *tmphost = NULL;

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
			sts(":%s SAJOIN %s %s", s_Netinfo, u->nick, map->channel);
			sts(":%s NOTICE %s :%s", s_Netinfo, u->nick, map->Message);
			return 1;
		}
	}

	/* ok, the didnt fall into any defined tld, join them to the Default channel. */
	
	sts(":%s SAJOIN %s %s", s_Netinfo, u->nick, Nchannel);
	sts(":%s NOTICE %s :%s", s_Netinfo, u->nick, Message);
	return 1;
}

static int s_mode(User *u) {
	char *modes;
	NI_Vhelper *helper;
	int i;
	int add = 0;

	if (findbot(u->nick)) return 1;
	modes = u->modes;
	while(*modes++) {
		switch(*modes) {
			case '+': add = 1;	break;
			case 'O':
			case 'o':
				if (add) {
					sts(":%s sajoin %s %s", s_Netinfo, u->nick, Ochannel);
				}
			case 'r':
				if (add) {
					for (i = 0; i < I_TABLE_SIZE; i++) {
						for (helper = helperlist[i]; helper; helper = helper->next) {
							if (!strcasecmp(u->nick, helper->nick)) {
								sts("%s SWHOIS %s :Is a Official Virus Helper", me.name, u->nick);
								return 1;
							}
						}
					}
				}					
		}
	}
	return 1;
}

int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module NetInfo Loaded, Version: %s %s %s",me.name,av,my_info[0].module_version,netversion_date,netversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u, *target;
	char *cmd, *tmp = NULL,*tmp2 = NULL;
	char *chan, *message;
	NI_Map *map;
	NI_Ignore *ignore;
	NI_Vhelper *helper;
	NI_Dccdeny *dcc;
	char *tmpcoreLine;
	int i, vlevel;;
	
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
				privmsg(u->nick, s_Netinfo, "Invalid Option, Please /msg %s help show", s_Netinfo);
			} else if (!strcasecmp(tmp, "MAP")) {
				privmsg(u->nick, s_Netinfo, "\2Domain to Channel Mapping:\2");
				for (i = 0; i < M_TABLE_SIZE; i++) {			
					for (map = maplist[i]; map; map = map->next) {
						privmsg(u->nick, s_Netinfo, "Hosts %s will be joined to Channel %s", map->tld, map->channel);
						privmsg(u->nick, s_Netinfo, "Greeting Message: %s", map->Message);
					}
				}
				privmsg(u->nick, s_Netinfo, "End of List.");
			} else if (!strcasecmp(tmp, "IGNORE")) {
				privmsg(u->nick, s_Netinfo, "Nicks Hosts to not Autojoin:");
				for (i = 0; i < I_TABLE_SIZE; i++) {
					for (ignore = ignorelist[i]; ignore; ignore = ignore->next) 
						privmsg(u->nick, s_Netinfo, "Nick: %s Host: %s", ignore->nick, ignore->host);		
				}
				privmsg(u->nick, s_Netinfo, "End of List.");
			} else if (!strcasecmp(tmp, "VHELPER")) {
				privmsg(u->nick, s_Netinfo, "Virus Helpers list:");
				for (i = 0; i < V_TABLE_SIZE; i++) {
					for (helper = helperlist[i]; helper; helper = helper->next) 
						privmsg(u->nick, s_Netinfo, "Nick: %s Level: %d", helper->nick, helper->level);
				}
			} else if (!strcasecmp(tmp, "DCCDENY")) {
				privmsg(u->nick, s_Netinfo, "DCC Deny File List:");
				for (i = 0; i < DC_TABLE_SIZE; i++) {
					for (dcc = dcclist[i]; dcc; dcc = dcc->next) {
						privmsg(u->nick, s_Netinfo, "File: %s Added By: %s", dcc->file, dcc->who);
						privmsg(u->nick, s_Netinfo, "DCC Deny Message: %s", dcc->message);
					}
				}
			}
		} else {
			privmsg(u->nick, s_Netinfo, "Access Denied");
		}
		return 1;
	} else if (!strcasecmp(cmd, "VJOIN" )) {
		vlevel = 0;
		for (i = 0; i < V_TABLE_SIZE; i++ ) {
			for (helper = helperlist[i]; helper; helper = helper->next) {
				if (!strcasecmp(helper->nick, u->nick)) {
					vlevel = helper->level;
					break;
				}
			}
		}
		if ((vlevel < 50) | (UserLevel(u) < 40)) {
			privmsg(u->nick, s_Netinfo, "Access Denied");
			return 1;
		}
		if (u->Umode & UMODE_REGNICK) {
		tmp2 = strtok(NULL, "");
		if (!tmp2) {
			privmsg(u->nick, s_Netinfo, "Error, you must specify a Nick");
			return 1;
		}
		sts(":%s SVSJOIN %s 0,#GVD", me.name, tmp2);
		notice(s_Netinfo, "\2VJOIN\2 %s Made %s Join #GVD and Part all other Channels", u->nick, tmp2);  
		sts(":%s SMO o :\2VJOIN\2 %s was Joined to #GVD by request of %s", me.name, tmp2, u->nick);
		return 1;
		} else {
			privmsg(u->nick, s_Netinfo, "Error, you must Identify with NickServ First");
		}
	} else if (!strcasecmp(cmd, "SET")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_Netinfo, "Incorrect Syntax. /msg %s help set", s_Netinfo);
			return 1;
		} else if (!strcasecmp(tmp, "MAP")) {
			if (UserLevel(u) < 150) {
				privmsg(u->nick, s_Netinfo, "Access Denined");
				return 1;
			}
			tmp2 = strtok(NULL, " ");
			if (!tmp2) {
				privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
			} else if (!strcasecmp(tmp2, "ADD")) {
				tmp = strtok(NULL, " ");
				if (checktld(tmp) == 1) {
					chan = strtok(NULL, " ");
					if (!chan) {
						privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
						return 1;		
					}		
					message = strtok(NULL, "");
					if (!message) {
						privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
					}	
					map = new_map(tmp);
					strcpy(map->channel, chan);
					strcpy(map->Message, message);
					privmsg(u->nick, s_Netinfo, "Added Mapping: %s ==> %s with Message %s", tmp, chan, message);
					return 1;
				} else {
					privmsg(u->nick, s_Netinfo, "The TLD Syntax is Incorrect eg: *@*.ac");
					return -1;
				}				
			} else if (!strcasecmp(tmp2, "DEL")) {
				tmp = strtok(NULL, " ");
				if (!tmp) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. /msg %s help set", s_Netinfo);
				} else {
					log("del %s", tmp);
					for (i = 0; i < M_TABLE_SIZE; i++) {
						for (map = maplist[i]; map; map = map->next) {
							if (!strcasecmp(tmp, map->tld)) {
								del_map_from_hash_table(tmp, map);
								privmsg(u->nick, s_Netinfo, "Deleted Mapping for Domain %s", tmp);
								return 1;
							}
						}
					}
					privmsg(u->nick, s_Netinfo, "Error, Can not find the TLD %s", tmp);
					return -1;
				}			
			} else {
				privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
			}	
		} else if (!strcasecmp(tmp, "IGNORE")) {
			if (UserLevel(u) < 150) {
				privmsg(u->nick, s_Netinfo, "Access Denined");
				return 1;
			}
			tmp2 = strtok(NULL, " ");
			if (!tmp2) {
				privmsg(u->nick, s_Netinfo, "Incorrect Syntax. /msg %s help SET IGNORE");
				return 1;
			} else if (!strcasecmp(tmp2, "ADD")) {					
				chan = strtok(NULL, " ");
				if (!chan) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You must specify a Nick");
					return 1;		
				}		
				message = strtok(NULL, "");
				if (!message) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You must Specify a Hostmask (eg, fish@localhost)");
				}	
				ignore = new_ignore(chan);
				strcpy(ignore->host, message);
				privmsg(u->nick, s_Netinfo, "Added Ignore: %s!%s", chan, message);
				return 1;
			} else if (!strcasecmp(tmp2, "DEL")) {
				tmp = strtok(NULL, " ");
				if (!tmp) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You Must Specify a Mask", s_Netinfo);
				} else {
					for (i = 0; i < I_TABLE_SIZE; i++) {
						for (ignore = ignorelist[i]; ignore; ignore = ignore->next) {
							if (!strcasecmp(tmp, ignore->host)) {
								del_ignore_from_hash_table(tmp, ignore);
								privmsg(u->nick, s_Netinfo, "Deleted Ignore list for %s", tmp);
								return 1;
							}
						}
					}
					privmsg(u->nick, s_Netinfo, "Error, Can not find the Ignore for %s", tmp);
					return -1;
				}			
			} else {
				privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
			}	
		} else if (!strcasecmp(tmp, "VHELPER")) {
			if (UserLevel(u) < 190) {
				privmsg(u->nick, s_Netinfo, "Access Denined");
				return 1;
			}
			tmp2 = strtok(NULL, " ");
			if (!tmp2) {
				privmsg(u->nick, s_Netinfo, "Incorrect Syntax. Either ADD/DEL");
				return 1;
			} else if (!strcasecmp(tmp2, "ADD")) {					
				chan = strtok(NULL, " ");
				if (!chan) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You must specify a Nick");
					return 1;		
				}		
				target = finduser(chan);
				if (!target) {
					privmsg(u->nick, s_Netinfo, "Error, the User must be online to add them");
					return 1;
				}
				if (target->Umode & UMODE_REGNICK) {	
					/* registered nick, its ok */				
					message = strtok(NULL, "");
					if (!message) {
						privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You must Specify a level");
					}	
					helper = new_helper(chan);
					helper->level = atoi(message);
					privmsg(u->nick, s_Netinfo, "Added Virus Helper %s at Level %s", chan, message);
					return 1;
				} else {
					/* nick isn't registered */
					privmsg(u->nick, s_Netinfo, "Error, The user must have Registered their Nick, and Identified");
					return -1;
				}
			} else if (!strcasecmp(tmp2, "DEL")) {
				tmp = strtok(NULL, " ");
				if (!tmp) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You Must Specify a nick", s_Netinfo);
				} else {
					for (i = 0; i < I_TABLE_SIZE; i++) {
						for (helper = helperlist[i]; helper; helper = helper->next) {
							if (!strcasecmp(tmp, helper->nick)) {
								del_vhelper_from_hash_table(tmp, helper);
								privmsg(u->nick, s_Netinfo, "Deleted Virus Helper %s", tmp);
								return 1;
							}
						}
					}
					privmsg(u->nick, s_Netinfo, "Error, Can not find the virus Helper %s", tmp);
					return -1;
				}			
			} else {
				privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
				return 1;
			}	
		} else if (!strcasecmp(tmp, "DCCDENY")) {
			vlevel = 0;
			for (i = 0; i < V_TABLE_SIZE; i++ ) {
				for (helper = helperlist[i]; helper; helper = helper->next) {
					if (!strcasecmp(helper->nick, u->nick)) {
						vlevel = helper->level;
						break;
					}
				}
			}
			if (vlevel < 50) {
				privmsg(u->nick, s_Netinfo, "Access Denied");
				return 1;
			}
			if (u->Umode & UMODE_REGNICK) {
				tmp2 = strtok(NULL, " ");
				if (!tmp2) {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax. Either ADD/DEL");
					return 1;
				} else if (!strcasecmp(tmp2, "ADD")) {					
					chan = strtok(NULL, " ");
					if (!chan) {
						privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You must specify a file mask");
						return 1;		
					}		
					message = strtok(NULL, "");
					if (!message) {
						privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You must Specify a Message");
					}	
					dccdeny = new_dcc(chan);
					strcpy(dccdeny->who, u->nick);
					strcpy(dccdeny->message, message); 
					sts(":%s SVSFLINE + %s :%s", me.name, dccdeny->file, dccdeny->message);
					privmsg(u->nick, s_Netinfo, "Added DCC Deny for file %s, with message %s", chan, message);
					return 1;
				} else if (!strcasecmp(tmp2, "DEL")) {
					tmp = strtok(NULL, " ");
					if (!tmp) {
						privmsg(u->nick, s_Netinfo, "Incorrect Syntax. You Must Specify a mask", s_Netinfo);
					} else {
						for (i = 0; i < I_TABLE_SIZE; i++) {
							for (dcc = dcclist[i]; dcc; dcc = dcc->next) {
								if (!strcasecmp(tmp, dcc->file)) {
									del_dccdeny_from_hash_table(tmp, dcc);
									privmsg(u->nick, s_Netinfo, "Deleted FileMask  %s", tmp);
										return 1;
								}
							}
						}
						privmsg(u->nick, s_Netinfo, "Error, Can not find the DCC file %s", tmp);
						return -1;
					}			
				} else {
					privmsg(u->nick, s_Netinfo, "Incorrect Syntax");
					return 1;
				}	
			} else {
				privmsg(u->nick, s_Netinfo, "Access Denied");
				return -1;
			}	
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
				
	notice(s_Netinfo, "%s Sent me this: %s", u->nick, coreLine);
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

int Online(Server *data) {

	if (init_bot(s_Netinfo,"Service",me.name,"Network Information Service", "+SAd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_Netinfo = strcat(s_Netinfo, "_");
		init_bot(s_Netinfo,"Service",me.name,"Network Information Service", "+SAd", my_info[0].module_name);
	}
	return 1;
};


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
	{ "SIGNON", 	s_sign_on},
	{ "UMODE", 	s_mode},
	{ "NEWSERVER",  s_new_server},
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
	int i;
	NI_Map *m, *mprev;
	NI_Ignore *ig, *iprev;
	NI_Dccdeny *d, *dprev;
	NI_Vhelper *v, *vprev;
	
	for (i = 0; i < V_TABLE_SIZE; i++) {
		v = helperlist[i];
		while (v) {
			
			vprev = v->next;
			free (v);
			v = vprev;
		}
		helperlist[i] = NULL;
	}
	
	for (i = 0; i < DC_TABLE_SIZE; i++) {
		d = dcclist[i];
		while (d) {
			
			dprev = d->next;
			free (d);
			d = dprev;
		}
		dcclist[i] = NULL;
	}

	for (i = 0; i < M_TABLE_SIZE; i++) {
		m = maplist[i];
		while (m) {
			
			mprev = m->next;
			free (m);
			m = mprev;
		}
		maplist[i] = NULL;
	}
	for (i = 0; i < I_TABLE_SIZE; i++) {
		ig = ignorelist[i];
		while (ig) {
			
			iprev = ig->next;
			free (ig);
			ig = iprev;
		}
		ignorelist[i] = NULL;
	}
	bzero((char *)maplist, sizeof(maplist));
	bzero((char *)ignorelist, sizeof(ignorelist));
	bzero((char *)dcclist, sizeof(dcclist));
	bzero((char *)helperlist, sizeof(helperlist));

	s_Netinfo = "NetInfo";
	
	Loadconfig();


	sts(":%s GLOBOPS :Netinfo Module Loaded", me.name);
}


void _fini() {
	Saveconfig();
	sts(":%s GLOBOPS :NetInfo Module Unloaded",me.name);

};
void Saveconfig()
{
	FILE *fp = fopen("data/niconfig.db", "w");
	NI_Map *map;
	NI_Ignore *ignore;
	NI_Vhelper *helper;
	NI_Dccdeny *dcc;
	int i;

	if (!fp) {
		log("Unable to open niconfig.db for writing.");
		return;
	}

	for (i = 0; i < M_TABLE_SIZE; i++) {
		for (map = maplist[i]; map; map = map->next) {
#ifdef DEBUG
			log("Writing netinfo database for %s", map->tld);
#endif
			fprintf(fp, "%s %s %s\n", map->tld, map->channel, map->Message);
		}
	}
	fprintf(fp, "DEFAULT %s %s\n", Nchannel, Message);
	fprintf(fp, "OPER %s\n", Ochannel);
	fprintf(fp, "%s", "#IGNORE\n");
	for (i = 0; i < I_TABLE_SIZE; i++) {
		for (ignore = ignorelist[i]; ignore; ignore = ignore->next) {
			fprintf(fp, "%s %s\n",ignore->nick, ignore->host);
#ifdef DEBUG
			log("Writing netinfo database for %s", ignore->nick);
#endif
		}
	}
	fprintf(fp, "%s", "#VHELPER\n");
	for (i = 0; i < V_TABLE_SIZE; i++) {
		for (helper = helperlist[i]; helper; helper = helper->next) {
			fprintf(fp, "%s %d\n",helper->nick, helper->level);
#ifdef DEBUG
			log("Writing netinfo database for %s", helper->nick);
#endif
		}
	}
	fprintf(fp, "%s", "#DCCDENY\n");
	for (i = 0; i < DC_TABLE_SIZE; i++) {
		for (dcc = dcclist[i]; dcc; dcc = dcc->next) {
			fprintf(fp, "%s %s %s\n",dcc->file, dcc->who, dcc->message);
#ifdef DEBUG
			log("Writing netinfo database for %s", dcc->file);
#endif
		}
	}
	fclose(fp);
}
void Loadconfig()
{
	FILE *fp = fopen("data/niconfig.db", "r");
	NI_Map *map;
	NI_Ignore *ignore;
	NI_Vhelper *helper;
	NI_Dccdeny *dcc;
	char buf[BUFSIZE];
	char *tmp;

	if (fp) {
		/* this is the Map */
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			log("Buf: %s", buf);
			if (!strcasecmp(buf, "#IGNORE")) { 
				break;
			}
			tmp = strtok(buf, " ");
			if (!strcasecmp(tmp, "DEFAULT")) {
				memcpy(Nchannel, strtok(NULL, " "), 25);
				memcpy(Message, strtok(NULL, ""), 255);
			} else if (!strcasecmp(tmp, "OPER")) {
				memcpy(Ochannel, strtok(NULL, " "), 25);
			} else {
				map = new_map(tmp);
				strcpy(map->channel, strtok(NULL, " "));
				strcpy(map->Message, strtok(NULL, ""));
			}			
#ifdef DEBUG
		log("Got Channel Mapping for %s", map->channel);
#endif
		}
		/* this is the ignore Map */
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			if (!strcasecmp(buf, "#VHELPER")) { 
				break;
			}
#ifdef DEBUG
			log("Buf: %s", buf);
#endif
			ignore = new_ignore(strtok(buf, " "));
			strcpy(ignore->host,strtok(NULL, " "));		
#ifdef DEBUG
			log("Got ignore Mapping for %s", ignore->host);
#endif
		}
		/* this is the vhelper stuff */
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			if (!strcasecmp(buf, "#DCCDENY")) { 
				break;
			}
#ifdef DEBUG
			log("Buf: %s", buf);
#endif
			helper = new_helper(strtok(buf, " "));
			helper->level = atoi(strtok(NULL, " "));
#ifdef DEBUG
			log("Got Virus Helper for %s", helper->nick);
#endif
		}
		/* this is the DCCDeny stuff */
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
#ifdef DEBUG
			log("Buf: %s", buf);
#endif
			dcc = new_dcc(strtok(buf, " "));
			strcpy(dcc->who,strtok(NULL, " "));		
			strcpy(dcc->message, strtok(NULL, ""));
#ifdef DEBUG
			log("Got DCC deny list for %s", dcc->file);
#endif
		}
		fclose(fp);
	} else {
		notice(s_Services, "No Database Found! AutoJoin is Disabled!");
	}
}
