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
** $Id: cs.c,v 1.24 2003/05/22 13:51:55 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "cs_help.c"
#include "cs.h"
#include "log.h"
#include "conf.h"

const char csversion_date[] = __DATE__;
const char csversion_time[] = __TIME__;
char *s_ConnectServ;

extern const char *cs_help[];
int cs_new_user(char **av, int ac);
int cs_user_modes(char **av, int ac);
int cs_user_smodes(char **av, int ac);
int cs_del_user(char **av, int ac);
int cs_user_kill(char **av, int ac);
int cs_user_nick(char **av, int ac);

static void cs_status(User *);
static void cs_signwatch(User *u);
static void cs_killwatch(User *u);
static void cs_modewatch(User *u);
static void cs_nickwatch(User *u);

int sign_watch;
int kill_watch;
int mode_watch;
int nick_watch;

void SaveSettings();
void Loadconfig();

char **Quit;
char **Local;
char **Kill;
char **Config;
int QuitCount = 0;
int LocalCount = 0;
int KillCount = 0;
int ConfigCount = 0;
int cs_online = 0;


Module_Info my_info[] = { {
    "ConnectServ",
    "Network Connection & Mode Monitoring Service",
    "1.6"
} };

int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module ConnectServ Loaded, Version: %s %s %s",my_info[0].module_version,csversion_date,csversion_time);
    return 0;
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
        if (ac <= 2 && (!(UserLevel(u) >= 185))) {
            prefmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
            return 1;
        } else if (ac <= 2) {
            privmsg_list(u->nick, s_ConnectServ, cs_help);
            return 1;
	} else if (!strcasecmp(av[2], "NICKWATCH") && (UserLevel(u) >= 185)) {
	    privmsg_list(u->nick, s_ConnectServ, cs_help_nickwatch);
	    return 1;
        } else if (!strcasecmp(av[2], "SIGNWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_signwatch);
            return 1;
        } else if (!strcasecmp(av[2], "KILLWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_killwatch);
            return 1;
        } else if (!strcasecmp(av[2], "MODEWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_modewatch);
            return 1;
        } else if (!strcasecmp(av[2], "STATUS")) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_status);
            return 1;
        } else if (!strcasecmp(av[2], "ABOUT")) {
                privmsg_list(u->nick, s_ConnectServ, cs_help_about);
            return 1;        
        } else 
            prefmsg(u->nick, s_ConnectServ, "Unknown Help Topic: \2%s\2", av[2]);
        }
	
	if (!strcasecmp(av[1], "ABOUT")) {
                privmsg_list(u->nick, s_ConnectServ, cs_help_about);
    } else if (!strcasecmp(av[1], "NICKWATCH") && (UserLevel(u) >= 185)) {
                cs_nickwatch(u);
    } else if (!strcasecmp(av[1], "SIGNWATCH") && (UserLevel(u) >= 185)) {
                cs_signwatch(u);
    } else if (!strcasecmp(av[1], "KILLWATCH") && (UserLevel(u) >= 185)) {
                cs_killwatch(u);
    } else if (!strcasecmp(av[1], "MODEWATCH") && (UserLevel(u) >= 185)) {
                cs_modewatch(u);
    } else if (!strcasecmp(av[1], "STATUS")) {
                cs_status(u);
    } else {
        prefmsg(u->nick, s_ConnectServ, "Unknown Command: \2%s\2, perhaps you need some HELP?", av[1]);
    }
    return 1;

}

int Online(char **av, int ac) {
	char *user;
	char *host;
	char *rname;
	
	if (GetConf((void *)&s_ConnectServ, CFGSTR, "Nick") < 0) {
		s_ConnectServ = "ConnectServ";
	}
	if (GetConf((void *)&user, CFGSTR, "User") < 0) {
		user = malloc(MAXUSER);
		snprintf(user, MAXUSER, "CS");
	}
	if (GetConf((void *)&host, CFGSTR, "Host") < 0) {
		host = malloc(MAXHOST);
		snprintf(host, MAXHOST, me.name);
	}
	if (GetConf((void *)&rname, CFGSTR, "RealName") < 0) {
		rname = malloc(MAXHOST);
		snprintf(rname, MAXHOST, "Connection Monitoring Service");
	}


    if (init_bot(s_ConnectServ,user, host, rname, "+oS", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_ConnectServ = strcat(s_ConnectServ, "_");
        init_bot(s_ConnectServ,user, host, rname, "+oS", my_info[0].module_name);
    }
    cs_online = 1;
    free(user);
    free(host);
    free(rname);
    return 1;
};

EventFnList my_event_list[] = {
    { "ONLINE", Online},
    { "SIGNON", cs_new_user},
    { "UMODE", cs_user_modes},
#ifdef ULTIMATE3
    { "SMODE", cs_user_smodes},
#endif
    { "SIGNOFF", cs_del_user},
    { "KILL", cs_user_kill},
    { "NICK_CHANGE", cs_user_nick},
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
    Loadconfig();
}

void _fini() {

};




/* Routine for SIGNON message to be echoed */
int cs_new_user(char **av, int ac) {
    User *u;
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_new_user");
    u = finduser(av[0]);
    /* Print Connection Notice */
    if (u && sign_watch) {
    if (cs_online) chanalert(s_ConnectServ, "\2SIGNON\2 %s (%s@%s) has Signed on at %s", u->nick, u->username, u->hostname, u->server->name);
    }
  return 1;
}


/* Routine for SIGNOFF message to be echoed */
int cs_del_user(char **av, int ac) {
    char *cmd, *lcl, *QuitMsg, *KillMsg;
    User *u;
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_del_user");
    u = finduser(av[0]);
    
    cmd = sstrdup(recbuf);
    lcl = sstrdup(recbuf);

    QuitCount = split_buf(cmd, &Quit, 0);
    QuitMsg = joinbuf (Quit, QuitCount, 2);

    /* Local Kill Watch For Signoff */
    if (kill_watch) {
        if (strstr(cmd ,"Local kill by") && strstr(cmd, "[") && strstr(cmd, "]")) {

		  LocalCount = split_buf(lcl, &Local, 0);
          	  KillMsg = joinbuf (Local, LocalCount, 7);
		  if (cs_online) chanalert(s_ConnectServ, "\2LOCAL KILL\2 %s (%s@%s) was Killed by %s - Reason sighted: \2%s\2", u->nick, u->username, u->hostname, Local[6], KillMsg);
          	  free(KillMsg);
		  free(QuitMsg);
	  	  free(cmd);
		  free(lcl);
		  return 1;

	   }
    }

    /* Print Disconnection Notice */
    if (sign_watch) {
        if (cs_online) chanalert(s_ConnectServ, "\2SIGNOFF\2 %s (%s@%s) has Signed off at %s - %s", u->nick, u->username, u->hostname, u->server->name, QuitMsg);
    }
    free(QuitMsg);
    free(cmd);
    free(lcl);
    free(Quit);
    QuitCount = 0;
    return 1;
}


/* Routine for MODES message to be echoed */
int cs_user_modes(char **av, int ac) {
    int add = 1;
    char *modes;
    User *u;

    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_user_modes");

    if (mode_watch != 1) return -1;

    u = finduser(av[0]);
    if (!u) {
        nlog(LOG_WARNING, LOG_MOD, "Changing modes for unknown user: %s", u->nick);
        return -1;
    }
  
    if (!u->modes) return -1; 
    modes = u->modes;
    switch (*av[1]) {
    	case '+': add = 1;	break;
        case '-': add = 0;	break;
    }

    while (*av[1]++) {
        switch(*av[1]) {
            case '+': add = 1;    break;
            case '-': add = 0;    break;
#ifndef ULTIMATE3
/* these modes in Ultimate3 are Smodes */
            case NETADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2NetAdmin\2 %s is Now a Network Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2NetAdmin\2 %s is No Longer a Network Administrator", u->nick);
                }
                break;
	    case CONETADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-NetAdmin\2 %s is Now a Co-Network Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-NetAdmin\2 %s is No Longer a Co-Network Administrator", u->nick);
                }
                break;
            case TECHADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2TechAdmin\2 %s is Now a Network Technical Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2TechAdmin\2 %s is No Longer a Network Technical Administrator", u->nick);
                }
                break;
            case SERVERADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2ServerAdmin\2 %s is Now a Server Administrator on %s", u->nick, u->server->name);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2ServerAdmin\2 %s is No Longer a Server Administrator on %s", u->nick, u->server->name);
                }
                break;
            case COSERVERADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-ServerAdmin\2 %s is Now a Co-Server Administrator on %s", u->nick, u->server->name);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-ServerAdmin\2 %s is No Longer a Co-Server Administrator on %s", u->nick, u->server->name);
                }
                break;
	    case GUESTADMIN_MODE:
		if (add) {
		    if (cs_online) chanalert(s_ConnectServ, "\2GuestAdmin\2 %s is Now a Guest Administrator on %s", u->nick, u->server->name);
		} else {
		    if (cs_online) chanalert(s_ConnectServ, "\2GuestAdmin\2 %s is No Longer a Guest Administrator on %s", u->nick, u->server->name);
		}
		break;
/* these modes are not used in Ultimate3 */
            case BOT_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Bot\2 %s is Now a Bot", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Bot\2 %s is No Longer a Bot", u->nick);
                }
                break;
            case INVISIBLE_MODE:
                if (add) {
                   globops(s_ConnectServ,"\2%s\2 Is Using \2Invisible Mode\2",u->nick);
                } else {
                   globops(s_ConnectServ,"\2%s\2 Is no longer using \2Invisible Mode\2",u->nick);
                }
                break;
#endif
            case SERVICESADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2ServicesAdmin\2 %s is Now a Services Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2ServicesAdmin\2 %s is No Longer a Services Administrator", u->nick);
                }
                break;
            case OPER_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Oper\2 %s is Now a Oper on %s", u->nick, u->server->name);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Oper\2 %s is No Longer a Oper on %s", u->nick, u->server->name);
                }
                break;
            case LOCOP_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2LocalOper\2 %s is Now a Local Oper on %s", u->nick, u->server->name);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2LocalOper\2 %s is No Longer a Local Oper on %s", u->nick, u->server->name);
                }
                break;
            case NETSERVICE_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Services\2 %s is Now a Network Service", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Services\2 %s is No Longer a Network Service", u->nick);
                }
                break;
            default: 
                break; 
        }
    }
    return 1;
}

#ifdef ULTIMATE3
/* smode support for Ultimate3 */
int cs_user_smodes(char **av, int ac) {
    int add = 1;
    char *modes;
    User *u;

    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_user_modes");

    if (mode_watch != 1) return -1;

    u = finduser(av[0]);
    if (!u) {
        nlog(LOG_WARNING, LOG_MOD, "Changing modes for unknown user: %s", u->nick);
        return -1;
    }
  
    if (!u->modes) return -1; 
    modes = u->modes;
    switch (*av[1]) {
    	case '+': add = 1;	break;
        case '-': add = 0;	break;
    }

    while (*av[1]++) {
        switch(*av[1]) {
            case '+': add = 1;    break;
            case '-': add = 0;    break;
            case NETADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2NetAdmin\2 %s is Now a Network Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2NetAdmin\2 %s is No Longer a Network Administrator", u->nick);
                }
                break;
	    case CONETADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-NetAdmin\2 %s is Now a Co-Network Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-NetAdmin\2 %s is No Longer a Co-Network Administrator", u->nick);
                }
                break;
            case TECHADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2TechAdmin\2 %s is Now a Network Technical Administrator", u->nick);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2TechAdmin\2 %s is No Longer a Network Technical Administrator", u->nick);
                }
                break;
	    case COTECHADMIN_MODE:
		if (add) {
			if (cs_online) chanalert(s_ConnectServ, "\2Co-TechAdmin\2 %s is Now a Network Co-Technical Administrator", u->nick);
		} else {
		 	if (cs_online) chanalert(s_ConnectServ, "\2Co-TechAdmin\2 %s is No Longer a Network Co-Technical Administrator", u->nick);
	 	}
		break;
            case SERVERADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2ServerAdmin\2 %s is Now a Server Administrator on %s", u->nick, u->server->name);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2ServerAdmin\2 %s is No Longer a Server Administrator on %s", u->nick, u->server->name);
                }
                break;
            case COSERVERADMIN_MODE:
                if (add) {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-ServerAdmin\2 %s is Now a Co-Server Administrator on %s", u->nick, u->server->name);
                } else {
                    if (cs_online) chanalert(s_ConnectServ, "\2Co-ServerAdmin\2 %s is No Longer a Co-Server Administrator on %s", u->nick, u->server->name);
                }
                break;
	    case GUESTADMIN_MODE:
		if (add) {
		    if (cs_online) chanalert(s_ConnectServ, "\2GuestAdmin\2 %s is Now a Guest Administrator on %s", u->nick, u->server->name);
		} else {
		    if (cs_online) chanalert(s_ConnectServ, "\2GuestAdmin\2 %s is No Longer a Guest Administrator on %s", u->nick, u->server->name);
		}
		break;
            default: 
                break; 
        }
    }
    return 1;
}
#endif

/* Routine for KILL message to be echoed */
int cs_user_kill(char **av, int ac) {
    char *cmd, *GlobalMsg;
    User *u;
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_user_kill");
    u = finduser(av[0]);
    cmd = sstrdup(recbuf);
    KillCount = split_buf(cmd, &Kill, 0);
	GlobalMsg = joinbuf (Kill, KillCount, 4);

    if (finduser(Kill[2])) {
    /* it was a User who was killed */
        if (kill_watch) if (cs_online) chanalert(s_ConnectServ, "\2GLOBAL KILL\2 %s (%s@%s) was Killed by %s - Reason sighted: \2%s\2", u->nick, u->username, u->hostname, Kill[0], GlobalMsg);
	} else if (findserver(Kill[2])) {
        if (kill_watch) if (cs_online) chanalert(s_ConnectServ, "\2SERVER KILL\2 %s was Killed by the Server %s - Reason sighted: %s", u->nick, Kill[0], GlobalMsg);
    }
    free(GlobalMsg);
	return 1;
}

/* If a user has changed their nick say so */
int cs_user_nick(char **av, int ac) {
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_user_nick");

    if (nick_watch) if (cs_online) chanalert(s_ConnectServ, "\2NICK\2 %s Changed their nick to %s", av[0], av[1]);
    return 1;
}


static void cs_nickwatch(User *u)
{
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_nickwatch");
    if (!(UserLevel(u) >= 185)) {
        prefmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!nick_watch) {
        nick_watch = 1;
        if (cs_online) chanalert(s_ConnectServ, "\2NICK WATCH\2 Activated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated NICK WATCH", u->nick, u->username, u->hostname);
	SetConf((void *)1, CFGBOOL, "NickWatch");
        prefmsg(u->nick, s_ConnectServ, "\2NICK WATCH\2 Activated");
   } else {
        nick_watch = 0;
        if (cs_online) chanalert(s_ConnectServ, "\2NICK WATCH\2 Deactivated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Deactivated NICK WATCH", u->nick, u->username, u->hostname);
	SetConf(0, CFGBOOL, "NickWatch");
        prefmsg(u->nick, s_ConnectServ, "\2NICK WATCH\2 Deactivated");
    }

}



/* Routine for Signon/Signoff ENABLE or DISABLE */
static void cs_signwatch(User *u)
{
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_signwatch");
    if (!(UserLevel(u) >= 185)) {
        prefmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!sign_watch) {
        sign_watch = 1;
        if (cs_online) chanalert(s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Activated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated SIGNON/SIGNOFF WATCH", u->nick, u->username, u->hostname);
	SetConf((void *)1, CFGBOOL, "SignWatch");
        prefmsg(u->nick, s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Activated");
   } else {
        sign_watch = 0;
        if (cs_online) chanalert(s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Deactivated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Deactivated SIGNON/SIGNOFF WATCH", u->nick, u->username, u->hostname);
	SetConf(0, CFGBOOL, "SignWatch");
        prefmsg(u->nick, s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Deactivated");
    }

}


/* Routine for kill watch ENABLE or DISABLE */
static void cs_killwatch(User *u)
{
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_killwatch");
    if (!(UserLevel(u) >= 185)) {
        prefmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!kill_watch) {
        kill_watch = 1;
        if (cs_online) chanalert(s_ConnectServ, "\2KILL WATCH\2 Activated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated KILL WATCH", u->nick, u->username, u->hostname);
	SetConf((void *)1, CFGBOOL, "KillWatch");
        prefmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 Activated");
   } else {
        kill_watch = 0;
        if (cs_online) chanalert(s_ConnectServ, "\2KILL WATCH\2 Deactivated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Deactivated KILL WATCH", u->nick, u->username, u->hostname);
	SetConf(0, CFGBOOL, "KillWatch");
        prefmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 Deactivated");
    }

}


/* Routine for mode watch ENABLE or DISABLE */
static void cs_modewatch(User *u)
{
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_modewatch");
    if (!(UserLevel(u) >= 185)) {
        prefmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!mode_watch) {
        mode_watch = 1;
        if (cs_online) chanalert(s_ConnectServ, "\2MODE WATCH\2 Activated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated MODE WATCH", u->nick, u->username, u->hostname);
	SetConf((void *)1, CFGBOOL, "ModeWatch");
        prefmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 Activated");
   } else {
        mode_watch = 0;
        if (cs_online) chanalert(s_ConnectServ, "\2MODE WATCH\2 Deactivated by %s",u->nick);
        nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Deactivated MODE WATCH", u->nick, u->username, u->hostname);
	SetConf(0, CFGBOOL, "ModeWatch");
        prefmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 Deactivated");
    }

}


/* Routine for STATUS echo */
static void cs_status(User *u)
{
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_status");

    prefmsg(u->nick, s_ConnectServ, "Current %s Settings:", s_ConnectServ);

    /* SIGNWATCH Check */
    if (!sign_watch) {
        prefmsg(u->nick, s_ConnectServ, "\2SIGN WATCH\2 is Not Currently Active");
    }
    if (sign_watch) {   
        prefmsg(u->nick, s_ConnectServ, "\2SIGN WATCH\2 is Currently Active");
    }

    /* KILLWATCH Check */
    if (!kill_watch) {
        prefmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 is Not Currently Active");
    }
    if (kill_watch) {   
        prefmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 is Currently Active");
    }

    /* MODEWATCH Check */
    if (!mode_watch) {
        prefmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 is Not Currently Active");
    }
    if (mode_watch) {   
        prefmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 is Currently Active");
    }

    /* NICKWATCH Check */
    if (!nick_watch) {
	prefmsg(u->nick, s_ConnectServ, "\2NICK WATCH\2 is Not Currently Active");
    }
    if (nick_watch) {
	prefmsg(u->nick, s_ConnectServ, "\2NICK WATCH\2 is Currently Active");
    }

}

/* Load ConnectServ Config file and set defaults if does not exist */
void Loadconfig()
{
    	strcpy(segv_location, "cs_Loadconfig");
	strcpy(segvinmodule, my_info[0].module_name);

	/* some defaults */
        sign_watch=1;
        kill_watch=1;
        mode_watch=1; 
	nick_watch=1;

	GetConf((void *)&sign_watch, CFGBOOL, "SignWatch");
	GetConf((void *)&kill_watch, CFGBOOL, "KillWatch");
	GetConf((void *)&mode_watch, CFGBOOL, "ModeWatch");
	GetConf((void *)&nick_watch, CFGBOOL, "NickWatch");
}
