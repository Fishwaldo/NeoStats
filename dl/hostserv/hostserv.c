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
** $Id: hostserv.c,v 1.31 2003/01/07 12:22:33 fishwaldo Exp $
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "dotconf.h"
#include "stats.h"
#include "hs_help.c"



/* hostserv doesn't work on Hybrid, Echo a error and exit the compile */
#ifdef HYBRID7
#error "Error: Hybrid7 doesn't support changing a users host. This module will not compile"
#endif



const char hsversion_date[] = __DATE__;
const char hsversion_time[] = __TIME__;
char *s_HostServ;
typedef struct hs_map_ {
    char nnick[30];
    char host[MAXHOST];
    char vhost[MAXHOST];
    char passwd[30];
    char added[30];
} hs_map;

hash_t *vhosts;

struct hs_lvl {
    int view;
    int list;
    int del;
    int add;
} hs_lvl;

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
void hs_cb_Config(char *arg, int configtype);
void hslog(char *, ...);
void hsdat(char *nick, char *host, char *vhost, char *pass, char *who);

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
    "2.3"
} };




static config_option options[] = {
{ "HOSTSERV_LVL_VIEW", ARG_STR, hs_cb_Config, 0},
{ "HOSTSERV_LVL_ADD", ARG_STR, hs_cb_Config, 1},
{ "HOSTSERV_LVL_DEL", ARG_STR, hs_cb_Config, 2},
{ "HOSTSERV_LVL_LIST", ARG_STR, hs_cb_Config, 3},
};


void hs_cb_Config(char *arg, int configtype) {
	int lvl;
	strcpy(segv_location, "HostServ-hs_cb_Config");

	if (configtype == 0) {
		/* View */
		lvl = atoi(arg);
		if (lvl > 200)
			return;
		hs_lvl.view = lvl;
	} else if (configtype == 1) {
		/* Add */
		lvl = atoi(arg);
		if (lvl > 200)
			return;
		hs_lvl.add = lvl;
	} else if (configtype == 2) {
		/* del */
		lvl = atoi(arg);
		if (lvl > 200)
			return;
		hs_lvl.del = lvl;
	} else if (configtype == 3) {
		/* list */
		lvl = atoi(arg);
		if (lvl > 200)
			return;
		hs_lvl.list = lvl;
	} 
	
}

int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module HostServ Loaded, Version: %s %s %s",HostServ_info[0].module_version,hsversion_date,hsversion_time);
    return 0;
}


/* Routine For Setting the Virtual Host */
static int hs_sign_on(char **av, int ac) {
    char *tmp = NULL;
    char *tmp2 = NULL;
    hnode_t *hn;
    hs_map *map;
    User *u;

        u = finduser(av[0]);
        if (!u) return 1;

        if (u->server->name == me.name) return 1;

        strcpy(segv_location, "HostServ-hs_signon");

    	if (findbot(u->nick)) return 1;
    	if (!load_synch) return 1;

    	/* Check HostName Against Data Contained in vhosts.data */        

      	hn = hash_lookup(vhosts, u->nick);
      	if (hn) {
      	  	map = hnode_get(hn);
          	tmp = strlower(map->host);
	  	tmp2 = strlower(u->hostname);
	  	if (fnmatch(tmp, tmp2, 0) == 0) {
          		ssvshost_cmd(u->nick, map->vhost);
			prefmsg(u->nick, s_HostServ, "Automatically setting your Virtual Host to %s", map->vhost);
              		return 1;
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
        } else if (!strcasecmp(av[2], "ADD") && (UserLevel(u) >= hs_lvl.add)) {
            privmsg_list(u->nick, s_HostServ, hs_help_add);
            return 1;
        } else if (!strcasecmp(av[2], "DEL") && (UserLevel(u) >= hs_lvl.del)) {
            privmsg_list(u->nick, s_HostServ, hs_help_del);
            return 1;        
        } else if (!strcasecmp(av[2], "LIST") && (UserLevel(u) >= hs_lvl.list)) {
            privmsg_list(u->nick, s_HostServ, hs_help_list);
            return 1;
	} else if (!strcasecmp(av[2], "VIEW") && (UserLevel(u) >= hs_lvl.view)) {
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
    } else if (!strcasecmp(av[1], "ADD") && (UserLevel(u) >= hs_lvl.add)) {
                if (ac < 6) {
                    prefmsg(u->nick, s_HostServ, "Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME> <PASSWORD>", s_HostServ);
                    prefmsg(u->nick, s_HostServ, "For addtional help: /msg %s HELP", s_HostServ);
                    return -1;
                }
                hs_add(u, av[2], av[3], av[4], av[5]);
    } else if (!strcasecmp(av[1], "DEL") && (UserLevel(u) >= hs_lvl.del)) {
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
    } else if (!strcasecmp(av[1], "LIST") && (UserLevel(u) >= hs_lvl.list)) {
                hs_list(u);
    } else if (!strcasecmp(av[1], "VIEW") && (UserLevel(u) >= hs_lvl.view)) {
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
	chanalert(s_HostServ, "%s tried %s, but they have no access, or command was not found", u->nick, av[1]);
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
    s_HostServ = malloc(MAXNICK);
    strcpy(s_HostServ, "HostServ");
    vhosts = hash_create(-1, 0, 0);
    if (!vhosts) {
	log("Error, Can't create vhosts hash");
	chanalert(s_Services, "Error, Can't create Vhosts Hash");
    }    	
    hs_lvl.add = 40;
    hs_lvl.del = 40;
    hs_lvl.list = 40;
    hs_lvl.view = 100;
	if (!config_read("neostats.cfg", options) == 0) {
		log("Error, HostServ could not be configured");
		chanalert(s_Services, "Error, HostServ could not be configured");
		return;
	}

    Loadhosts();
}

void _fini() {
	hnode_t *hn;
	hscan_t hs;
	free(s_HostServ);
	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		free(hnode_get(hn));
		hash_scan_delete(vhosts, hn);
	}
	hash_destroy(vhosts);

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
void write_database() {
        FILE *hsfile = fopen("data/vhosts.db", "w");
	hnode_t *hn;
	hscan_t hs;
	hs_map *map;

	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		map = hnode_get(hn);
	        fprintf(hsfile, ":%s %s %s %s %s\n", map->nnick, map->host, map->vhost, map->passwd, map->added);
	}
        fclose(hsfile);
}

/* Routine for registrations with the 'vhosts.db' file */
void hsdat(char *nick, char *host, char *vhost, char *pass, char *who)
{
	hnode_t *hn;
	hs_map *map;

	map = malloc(sizeof(hs_map));
	strncpy(map->nnick, nick, MAXNICK);
	strncpy(map->host, host, MAXHOST);
	strncpy(map->vhost, vhost, MAXHOST);
	strncpy(map->passwd, pass, 30);
	strncpy(map->added, who, MAXNICK);
	hn = hnode_create(map);
	hash_insert(vhosts, hn, map->nnick);

	write_database();

}


/* Routine for ADD */
static void hs_add(User *u, char *cmd, char *m, char *h, char *p) {

    char *tmp;
    strcpy(segv_location, "hs_add");
    hsdat(cmd, m, h, p, u->nick);
    hslog("%s added a vhost for %s with realhost %s vhost %s and password %s",u->nick, cmd, m, h, p);
    prefmsg(u->nick, s_HostServ, "%s has sucessfuly been registered under realhost: %s vhost: %s and password: %s",cmd, m, h, p);
    chanalert(s_HostServ, "%s added a vhost %s for %s with realhost %s", u->nick, h, cmd, h);
    /* Apply The New Hostname If The User Is Online */        
    if ((u = finduser(cmd)) != NULL) {
          if (findbot(cmd)) return;
          tmp = strlower(u->hostname);
	  if (fnmatch(m, tmp, 0) == 0) {
              ssvshost_cmd(u->nick, h);
              prefmsg(u->nick, s_HostServ, "%s is online now, setting vhost to %s", cmd, h);
              return;
          }
    }
}


/* Routine for 'HostServ' to print out its data */
static void hs_list(User *u)
{
    int i;
    hnode_t *hn;
    hscan_t hs;
    hs_map *map;

    strcpy(segv_location, "hs_list");

    i = 1;
    prefmsg(u->nick, s_HostServ, "Current HostServ VHOST list:");
    prefmsg(u->nick, s_HostServ, "%-5s %-12s %-30s","Num", "Nick", "Vhost");
    hash_scan_begin(&hs, vhosts);
    while ((hn = hash_scan_next(&hs)) != NULL) {
	map = hnode_get(hn);
        prefmsg(u->nick, s_HostServ, "%-5d %-12s %-30s", i, map->nnick, map->vhost);
	i++;
    }
    prefmsg(u->nick, s_HostServ, "For more information on someone use /msg %s VIEW #", s_HostServ);
    prefmsg(u->nick, s_HostServ, "--- End of List ---");
}

/* Routine for VIEW */
static void hs_view(User *u, int tmpint)
{
    int i;
    hnode_t *hn;
    hscan_t hs;
    hs_map *map;
    strcpy(segv_location, "hs_view");

    i = 1;
    hash_scan_begin(&hs, vhosts);
    while ((hn = hash_scan_next(&hs)) != NULL) {
        if (tmpint == i) {
	    map = hnode_get(hn);
	    prefmsg(u->nick, s_HostServ, "Virtual Host information:");
            prefmsg(u->nick, s_HostServ, "Nick:     %s", map->nnick);
	    prefmsg(u->nick, s_HostServ, "RealHost: %s", map->host);
	    prefmsg(u->nick, s_HostServ, "V-host:   %s", map->vhost);
	    prefmsg(u->nick, s_HostServ, "Password: %s", map->passwd);
	    prefmsg(u->nick, s_HostServ, "Added by: %s", map->added ? map->added : "<unknown>");
	    prefmsg(u->nick, s_HostServ, "--- End of information for %s ---",map->nnick);
        }
	i++;
    }
    if (tmpint > i) prefmsg(u->nick, s_HostServ, "ERROR: There is no vhost on list number \2%d\2",tmpint);
}




/* Routine For Loading The Hosts into Array */
void Loadhosts()
{
    FILE *fp = fopen("data/vhosts.db", "r");
    hs_map *map;
    hnode_t *hn;
    char buf[512];

    if (fp) {
    	load_synch = 1;
		while (fgets(buf, 512, fp)) {
        	    strip(buf);
	            map = malloc(sizeof(hs_map));

        	    LoadArryCount = split_buf(buf, &LoadArry, 0);
	            strcpy(map->nnick, LoadArry[0]);
        	    strcpy(map->host, LoadArry[1]);
	            strcpy(map->vhost, LoadArry[2]);
		    if (LoadArryCount > 3) { /* Check for upgrades from earlier versions */
			strcpy(map->passwd, LoadArry[3]);
		    } else /* Upgrading from earlier version, no passwds exist */
			strcpy(map->passwd, NULL);
		    if (LoadArryCount > 4) { /* Does who set it exist? Yes? go ahead */
	    		strcpy(map->added, LoadArry[4]);
		    } else /* We have no information on who set it so its null */
		    	strcpy(map->added, "0");
		    /* add it to the hash */
		    hn = hnode_create(map);
		    hash_insert(vhosts, hn, map->nnick);
	    	}
    	fclose(fp);
    } 
}


/* Routine for HostServ to delete the given information */
static void hs_del(User *u, int tmpint)
{

    int i = 1;
    hnode_t *hn;
    hscan_t hs;
    hs_map *map;

    strcpy(segv_location, "hs_del");

    hash_scan_begin(&hs, vhosts);
    while ((hn = hash_scan_next(&hs)) != NULL) {
	      if (i == tmpint) {
		map = hnode_get(hn);
		prefmsg(u->nick, s_HostServ, "The following vhost was removed from the Vhosts Database");
	        prefmsg(u->nick, s_HostServ, "\2%s - %s\2", map->nnick, map->vhost);
		hslog("%s removed the VHOST: %s for %s", u->nick,map->vhost,map->nnick);
		chanalert(s_HostServ, "%s removed vhost %s for %s", u->nick, map->vhost, map->nnick);
		hash_scan_delete(vhosts, hn);
		free(map);
		hnode_destroy(hn);
	    }
	i++;
    }

    if (tmpint > i) 
	prefmsg(u->nick, s_HostServ, "ERROR: There is no vhost on list number \2%d\2",tmpint);
    else
    	write_database();
    return;
}


/* Routine to allow users to login and get their vhosts */
static void hs_login(User *u, char *login, char *pass)
{
        hs_map *map;
        hnode_t *hn;
        
        strcpy(segv_location, "HostServ-hs_login");

    	/* Check HostName Against Data Contained in vhosts.data */
      	hn = hash_lookup(vhosts, login);
	if (hn) {
		map = hnode_get(hn);
		if (!strcasecmp(map->passwd, pass)) {
	        	ssvshost_cmd(u->nick, map->vhost);
		      	prefmsg(u->nick, s_HostServ, "Your VHOST %s has been set.", map->vhost);
		      	hslog("%s used LOGIN to obtain userhost of %s",u->nick, map->vhost);
			chanalert(s_HostServ, "%s used login to get Vhost %s", u->nick, map->vhost);
	              	return;
           	}
       	}
	prefmsg(u->nick, s_HostServ, "Incorrect Login or Password.  Do you have a vhost added?");
        return;
}

