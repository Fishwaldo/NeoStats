/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module: HostServ
** Description: Network User Virtual Host Service
** Version: 1.6
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
static void hs_del(User *u, char *cmd, char *m, char *h);

void hslog(char *, ...);
void hsdat(char *, ...);
void hshsamend(char *, ...);
void Loadhosts();

char **LoadArry;
char **DelArry;
char **ListArry;
int LoadArryCount = 0;
int DelArryCount = 0;
int ListArryCount = 0;

Module_Info my_info[] = { {
    "HostServ",
    "Network User Virtual Host Service",
    "1.6"
} };

int new_m_version(char *av, char *tmp) {
    snumeric_cmd(351, av, "Module HostServ Loaded, Version: %s %s %s",my_info[0].module_version,hsversion_date,hsversion_time);
    return 0;
}


/* Routine For Setting the Virtual Host */
static int hs_sign_on(User *u) {
    char *tmp = NULL;
	HS_Map *map;

    Loadhosts();

    if (findbot(u->nick)) return 1;

    /* Check HostName Against Data Contained in vhosts.data */        
      for (map = nnickmap; map; map = map->next) {
	   if (!strcasecmp(map->nnick, u->nick)) {
          tmp = map->host;
		  if (fnmatch(strlower(tmp), strlower(u->hostname), 0) == 0) {
              ssvshost_cmd(u->nick, map->vhost);
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

int __Bot_Message(char *origin, char **av, int ac)
{
    User *u;
    u = finduser(origin);

    if (!strcasecmp(av[1], "HELP")) {
        if (ac <= 2 && (!(UserLevel(u) < 40))) {
            privmsg(u->nick, s_HostServ, "Permission Denied.");
            return 1;
        } else if (ac <= 2 && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help); 
            return 1;
        } else if (!strcasecmp(av[2], "ADD") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_add);
            return 1;
        } else if (!strcasecmp(av[2], "DEL") && (UserLevel(u) >= 60)) {
            privmsg_list(u->nick, s_HostServ, hs_help_del);
            return 1;        
        } else if (!strcasecmp(av[2], "LIST") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_list);
            return 1;
        } else if (!strcasecmp(av[2], "ABOUT") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_about);
            return 1;
        } else 
            privmsg(u->nick, s_HostServ, "Unknown Help Topic: \2%s\2", av[2]);
        }

    if (!strcasecmp(av[1], "ABOUT")) {
                privmsg_list(u->nick, s_HostServ, hs_help_about);
    } else if (!strcasecmp(av[1], "ADD") && (UserLevel(u) >= 40)) {
                if (ac < 5) {
                    privmsg(u->nick, s_HostServ, "Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
                    privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
                    return -1;
                }
                hs_add(u, av[2], av[3], av[4]);
    } else if (!strcasecmp(av[1], "DEL") && (UserLevel(u) >= 60)) {
                if (ac < 5) {
                    privmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>", s_HostServ);
                    privmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
                    return -1;
                }
                hs_del(u, av[2], av[3], av[4]);    
    } else if (!strcasecmp(av[1], "LIST") && (UserLevel(u) >= 40)) {
                hs_list(u);
    } else {
        privmsg(u->nick, s_HostServ, "Unknown Command: \2%s\2, perhaps you need some HELP?", av[1]);
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
    globops(me.name, "HostServ Module Loaded",me.name);
    Loadhosts();
}

void _fini() {
    globops(me.name, "HostServ Module Unloaded",me.name);

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

    strcpy(segv_location, "hs_add");
    hsdat(":%s %s %s", cmd, m, h);
    privmsg(u->nick, s_HostServ, "%s has sucessfuly been registered under realhost: %s and vhost: %s",cmd, m, h);

    /* Apply The New Hostname If The User Is Online */        
    if (finduser(cmd)) {
          u = finduser(cmd);
          if (findbot(u->nick)) return;
          if (fnmatch(m, strlower(u->hostname), 0) == 0) {
              ssvshost_cmd(u->nick, h);
              return;
          }
    }
}


/* Routine for 'HostServ' to print out its data */
static void hs_list(User *u)
{
    FILE *fp;
    char buf[512];

    strcpy(segv_location, "hs_list");
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

        ListArryCount = split_buf(buf, &ListArry, 0);
        privmsg(u->nick, s_HostServ, "%s %s %s", ListArry[0], ListArry[1], ListArry[2]);
    }
    fclose(fp);
}


/* Routine For Loading The Hosts into Array */
void Loadhosts()
{
    FILE *fp = fopen("data/vhosts.db", "r");
    HS_Map *map;
    char buf[BUFSIZE];

    if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);
            map = smalloc(sizeof(HS_Map));

            LoadArryCount = split_buf(buf, &LoadArry, 0);
            strcpy(map->nnick, LoadArry[0]);
            strcpy(map->host, LoadArry[1]);
            strcpy(map->vhost, LoadArry[2]);
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
static void hs_del(User *u, char *cmd, char *m, char *h)
{

    FILE *fp = fopen("data/vhosts.db", "r");
    FILE *hstmp;
    char buf[BUFSIZE];
    char dat[512];
    int del_verify = 0;
    char *compare = NULL;

    strcpy(segv_location, "hs_del");
    if (!(UserLevel(u) >= 60)) {
        hslog("Access Denied To %s To Delete User: %s", u->nick, cmd);
        privmsg(u->nick, s_HostServ, "Access Denied To %s To Delete User: %s", u->nick, cmd);
        return;
    }

    hstmp = fopen("data/HostServ.tmp","w");
    fprintf(hstmp, ":%s %s %s", cmd, m, h);
    fclose(hstmp);
                        
    hstmp = fopen("data/HostServ.tmp", "r");
    while (fgets(dat, sizeof(dat), hstmp)) {
        buf[strlen(dat)] = '\0';
        compare = dat;
    }
    fclose(hstmp);
    remove("data/HostServ.tmp");

    if (!fp) {
        privmsg(u->nick, s_HostServ, "Unable to open data/vhosts.db");
        return;
    }

    if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);

            if (fnmatch(buf, compare, 0) != 0) {
                hsamend("%s", buf);
            }

            if (fnmatch(buf, compare, 0) == 0) {
                privmsg(u->nick, s_HostServ, "%s (%s) Was Removed From The Vhosts Database", cmd, h);
                del_verify=1;
            } 

    }
     fclose(fp);
    remove("data/vhosts.db");
    rename("data/vhosts.new", "data/vhosts.db");    
    } 

    if (!del_verify) {
        privmsg(u->nick, s_HostServ, "Sorry, The User '%s' With A Host Of '%s' Was Unable To Be Removed From The Vhosts Database.", cmd, h);
        return;
    }

}