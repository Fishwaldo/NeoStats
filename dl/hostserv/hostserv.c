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
** $Id: hostserv.c,v 1.25 2003/01/04 04:48:26 shmad Exp $
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"
#include "hs_help.c"



/* hostserv doesn't work on Hybrid, Echo a error and exit the compile */
#ifdef HYBRID7
#error "Error: Hybrid7 doesn't support changing a users host. This module will not compile"
#endif



const char hsversion_date[] = __DATE__;
const char hsversion_time[] = __TIME__;
char *s_HostServ;
typedef struct hs_map_ HS_Map;
struct hs_map_ {
    HS_Map *next, *prev;
    char nnick[30];
    char host[MAXHOST];
    char vhost[MAXHOST];
    char passwd[30];
};
HS_Map *nnickmap;

char nnick[255];

extern const char *hs_help[];
static int hs_sign_on(char **av, int ac);

static void hs_add(User *u, char *cmd, char *m, char *h, char *p);
static void hs_list(User *u);
static void hs_view(User *u, int tmpint);
static void hs_del(User *u, int tmpint);
static void hs_login(User *u, char *login, char *pass);
/* static void hs_chpass(User *u, char *login, char *pass, char *newpass); */
static int new_m_version(char *origin, char **av, int ac);

void hslog(char *, ...);
void hsdat(char *, ...);
void hshsamend(char *, ...);
void Loadhosts();

int data_synch;
int load_synch;

char **LoadArry;
char **DelArry;
char **ListArry;
int LoadArryCount = 0;
int DelArryCount = 0;
int ListArryCount = 0;

Module_Info HostServ_info[] = { {
    "HostServ",
    "Network User Virtual Host Service",
    "2.2"
} };


int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module HostServ Loaded, Version: %s %s %s",HostServ_info[0].module_version,hsversion_date,hsversion_time);
    return 0;
}


/* Routine For Setting the Virtual Host */
static int hs_sign_on(char **av, int ac) {
    char *tmp = NULL;
    char *tmp2 = NULL;
        HS_Map *map;

        User *u;
        u = finduser(av[0]);
        if (!u) return 1;

        if (u->server->name == me.name) return 1;

        strcpy(segv_location, "HostServ-hs_signon");

    if (findbot(u->nick)) return 1;
    if (!load_synch) return 1;

    /* Check HostName Against Data Contained in vhosts.data */        

      for (map = nnickmap; map; map = map->next) {
	   if (!strcasecmp(map->nnick, u->nick)) {
          tmp = strlower(map->host);
	  tmp2 = strlower(u->hostname);
		  if (fnmatch(tmp, tmp2, 0) == 0) {
              ssvshost_cmd(u->nick, map->vhost);
              return 1;
           }
       }
    }
return 1;
}

Functions HostServ_fn_list[] = { 
    { MSG_VERSION,  new_m_version,  1 },
#ifdef HAVE_TOKEN_SUP
    { TOK_VERSION,  new_m_version,  1 },
#endif
    { NULL,        NULL,     0}
};

int __Bot_Message(char *origin, char **av, int ac)
{
    int t = 0;
    User *u;
    u = finduser(origin);

    if (!strcasecmp(av[1], "HELP")) {
        if (ac <= 2) {
            privmsg_list(u->nick, s_HostServ, hs_help); 
            return 1;
        } else if (!strcasecmp(av[2], "ADD") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_add);
            return 1;
        } else if (!strcasecmp(av[2], "DEL") && (UserLevel(u) >= 100)) {
            privmsg_list(u->nick, s_HostServ, hs_help_del);
            return 1;        
        } else if (!strcasecmp(av[2], "LIST") && (UserLevel(u) >= 40)) {
            privmsg_list(u->nick, s_HostServ, hs_help_list);
            return 1;
	} else if (!strcasecmp(av[2], "VIEW") && (UserLevel(u) >= 40)) {
	    privmsg_list(u->nick, s_HostServ, hs_help_view);
	    return 1;
	} else if (!strcasecmp(av[2], "LOGIN")) {
	    privmsg_list(u->nick, s_HostServ, hs_help_login);
	    return 1;
        } else if (!strcasecmp(av[2], "ABOUT")) {
            privmsg_list(u->nick, s_HostServ, hs_help_about);
            return 1;
        } else 
            prefmsg(u->nick, s_HostServ, "Unknown Help Topic: \2%s\2", av[2]);
        }

    if (!strcasecmp(av[1], "ABOUT")) {
                privmsg_list(u->nick, s_HostServ, hs_help_about);
    } else if (!strcasecmp(av[1], "ADD") && (UserLevel(u) >= 40)) {
                if (ac < 6) {
                    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME> <PASSWORD>", s_HostServ);
                    prefmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
                    return -1;
                }
                hs_add(u, av[2], av[3], av[4], av[5]);
    } else if (!strcasecmp(av[1], "DEL") && (UserLevel(u) >= 100)) {
                if (!av[2]) {
                    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL #", s_HostServ);
                    prefmsg(u->nick, s_HostServ, "The users # is got from /msg %s LIST", s_HostServ);
                    return -1;
                }
                t = atoi(av[2]);
                if (!t) {
                    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s DEL #", s_HostServ);
                    prefmsg(u->nick, s_HostServ, "The users # is got from /msg %s LIST", s_HostServ);
                    return -1;
                }
                hs_del(u, t);
    } else if (!strcasecmp(av[1], "LIST") && (UserLevel(u) >= 40)) {
                hs_list(u);
    } else if (!strcasecmp(av[1], "VIEW") && (UserLevel(u) >= 40)) {
		if (!av[2]) {
		    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s VIEW #", s_HostServ);
		    prefmsg(u->nick, s_HostServ, "The users # is got from /msg %s LIST", s_HostServ);
		    return -1;
		}
		t = atoi(av[2]);
                if (!t) {
                    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s VIEW #", s_HostServ);
                    prefmsg(u->nick, s_HostServ, "The users # is got from /msg %s LIST", s_HostServ);
                    return -1;
                }
		hs_view(u, t);
    } else if (!strcasecmp(av[1], "LOGIN")) {
                if (!av[3]) {
                    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s LOGIN <NICK> <PASSWORD>", s_HostServ);
                    prefmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP LOGIN", s_HostServ);
                    return -1;
                }
                hs_login(u, av[2], av[3]);
    } else {
        prefmsg(u->nick, s_HostServ, "Unknown Command: \2%s\2, perhaps you need some HELP?", av[1]);
    }
    return 1;

}

int Online(char **av, int ac) {
    if (init_bot(s_HostServ,"HostServ",me.name,"Network User Virtual Host Service", "+oikSwgleq-x", HostServ_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_HostServ = strcat(s_HostServ, "_");
        init_bot(s_HostServ,"HostServ",me.name,"Network User Virtual Host Service", "+oikSwgleq-x", HostServ_info[0].module_name);
    }
    return 1;
};

EventFnList HostServ_Event_list[] = {
    { "ONLINE", Online},
    { "SIGNON", hs_sign_on},
    { NULL, NULL}
};

Module_Info *__module_get_info() {
    return HostServ_info;
};

Functions *__module_get_functions() {
    return HostServ_fn_list;
};

EventFnList *__module_get_events() {
    return HostServ_Event_list;
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
        hslog("Unable to open data/vhosts.new for writing.");
        return;
        }

        fprintf(hsamend, "%s\n", buf);
        va_end(ap);
        fclose(hsamend);

}


/* Routine for ADD */
static void hs_add(User *u, char *cmd, char *m, char *h, char *p) {

    char *tmp;
    strcpy(segv_location, "hs_add");
    hsdat(":%s %s %s %s", cmd, m, h, p);
    hslog("%s added a vhost for %s with realhost %s vhost %s and password %s",u->nick, cmd, m, h, p);
    prefmsg(u->nick, s_HostServ, "%s has sucessfuly been registered under realhost: %s vhost: %s and password: %s",cmd, m, h, p);
    /* Apply The New Hostname If The User Is Online */        
    if (finduser(cmd)) {
          u = finduser(cmd);
          if (findbot(u->nick)) return;
          tmp = strlower(u->hostname);
	  if (fnmatch(m, tmp, 0) == 0) {
              ssvshost_cmd(u->nick, h);
	    Loadhosts();
              return;
          }
    }
    Loadhosts();
}


/* Routine for 'HostServ' to print out its data */
static void hs_list(User *u)
{
    FILE *fp;
    char buf[512];
    int i;

    strcpy(segv_location, "hs_list");
    if (!(UserLevel(u) >= 40)) {
        hslog("Access Denied (LIST) to %s", u->nick);
        prefmsg(u->nick, s_HostServ, "Access Denied.");
        return;
    }

    fp = fopen("data/vhosts.db", "r");
    if (!fp) {
        prefmsg(u->nick, s_HostServ, "Unable to open data/vhosts.db");
        return;
    }
    i = 1;
    prefmsg(u->nick, s_HostServ, "Current HostServ VHOST list:");
    prefmsg(u->nick, s_HostServ, "%-5s %-12s %-30s","Num", "Nick", "Vhost");
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';

        ListArryCount = split_buf(buf, &ListArry, 0);
/*        prefmsg(u->nick, s_HostServ, "%-5d %-12s %-19s %-8s", i, ListArry[0], ListArry[1], ListArry[2]); */
        prefmsg(u->nick, s_HostServ, "%-5d %-12s %-30s", i, ListArry[0], ListArry[2]);
	i++;
    }
    fclose(fp);
    prefmsg(u->nick, s_HostServ, "For more information on someone use /msg %s VIEW #", s_HostServ);
    prefmsg(u->nick, s_HostServ, "--- End of List ---");
}

/* Routine for VIEW */
static void hs_view(User *u, int tmpint)
{
    FILE *fp;
    char buf[512];
    int i;
    strcpy(segv_location, "hs_view");
    if (!(UserLevel(u) >= 40)) {
        hslog("Access Denied (LIST) to %s", u->nick);
        prefmsg(u->nick, s_HostServ, "Access Denied.");
        return;
    }

    fp = fopen("data/vhosts.db", "r");
    if (!fp) {
        prefmsg(u->nick, s_HostServ, "Unable to open data/vhosts.db");
        return;
    }
    i = 1;
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';

        ListArryCount = split_buf(buf, &ListArry, 0);
        if (tmpint == i) {
	    prefmsg(u->nick, s_HostServ, "Virtual Host information:");
            prefmsg(u->nick, s_HostServ, "Nick:     %s", ListArry[0]);
	    prefmsg(u->nick, s_HostServ, "RealHost: %s", ListArry[1]);
	    prefmsg(u->nick, s_HostServ, "V-host:   %s", ListArry[2]);
	    prefmsg(u->nick, s_HostServ, "Password: %s", ListArry[3]);
	    prefmsg(u->nick, s_HostServ, "--- End of information for %s ---",ListArry[0]);
        }
	i++;
    }
    fclose(fp);
    if (tmpint > i) prefmsg(u->nick, s_HostServ, "ERROR: There is no vhost on list number \2%d\2",tmpint);
}




/* Routine For Loading The Hosts into Array */
void Loadhosts()
{
    FILE *fp = fopen("data/vhosts.db", "r");
    HS_Map *map;
    char buf[BUFSIZE];

    if (fp) {
        load_synch = 1;
		while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);
            map = malloc(sizeof(HS_Map));

            LoadArryCount = split_buf(buf, &LoadArry, 0);
            strcpy(map->nnick, LoadArry[0]);
            strcpy(map->host, LoadArry[1]);
            strcpy(map->vhost, LoadArry[2]);
	    strcpy(map->passwd, LoadArry[3]);
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
      if (!data_synch) {
/*          notice(s_Services, "data/vhosts.db Database Not Found! Either Add a User or Disable This Function"); */
          data_synch = 1;
	  }
   }
}


/* Routine for HostServ to delete the given information */
static void hs_del(User *u, int tmpint)
{

    FILE *fp = fopen("data/vhosts.db", "r");
    char buf[BUFSIZE];
    int i = 1;

    strcpy(segv_location, "hs_del");
    if (!(UserLevel(u) >= 100)) {
        hslog("Access Denied To %s To User on Access list #%s", u->nick, tmpint);
        prefmsg(u->nick, s_HostServ, "Access Denied To %s To Delete User \2#%s\2", u->nick, tmpint);
        return;
    }

    if (!fp) {
        prefmsg(u->nick, s_HostServ, "Unable to open data/vhosts.db");
        return;
    }

    if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);

	      if (i != tmpint) {
		hsamend("%s", buf);
	    }


	      if (i == tmpint) {
		prefmsg(u->nick, s_HostServ, "The following line was removed from the Vhosts Database");
	        prefmsg(u->nick, s_HostServ, "\2%s\2", buf);
		hslog("%s removed the VHOST: %s", u->nick, buf);
	    }
	i++;
    }
     fclose(fp);
    remove("data/vhosts.db");
    rename("data/vhosts.new", "data/vhosts.db");    
    Loadhosts();
    } 

    if (tmpint > i) prefmsg(u->nick, s_HostServ, "ERROR: There is no vhost on list number \2%d\2",tmpint);
        return;
}


/* Routine to allow users to login and get their vhosts */
static void hs_login(User *u, char *login, char *pass)
{
        HS_Map *map;

        strcpy(segv_location, "HostServ-hs_login");

    /* Check HostName Against Data Contained in vhosts.data */
      for (map = nnickmap; map; map = map->next) {
           if (!strcasecmp(map->nnick, login)) {
	      if (!strcasecmp(map->passwd, pass)) {
	              ssvshost_cmd(u->nick, map->vhost);
		      prefmsg(u->nick, s_HostServ, "Your VHOST %s has been set.", map->vhost);
		      hslog("%s used LOGIN to obtain userhost of %s",u->nick, map->vhost);
	              return;
           }
       }
    }
	prefmsg(u->nick, s_HostServ, "Incorrect Login or Password.  Do you have a vhost added?");
        return;
}

