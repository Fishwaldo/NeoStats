/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module: HostServ
** Description: Network User Virtual Host Service
** Version: 1.5
** Authors: ^Enigma^ & Shmad
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"
#include "hs_help.c"

const char hsversion_date[] = __DATE__;
const char hsversion_time[] = __TIME__;
char *s_HostServ;
typedef struct hs_map_ HS_Map;
struct hs_map_ {
	HS_Map *next, *prev;
    char nnick[30];
	char host[MAXHOST];
	char vhost[MAXHOST];
};
HS_Map *nnickmap;

char nnick[255];

extern const char *hs_help[];
static int hs_sign_on(User *);

static void hs_add(User *u, char *cmd, char *m, char *h);
static void hs_list(User *u);
static void hs_del(User *u, char *cmd);

void hslog(char *, ...);
void hsdat(char *, ...);
void hshsamend(char *, ...);
void Loadhosts();


Module_Info my_info[] = { {
    "HostServ",
    "Network User Virtual Host Service",
    "1.5"
} };

int new_m_version(char *av, char *tmp) {
    sts(":%s 351 %s :Module HostServ Loaded, Version: %s %s %s",me.name,av,my_info[0].module_version,hsversion_date,hsversion_time);
    return 0;
}



/* Routine For Setting the Virtual Host */
static int hs_sign_on(User *u) {
	HS_Map *map;

    Loadhosts();

	if (findbot(u->nick)) return 1;
	/* Check HostName Against Data Contained in vhosts.data */		
      for (map = nnickmap; map; map = map->next) {
	   if (fnmatch(strlower(map->nnick), strlower(u->nick), 0) == 0) {
		  if (fnmatch(map->host, strlower(u->hostname), 0) == 0) {
              sts(":%s CHGHOST %s %s", me.name, u->nick, map->vhost);
              return 1;
           }
       }
    }
return 1;
}


Functions my_fn_list[] = { 
    { "VERSION",    new_m_version,    1 },
    { NULL,        NULL,     0}
};

int __Bot_Message(char *origin, char *coreLine, int type)
{
    User *u;
    char *cmd;
	u = finduser(origin);

    if (coreLine == NULL) return -1;
    cmd = strtok(coreLine, " ");
    if (!strcasecmp(cmd, "HELP")) {
        coreLine = strtok(NULL, " ");
        if (!coreLine && (!(UserLevel(u) >= 40))) {
            privmsg(u->nick, s_HostServ, "Permission Denied.");
            return 1;
        } else if (!coreLine && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help); 
            return 1;
        } else if (!strcasecmp(coreLine, "ADD") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_add);
            return 1;
        } else if (!strcasecmp(coreLine, "DEL") && (UserLevel(u) >= 60)) {
            privmsg_list(u->nick, s_HostServ, hs_help_del);
            return 1;		
		} else if (!strcasecmp(coreLine, "LIST") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_list);
            return 1;
        } else if (!strcasecmp(coreLine, "ABOUT") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_about);
            return 1;
        } else 
            privmsg(u->nick, s_HostServ, "Unknown Help Topic: \2%s\2",coreLine);
        }

    if (!strcasecmp(cmd, "ABOUT")) {
                privmsg_list(u->nick, s_HostServ, hs_help_about);
    } else if (!strcasecmp(cmd, "ADD") && (UserLevel(u) >= 40)) {
				char *m, *h;
				cmd = strtok(NULL, " ");
				m = strtok(NULL, " ");
				h = strtok(NULL, "");
				hs_add(u, cmd, m, h);
    } else if (!strcasecmp(cmd, "DEL") && (UserLevel(u) >= 60)) {
				cmd = strtok(NULL, "");
				hs_del(u, cmd);				
	} else if (!strcasecmp(cmd, "LIST") && (UserLevel(u) >= 40)) {
                hs_list(u);
	} else {
        privmsg(u->nick, s_HostServ, "Unknown Command: \2%s\2, perhaps you need some HELP?",cmd);
    }
    return 1;

}

int Online(Server *data) {

    if (init_bot(s_HostServ,"HostServ",me.name,"Network User Virtual Host Service", "+oikSdwgleq-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_HostServ = strcat(s_HostServ, "_");
        init_bot(s_HostServ,"HostServ",me.name,"Network User Virtual Host Service", "+oikSdwgleq-x", my_info[0].module_name);
    }
    return 1;
};

EventFnList my_event_list[] = {
    { "ONLINE", Online},
	{ "SIGNON", hs_sign_on},
    { NULL, NULL}
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
    s_HostServ = "HostServ";
    sts(":%s GLOBOPS :HostServ Module Loaded",me.name);
	Loadhosts();
}

void _fini() {
    sts(":%s GLOBOPS :HostServ Module Unloaded",me.name);

};


/* Routine for logging items with the 'hslog' */
void hslog(char *fmt, ...)
{
        va_list ap;
        FILE *hsfile = fopen("logs/HostServ.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!hsfile) {
        log("Unable to open logs/HostServ.log for writing.");
        return;
        }

    fprintf(hsfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(hsfile);

}


/* Routine for registrations with the 'vhosts.db' file */
void hsdat(char *fmt, ...)
{
        va_list ap;
        FILE *hsfile = fopen("data/vhosts.db", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!hsfile) {
        log("Unable to open data/vhosts.db for writing.");
        return;
    }

    fprintf(hsfile, "%s\n", buf);
        va_end(ap);
        fclose(hsfile);

}


/* Routine for creating the 'vhosts.new' file */
void hsamend(char *fmt, ...)
{
        va_list ap;
        FILE *hsamend = fopen("data/vhosts.new", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!hsamend) {
        log("Unable to open data/vhosts.new for writing.");
        return;
    }

    fprintf(hsamend, "%s\n", buf);
        va_end(ap);
        fclose(hsamend);

}


/* Routine for ADD */
static void hs_add(User *u, char *cmd, char *m, char *h) {

	if (!cmd) {
            privmsg(u->nick, s_HostServ, "Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
            privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
            return;
        }

	if (!h) {
            privmsg(u->nick, s_HostServ, "Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
            privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
            return;
        } 
    if (!m) {
            privmsg(u->nick, s_HostServ, "Syntax: /msg %s ADD ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
            privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
            return;
   }
   /* The user has passed the minimum requirements for input */
    
    segv_location = sstrdup("hs_add");
	hsdat("%s %s %s", cmd, m, h);
    privmsg(u->nick, s_HostServ, "%s has sucessfuly been registered under realhost: %s and vhost: %s",cmd, m, h);

	/* Apply The New Hostname If The User Is Online */		
	if (finduser(cmd)) {
          u = finduser(cmd);
          if (findbot(u->nick)) return;
		  if (fnmatch(m, strlower(u->hostname), 0) == 0) {
              sts(":%s CHGHOST %s %s", me.name, u->nick, h);
              return;
          }
	}
}


/* Routine for 'HostServ' to print out its data */
static void hs_list(User *u)
{
    FILE *fp;
    char buf[512];

    segv_location = sstrdup("hs_list");
    if (!(UserLevel(u) >= 40)) {
        hslog("Access Denied (LIST) to %s", u->nick);
        privmsg(u->nick, s_HostServ, "Access Denied.");
        return;
    }

    fp = fopen("data/vhosts.db", "r");
    if (!fp) {
        privmsg(u->nick, s_HostServ, "Unable to open data/vhosts.db");
        return;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';
        privmsg(u->nick, s_HostServ, "%s", buf);
    }
    fclose(fp);
}


/* Routine For Loading The Hosts into Array */
void Loadhosts()
{
	FILE *fp = fopen("data/vhosts.db", "r");
	HS_Map *map;
	char buf[BUFSIZE];
	char *tmp;

	if (fp) {
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			map = smalloc(sizeof(HS_Map));
			tmp = strtok(buf, " ");

			strcpy(map->nnick, tmp);
			strcpy(map->host, strtok(NULL, " "));
			strcpy(map->vhost, strtok(NULL, ""));
			if (!nnickmap) {
				nnickmap = map;
				nnickmap->next = NULL;
			} else {
				map->next = nnickmap;
				nnickmap = map;
			}
    }
	  fclose(fp);
    } else {
		notice(s_Services, "data/vhosts.db Database Not Found! Either Add a User or Disable This Function");
	}
}


/* Routine for HostServ to delete the given information */
static void hs_del(User *u, char *cmd)
{

    FILE *fp = fopen("data/vhosts.db", "r");
    FILE *hstmp;
    int del_verify = 0;
    char buf[BUFSIZE];
    char dat[512];
    char *ipname, *iphost, *ipvhost;
    char *temp = NULL;

    segv_location = sstrdup("hs_del");
    if (!(UserLevel(u) >= 60)) {
        hslog("Access Denied To %s To Delete User: %s", u->nick, cmd);
        privmsg(u->nick, s_HostServ, "Access Denied To %s To Delete User: %s", u->nick, cmd);
        return;
    }
    if (!cmd) {
        privmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
        privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
        return;
    }

    hstmp = fopen("data/HostServ.tmp","w");
    fprintf(hstmp, "%s", cmd);
    fclose(hstmp);
                        
    hstmp = fopen("data/HostServ.tmp", "r");
    while (fgets(dat, sizeof(dat), hstmp)) {
        buf[strlen(dat)] = '\0';
        temp = dat;
    }
    fclose(hstmp);
    remove("data/HostServ.tmp");

    ipname = strtok(temp, " ");
    iphost = strtok(NULL, " ");
    ipvhost = strtok(NULL, "");

    if (!ipname) {
        privmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
        privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
        return;
    }
	if (!iphost) {
        privmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
        privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
        return;
    }
	if (!ipvhost) {
        privmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
        privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
        return;
    }

    if (!fp) {
        privmsg(u->nick, s_HostServ, "Unable to open data/vhosts.db");
        return;
	}

	if (fp) {
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);

            if (fnmatch(buf, cmd, 0) != 0) {
                hsamend("%s", buf);
            }

            if (fnmatch(buf, cmd, 0) == 0) {
                privmsg(u->nick, s_HostServ, "%s Was Removed From The Vhosts Database", cmd);
                del_verify=1;
			} 

	}
 	fclose(fp);
    remove("data/vhosts.db");
    rename("data/vhosts.new", "data/vhosts.db");	
    } 

    if (!del_verify) {
        privmsg(u->nick, s_HostServ, "Sorry, The User '%s' With A Host Of '%s' Was Unable To Be Removed From The Vhosts Database.", ipname, iphost);
        return;
	}

}
