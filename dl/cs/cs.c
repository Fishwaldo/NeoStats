/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module: ConnectServ
** Description: Network Connection & Mode Monitoring Service
** Version: 1.3
** Authors: ^Enigma^ & Shmad
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "cs_help.c"

const char csversion_date[] = __DATE__;
const char csversion_time[] = __TIME__;
char *s_ConnectServ;

extern const char *cs_help[];
int cs_new_user(char **av, int ac);
int cs_user_modes(char **av, int ac);
int cs_del_user(char **av, int ac);
int cs_user_kill(char **av, int ac);

static void cs_status(User *);
static void cs_signwatch(User *u);
static void cs_killwatch(User *u);
static void cs_modewatch(User *u);

int sign_watch;
int kill_watch;
int mode_watch;

void cslog(char *, ...);
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

Module_Info my_info[] = { {
    "ConnectServ",
    "Network Connection & Mode Monitoring Service",
    "1.4"
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

    if (init_bot(s_ConnectServ,"ConnectServ",me.name,"Network Connection & Mode Monitoring Service", "+oikSdwgleq-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_ConnectServ = strcat(s_ConnectServ, "_");
        init_bot(s_ConnectServ,"ConnectServ",me.name,"Network Connection & Mode Monitoring Service", "+oikSdwgleq-x", my_info[0].module_name);
    }
    return 1;
};

EventFnList my_event_list[] = {
    { "ONLINE", Online},
    { "SIGNON", cs_new_user},
    { "UMODE", cs_user_modes},
    { "SIGNOFF", cs_del_user},
    { "KILL", cs_user_kill},
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

    s_ConnectServ = "ConnectServ";
    globops(me.name, "ConnectServ Module Loaded",me.name);

}

void _fini() {
    globops(me.name, "ConnectServ Module Unloaded",me.name);

};


/* Routine for logging items with the 'cslog' */
void cslog(char *fmt, ...)
{
        va_list ap;
        FILE *csfile = fopen("logs/ConnectServ.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!csfile) {
        log("Unable to open logs/ConnectServ.log for writing.");
        return;
        }

    fprintf(csfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(csfile);

}


/* Routine for saving settings to connect.db */
void SaveSettings()
{
        FILE *fp = fopen("data/connect.db", "w");
        fprintf(fp, ":SIGNWATCH %i\n", sign_watch);
        fprintf(fp, ":KILLWATCH %i\n", kill_watch);
        fprintf(fp, ":MODEWATCH %i\n", mode_watch);
        fclose(fp);
}


/* Routine for SIGNON message to be echoed */
int cs_new_user(char **av, int ac) {
    User *u;
    /* Approximate Segfault Location */
    strcpy(segv_location, "cs_new_user");
    u = finduser(av[0]);
    /* Print Connection Notice */
    if (sign_watch) {
    if (is_synced) chanalert(s_ConnectServ, "\2SIGNON\2 %s (%s@%s) has Signed on at %s", u->nick, u->username, u->hostname, u->server->name);
    }
    if (findbot(u->nick)) return 1;
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

		  if (is_synced) chanalert(s_ConnectServ, "\2LOCAL KILL\2 %s (%s@%s) was Killed by %s - Reason sighted: \2%s\2", u->nick, u->username, u->hostname, Local[6], KillMsg);
          free(KillMsg);
		  return 1;

	   }
    }

    /* Print Disconnection Notice */
    if (sign_watch) {
        if (is_synced) chanalert(s_ConnectServ, "\2SIGNOFF\2 %s (%s@%s) has Signed off at %s - %s", u->nick, u->username, u->hostname, u->server->name, QuitMsg);
	}
	free(QuitMsg);

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
        cslog("Changing modes for unknown user: %s", u->nick);
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
            case 'N':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2NetAdmin\2 %s is Now a Network Administrator (+N)", u->nick);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2NetAdmin\2 %s is No Longer a Network Administrator (-N)", u->nick);
                }
                break;
            case 'S':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2Services\2 %s is Now a Network Service (+S)", u->nick);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2Services\2 %s is No Longer a Network Service (-S)", u->nick);
                }
                break;
            case 'T':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2TechAdmin\2 %s is Now a Network Technical Administrator (+T)", u->nick);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2TechAdmin\2 %s is No Longer a Network Technical Administrator (-T)", u->nick);
                }
                break;
            case 'A':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2ServerAdmin\2 %s is Now a Server Administrator on %s (+A)", u->nick, u->server->name);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2ServerAdmin\2 %s is No Longer a Server Administrator on %s (-A)", u->nick, u->server->name);
                }
                break;
            case 'a':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2ServicesAdmin\2 %s is Now a Services Administrator (+a)", u->nick);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2ServicesAdmin\2 %s is No Longer a Services Administrator (-a)", u->nick);
                }
                break;
            case 'C':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2Co-ServerAdmin\2 %s is Now a Co-Server Administrator on %s (+C)", u->nick, u->server->name);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2Co-ServerAdmin\2 %s is No Longer a Co-Server Administrator on %s (-C)", u->nick, u->server->name);
                }
                break;
            case 'B':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2Bot\2 %s is Now a Bot (+B)", u->nick);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2Bot\2 %s is No Longer a Bot (-B)", u->nick);
                }
                break;
            case 'I':
                if (add) {
                   globops(s_ConnectServ,"\2%s\2 Is Using \2Invisible Mode\2 (+I)",u->nick);
                } else {
                   globops(s_ConnectServ,"\2%s\2 Is no longer using \2Invisible Mode\2 (-I)",u->nick);
                }
                break;
            case 'o':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2Oper\2 %s is Now a Oper on %s (+o)", u->nick, u->server->name);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2Oper\2 %s is No Longer a Oper on %s (-o)", u->nick, u->server->name);
                }
                break;
            case 'O':
                if (add) {
                    if (is_synced) chanalert(s_ConnectServ, "\2Oper\2 %s is Now a Oper on %s (+o)", u->nick, u->server->name);
                } else {
                    if (is_synced) chanalert(s_ConnectServ, "\2Oper\2 %s is No Longer a Oper on %s (-o)", u->nick, u->server->name);
                }
                break;
            default: 
                break; 
        }
    }
    return 1;
}


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
        if (kill_watch) if (is_synced) chanalert(s_ConnectServ, "\2GLOBAL KILL\2 %s (%s@%s) was Killed by %s - Reason sighted: \2%s\2", u->nick, u->username, u->hostname, Kill[0], GlobalMsg);
	} else if (findserver(Kill[2])) {
        if (kill_watch) if (is_synced) chanalert(s_ConnectServ, "\2SERVER KILL\2 %s was Killed by the Server %s - Reason sighted: %s", u->nick, Kill[0], GlobalMsg);
    }
    free(GlobalMsg);
	return 1;
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
        if (is_synced) chanalert(s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Activated by %s",u->nick);
        cslog("%s!%s@%s Activated SIGNON/SIGNOFF WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        prefmsg(u->nick, s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Activated");
   } else {
        sign_watch = 0;
        if (is_synced) chanalert(s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Deactivated by %s",u->nick);
        cslog("%s!%s@%s Deactivated SIGNON/SIGNOFF WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
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
        if (is_synced) chanalert(s_ConnectServ, "\2KILL WATCH\2 Activated by %s",u->nick);
        cslog("%s!%s@%s Activated KILL WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        prefmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 Activated");
   } else {
        kill_watch = 0;
        if (is_synced) chanalert(s_ConnectServ, "\2KILL WATCH\2 Deactivated by %s",u->nick);
        cslog("%s!%s@%s Deactivated KILL WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
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
        if (is_synced) chanalert(s_ConnectServ, "\2MODE WATCH\2 Activated by %s",u->nick);
        cslog("%s!%s@%s Activated MODE WATCH", u->nick, u->username, u->hostname);
        SaveSettings();    
        prefmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 Activated");
   } else {
        mode_watch = 0;
        if (is_synced) chanalert(s_ConnectServ, "\2MODE WATCH\2 Deactivated by %s",u->nick);
        cslog("%s!%s@%s Deactivated MODE WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
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

}

/* Load ConnectServ Config file and set defaults if does not exist */
void Loadconfig()
{
    FILE *fp = fopen("data/connect.db", "r");
    char buf[BUFSIZE];
    strcpy(segv_location, "cs_Loadconfig");

	if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);
            ConfigCount = split_buf(buf, &Config, 0);

            if (!strcasecmp(Config[0], "SIGNWATCH")) {
                sign_watch = atoi(Config[1]);
            } else if (!strcasecmp(Config[0], "KILLWATCH")) {
                                kill_watch = atoi(Config[1]);
            } else if (!strcasecmp(Config[0], "MODEWATCH")) {
                                mode_watch = atoi(Config[1]);
            } else {
                cslog("%s is not a valid connect.db option!", Config[0]);
                if (is_synced) chanalert(s_Services, "%s is not a valid connect.db option! Please check your data/connect.db file!", Config[0]);
            }
    }
        fclose(fp);
    } else {
        if (is_synced) chanalert(s_Services, "No Database Found! Creating one with Defaults!");
        sign_watch=1;
        kill_watch=1;
        mode_watch=1; 
    }
}
