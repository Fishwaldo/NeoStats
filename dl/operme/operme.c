/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module: OperMe!
** Description: A network wide OPERing service
** Version: 0.01
** Authors: Shmad
** Sidenote: Some code shamelessly borrowed from HostServ :P~
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"
#include "om_help.c"

const char omversion_date[] = __DATE__;
const char omversion_time[] = __TIME__;
char *s_OperMe;
typedef struct om_map_ OM_Map;
struct om_map_ {
    OM_Map *next, *prev;
    char nnick[30];
    char host[MAXHOST];
    char pass[30];
	char flags[40];
};
OM_Map *nnickmap;

char nnick[255];

extern const char *om_help[];
static int om_operme(User *u, char *cmd);

static void om_add(User *u, char *cmd, char *m, char *h, char *f);
static void om_list(User *u);
static void om_del(User *u, char *cmd, char *m, char *h, char *f);

void omlog(char *, ...);
void omdat(char *, ...);
void omomamend(char *, ...);
void Loadopers();

char **LoadArry;
char **DelArry;
char **ListArry;
int LoadArryCount = 0;
int DelArryCount = 0;
int ListArryCount = 0;

Module_Info my_info[] = { {
    "OperMe",
    "A Network-wide OPERing Service",
    "0.01"
} };

int new_m_version(char *av, char *tmp) {
    snumeric_cmd(351, av, "Module OperMe! Loaded, Version: %s %s %s",my_info[0].module_version,omversion_date,omversion_time);
    return 0;
}


/* Routine For OPERing up */
static int om_operme(User *u, char *cmd) {
    char *tmp = NULL;
	OM_Map *map;

    Loadopers();

    if (findbot(u->nick)) return 1;

    /* Check HostName Against Data Contained in operme.data */        

/* THIS PROC TO CHANGE */

	  for (map = nnickmap; map; map = map->next) {
	   if (!strcasecmp(map->nnick, u->nick)) {
          if (!strcasecmp(map->pass, cmd)) {
 		     tmp = map->host;
		     if (fnmatch(strlower(tmp), strlower(u->hostname), 0) == 0) {
                /* ssvsmode_cmd(u->nick, map->flags); */
            privmsg(u->nick, s_OperMe, "Logged in.");

                 return 1;
			  }
		   }
       }
    }


return 1;
}


Functions my_fn_list[] = { 
    { "VERSION",    new_m_version,    1 },
    { NULL,        NULL,     0}
};

int __Bot_Message(char *origin, char **av, int ac)
{
    User *u;
    u = finduser(origin);

    if (!strcasecmp(av[1], "HELP")) {
        if (ac <= 2 && (!(UserLevel(u) > 40))) {
            privmsg(u->nick, s_OperMe, "Permission Denied.");
            return 1;
        } else if (ac <= 2 && (UserLevel(u) <= 40)) {
            privmsg_list(u->nick, s_OperMe, om_help); 
            return 1;
        } else if (!strcasecmp(av[2], "ADD") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_OperMe, om_help_add);
            return 1;
        } else if (!strcasecmp(av[2], "DEL") && (UserLevel(u) >= 60)) {
            privmsg_list(u->nick, s_OperMe, om_help_del);
            return 1;        
        } else if (!strcasecmp(av[2], "LIST") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_OperMe, om_help_list);
            return 1;
        } else if (!strcasecmp(av[2], "ABOUT") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_OperMe, om_help_about);
            return 1;
		} else if (!strcasecmp(av[2], "LOGIN")) {
			privmsg_list(u->nick, s_OperMe, om_help_login);
			return 1;
		} else 
            privmsg(u->nick, s_OperMe, "Unknown Help Topic: \2%s\2", av[2]);
        }

    if (!strcasecmp(av[1], "ABOUT")) {
                privmsg_list(u->nick, s_OperMe, om_help_about);
    } else if (!strcasecmp(av[1], "ADD") && (UserLevel(u) >= 40)) {
                if (ac < 5) {
                    privmsg(u->nick, s_OperMe, "Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_OperMe);
                    privmsg(u->nick, s_OperMe, "For addtional help: /msg %s HELP", s_OperMe);
                    return -1;
                }
                om_add(u, av[2], av[3], av[4], av[5]);
    } else if (!strcasecmp(av[1], "DEL") && (UserLevel(u) >= 60)) {
                if (ac < 5) {
                    privmsg(u->nick, s_OperMe, "Syntax: /msg %s DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_OperMe);
                    privmsg(u->nick, s_OperMe, "For addtional help: /msg %s HELP", s_OperMe);
                    return -1;
                }
                om_del(u, av[2], av[3], av[4], av[5]);    
    } else if (!strcasecmp(av[1], "LIST") && (UserLevel(u) >= 40)) {
                om_list(u);
	} else if (!strcasecmp(av[1], "LOGIN")) {
				om_operme(u, av[2]);
	} else {
        privmsg(u->nick, s_OperMe, "Unknown Command: \2%s\2, perhaps you need some HELP?", av[1]);
    }
    return 1;

}

int Online(Server *data) {

    if (init_bot(s_OperMe,"OperMe",me.name,"Network-wide OPERing Service", "+oikSdwgleq-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_OperMe = strcat(s_OperMe, "_");
        init_bot(s_OperMe,"OperMe",me.name,"Network-wide OPERing Service", "+oikSdwgleq-x", my_info[0].module_name);
    }
    return 1;
};

EventFnList my_event_list[] = {
    { "ONLINE", Online},
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
    s_OperMe = "OperMe";
    globops(me.name, "OperMe! Module Loaded",me.name);
    Loadopers();
}

void _fini() {
    globops(me.name, "OperMe! Module Unloaded",me.name);

};


/* Routine for logging items with the 'omlog' */
void omlog(char *fmt, ...)
{
        va_list ap;
        FILE *omfile = fopen("logs/OperMe.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!omfile) {
        log("Unable to open logs/OperMe.log for writing.");
        return;
        }

       fprintf(omfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(omfile);

}


/* Routine for registrations with the 'operme.db' file */
void omdat(char *fmt, ...)
{
        va_list ap;
        FILE *omfile = fopen("data/operme.db", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!omfile) {
        log("Unable to open data/operme.db for writing.");
        return;
        }

        fprintf(omfile, "%s\n", buf);
        va_end(ap);
        fclose(omfile);

}


/* Routine for creating the 'operme.new' file */
void omamend(char *fmt, ...)
{
        va_list ap;
        FILE *omamend = fopen("data/operme.new", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!omamend) {
        log("Unable to open data/operme.new for writing.");
        return;
        }

        fprintf(omamend, "%s\n", buf);
        va_end(ap);
        fclose(omamend);

}


/* Routine for ADD */

static void om_add(User *u, char *cmd, char *m, char *h, char *f) {

    strcpy(segv_location, "om_add");
    omdat(":%s %s %s %s", cmd, m, h, f);
    privmsg(u->nick, s_OperMe, "%s has sucessfuly been registered under hostname: %s password: %s and flags: %s",cmd, m, h, f);

}


/* Routine for 'OperMe' to print out its data */
static void om_list(User *u)
{
    FILE *fp;
    char buf[512];

    strcpy(segv_location, "om_list");
    if (!(UserLevel(u) >= 40)) {
        omlog("Access Denied (LIST) to %s", u->nick);
        privmsg(u->nick, s_OperMe, "Access Denied.");
        return;
    }

    fp = fopen("data/operme.db", "r");
    if (!fp) {
        privmsg(u->nick, s_OperMe, "Unable to open data/operme.db");
        return;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';

        ListArryCount = split_buf(buf, &ListArry, 0);
		if (UserLevel(u) > 180) {
			privmsg(u->nick, s_OperMe, "%s %s PASSWORD_NOT_DISPLAYED %s", ListArry[0], ListArry[1], ListArry[3]);
		} else {
            privmsg(u->nick, s_OperMe, "%s %s %s %s", ListArry[0], ListArry[1], ListArry[2], ListArry[3]);
		}
	}
    fclose(fp);
}


/* Routine For Loading The OperMe! O-Lines into Array */
void Loadopers()
{
    FILE *fp = fopen("data/operme.db", "r");
    OM_Map *map;
    char buf[BUFSIZE];

    if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);
            map = smalloc(sizeof(OM_Map));

            LoadArryCount = split_buf(buf, &LoadArry, 0);
            strcpy(map->nnick, LoadArry[0]);
            strcpy(map->host, LoadArry[1]);
			strcpy(map->pass, LoadArry[2]);
            strcpy(map->flags, LoadArry[3]);
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
        notice(s_Services, "data/operme.db Database Not Found! Either Add a User or Disable This Function");
    }
}


/* Routine for OperMe! to delete the given information */
static void om_del(User *u, char *cmd, char *m, char *h, char *f)
{

    FILE *fp = fopen("data/operme.db", "r");
    FILE *omtmp;
    char buf[BUFSIZE];
    char dat[512];
    int del_verify = 0;
    char *compare = NULL;

    strcpy(segv_location, "om_del");
    if (!(UserLevel(u) >= 60)) {
        omlog("Access Denied To %s To Delete User: %s", u->nick, cmd);
        privmsg(u->nick, s_OperMe, "Access Denied To %s To Delete User: %s", u->nick, cmd);
        return;
    }

    omtmp = fopen("data/OperMe.tmp","w");
    fprintf(omtmp, ":%s %s %s %s", cmd, m, h, f);
    fclose(omtmp);
                        
    omtmp = fopen("data/OperMe.tmp", "r");
    while (fgets(dat, sizeof(dat), omtmp)) {
        buf[strlen(dat)] = '\0';
        compare = dat;
    }
    fclose(omtmp);
    remove("data/OperMe.tmp");

    if (!fp) {
        privmsg(u->nick, s_OperMe, "Unable to open data/operme.db");
        return;
    }

    if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);

            if (fnmatch(buf, compare, 0) != 0) {
                omamend("%s", buf);
            }

            if (fnmatch(buf, compare, 0) == 0) {
                privmsg(u->nick, s_OperMe, "%s (%s) Was Removed From The OperMe! Database", cmd, h);
                del_verify=1;
            } 

    }
     fclose(fp);
    remove("data/operme.db");
    rename("data/operme.new", "data/operme.db");    
    } 

    if (!del_verify) {
        privmsg(u->nick, s_OperMe, "Sorry, The User '%s' With A Host Of '%s' was unable to be removed from the OperMe! database.", cmd, h);
        return;
    }

}
