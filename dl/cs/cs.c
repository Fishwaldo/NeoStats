/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module: ConnectServ
** Description: Network Connection & Mode Monitoring Service
** Version: 1.2
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
static int cs_new_user(User *);
static int cs_user_modes(User *);
static int cs_del_user(User *);
static int cs_user_kill(User *);

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


Module_Info my_info[] = { {
    "cs",
    "Network Connection & Mode Monitoring Service",
    "1.2"
} };

int new_m_version(char *av, char *tmp) {
    sts(":%s 351 %s :Module ConnectServ Loaded, Version: %s %s %s",me.name,av,my_info[0].module_version,csversion_date,csversion_time);
    return 0;
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
        if (!coreLine&& (!(UserLevel(u) >= 185))) {
            privmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
            return 1;
        } else if (!coreLine && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help); 
            return 1;
        } else if (!strcasecmp(coreLine, "SIGNWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_signwatch);
            return 1;
        } else if (!strcasecmp(coreLine, "KILLWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_killwatch);
            return 1;
        } else if (!strcasecmp(coreLine, "MODEWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_modewatch);
            return 1;
        } else if (!strcasecmp(coreLine, "STATUS")) {
            privmsg_list(u->nick, s_ConnectServ, cs_help_status);
            return 1;
        } else if (!strcasecmp(coreLine, "ABOUT")) {
                privmsg_list(u->nick, s_ConnectServ, cs_help_about);
            return 1;        
        } else 
            privmsg(u->nick, s_ConnectServ, "Unknown Help Topic: \2%s\2",coreLine);
        }

    if (!strcasecmp(cmd, "ABOUT")) {
                privmsg_list(u->nick, s_ConnectServ, cs_help_about);
    } else if (!strcasecmp(cmd, "SIGNWATCH") && (UserLevel(u) >= 185)) {
                cs_signwatch(u);
    } else if (!strcasecmp(cmd, "KILLWATCH") && (UserLevel(u) >= 185)) {
                cs_killwatch(u);
    } else if (!strcasecmp(cmd, "MODEWATCH") && (UserLevel(u) >= 185)) {
                cs_modewatch(u);
    } else if (!strcasecmp(cmd, "STATUS")) {
                cs_status(u);
    } else {
        privmsg(u->nick, s_ConnectServ, "Unknown Command: \2%s\2, perhaps you need some HELP?",cmd);
    }
    return 1;

}

int Online(Server *data) {

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
    User *u;
    int i;

    Loadconfig();

    s_ConnectServ = "ConnectServ";
    sts(":%s GLOBOPS :ConnectServ Module Loaded",me.name);

         for (i=0; i < U_TABLE_SIZE; i++) {
              for (u = userlist[i]; u; u = u->next) {
              }
         }
}

void _fini() {
    sts(":%s GLOBOPS :ConnectServ Module Unloaded",me.name);

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
        fprintf(fp, "SIGNWATCH %i\n", sign_watch);
        fprintf(fp, "KILLWATCH %i\n", kill_watch);
        fprintf(fp, "MODEWATCH %i\n", mode_watch);
        fclose(fp);
}


/* Routine for SIGNON message to be echoed */
static int cs_new_user(User *u) {
    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_new_user");

    /* Print Connection Notice */
    if (sign_watch) {
    notice(s_ConnectServ, "\2SIGNON\2 %s(%s@%s) has Signed on at %s", u->nick, u->username, u->hostname, u->server->name);
    }
    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for SIGNOFF message to be echoed */
static int cs_del_user(User *u) {
    char *cmd, *lcl, *who;

    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_del_user");

    cmd = sstrdup(recbuf);
    lcl = sstrdup(recbuf);
    cmd = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, "");
    cmd++;

    /* Local Kill Watch For Signoff */
    if (kill_watch) {
        if (strstr(cmd ,"Local kill by") && strstr(cmd, "[") && strstr(cmd, "]")) {
       lcl = strtok(lcl, " ");
       lcl = strtok(NULL, " ");
       lcl = strtok(NULL, " ");
       lcl = strtok(NULL, " ");
       lcl = strtok(NULL, " ");
       lcl = strtok(NULL, " ");
       lcl = strtok(NULL, " ");
       who = strtok(NULL, " ");
	   lcl = strtok(NULL, "");

		  notice(s_ConnectServ, "\2LOCAL KILL\2 %s(%s@%s) was Killed by %s - Reason sighted: \2%s\2", u->nick, u->username, u->hostname, who, lcl);
          return 1;

	   }
    }

    /* Print Disconnection Notice */
    if (sign_watch) {
    notice(s_ConnectServ, "\2SIGNOFF\2 %s(%s@%s) has Signed off at %s - %s", u->nick, u->username, u->hostname, u->server->name, cmd);
    }

    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for MODES message to be echoed */
static int cs_user_modes(User *u) {
    int add = 0;
    char *modes;

    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_user_modes");

    if (mode_watch != 1) return -1;

    if (!u) {
        cslog("Changing modes for unknown user: %s", u->nick);
        return -1;
    }
  
    if (!u->modes) return -1; 
    modes = u->modes;

    while (*modes++) {

        switch(*modes) {
            case '+': add = 1;    break;
            case '-': add = 0;    break;
            case 'N':
                if (add) {
                    notice(s_ConnectServ, "\2NetAdmin\2 %s is Now a Network Administrator (+N)", u->nick);
                } else {
                    notice(s_ConnectServ, "\2NetAdmin\2 %s is No Longer a Network Administrator (-N)", u->nick);
                }
                break;
            case 'S':
                if (add) {
                    notice(s_ConnectServ, "\2Services\2 %s is Now a Network Service (+S)", u->nick);
                } else {
                    notice(s_ConnectServ, "\2Services\2 %s is No Longer a Network Service (-S)", u->nick);
                }
                break;
            case 'T':
                if (add) {
                    notice(s_ConnectServ, "\2TechAdmin\2 %s is Now a Network Technical Administrator (+T)", u->nick);
                } else {
                    notice(s_ConnectServ, "\2TechAdmin\2 %s is No Longer a Network Technical Administrator (-T)", u->nick);
                }
                break;
            case 'A':
                if (add) {
                    notice(s_ConnectServ, "\2ServerAdmin\2 %s is Now a Server Administrator on %s (+A)", u->nick, u->server->name);
                } else {
                    notice(s_ConnectServ, "\2ServerAdmin\2 %s is No Longer a Server Administrator on %s (-A)", u->nick, u->server->name);
                }
                break;
            case 'a':
                if (add) {
                    notice(s_ConnectServ, "\2ServicesAdmin\2 %s is Now a Services Administrator (+a)", u->nick);
                } else {
                    notice(s_ConnectServ, "\2ServicesAdmin\2 %s is No Longer a Services Administrator (-a)", u->nick);
                }
                break;
            case 'C':
                if (add) {
                    notice(s_ConnectServ, "\2Co-ServerAdmin\2 %s is Now a Co-Server Administrator on %s (+C)", u->nick, u->server->name);
                } else {
                    notice(s_ConnectServ, "\2Co-ServerAdmin\2 %s is No Longer a Co-Server Administrator on %s (-C)", u->nick, u->server->name);
                }
                break;
            case 'B':
                if (add) {
                    notice(s_ConnectServ, "\2Bot\2 %s is Now a Bot (+B)", u->nick);
                } else {
                    notice(s_ConnectServ, "\2Bot\2 %s is No Longer a Bot (-B)", u->nick);
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
                    notice(s_ConnectServ, "\2Oper\2 %s is Now a Oper on %s (+o)", u->nick, u->server->name);
                } else {
                    notice(s_ConnectServ, "\2Oper\2 %s is No Longer a Oper on %s (-o)", u->nick, u->server->name);
                }
                break;
            case 'O':
                if (add) {
                    notice(s_ConnectServ, "\2Oper\2 %s is Now a Oper on %s (+o)", u->nick, u->server->name);
                } else {
                    notice(s_ConnectServ, "\2Oper\2 %s is No Longer a Oper on %s (-o)", u->nick, u->server->name);
                }
                break;
            default: 
                break; 
        }
    }
    return 1;
}


/* Routine for KILL message to be echoed */
static int cs_user_kill(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_user_kill");

    cmd = sstrdup(recbuf);
    who = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    tmp = strtok(NULL, "");
    cmd++;
    who++;
    if (finduser(who)) {
    /* it was a User who was killed */
        if (kill_watch) notice(s_ConnectServ, "\2GLOBAL KILL\2 %s(%s@%s) was Killed by %s - Reason sighted: \2%s\2",
u->nick, u->username, u->hostname, who, tmp);
    } else if (findserver(who)) {
        if (kill_watch) notice(s_ConnectServ, "\2SERVER KILL\2 %s was Killed by the Server %s - Reason sighted: %s", u->nick, who, cmd);
    }
    return 1;
}


/* Routine for Signon/Signoff ENABLE or DISABLE */
static void cs_signwatch(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_signwatch");
    if (!(UserLevel(u) >= 185)) {
        privmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!sign_watch) {
        sign_watch = 1;
        notice(s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Activated by %s",u->nick);
        cslog("%s!%s@%s Activated SIGNON/SIGNOFF WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Activated");
   } else {
        sign_watch = 0;
        notice(s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Deactivated by %s",u->nick);
        cslog("%s!%s@%s Deactivated SIGNON/SIGNOFF WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_ConnectServ, "\2SIGNON/SIGNOFF WATCH\2 Deactivated");
    }

}


/* Routine for kill watch ENABLE or DISABLE */
static void cs_killwatch(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_killwatch");
    if (!(UserLevel(u) >= 185)) {
        privmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!kill_watch) {
        kill_watch = 1;
        notice(s_ConnectServ, "\2KILL WATCH\2 Activated by %s",u->nick);
        cslog("%s!%s@%s Activated KILL WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 Activated");
   } else {
        kill_watch = 0;
        notice(s_ConnectServ, "\2KILL WATCH\2 Deactivated by %s",u->nick);
        cslog("%s!%s@%s Deactivated KILL WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 Deactivated");
    }

}


/* Routine for mode watch ENABLE or DISABLE */
static void cs_modewatch(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_modewatch");
    if (!(UserLevel(u) >= 185)) {
        privmsg(u->nick, s_ConnectServ, "Permission Denied, you need to be a TechAdmin or a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!mode_watch) {
        mode_watch = 1;
        notice(s_ConnectServ, "\2MODE WATCH\2 Activated by %s",u->nick);
        cslog("%s!%s@%s Activated MODE WATCH", u->nick, u->username, u->hostname);
        SaveSettings();    
        privmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 Activated");
   } else {
        mode_watch = 0;
        notice(s_ConnectServ, "\2MODE WATCH\2 Deactivated by %s",u->nick);
        cslog("%s!%s@%s Deactivated MODE WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 Deactivated");
    }

}


/* Routine for STATUS echo */
static void cs_status(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("cs_status");

    privmsg(u->nick, s_ConnectServ, "Current %s Settings:", s_ConnectServ);

    /* SIGNWATCH Check */
    if (!sign_watch) {
        privmsg(u->nick, s_ConnectServ, "\2SIGN WATCH\2 is Not Currently Active");
    }
    if (sign_watch) {   
        privmsg(u->nick, s_ConnectServ, "\2SIGN WATCH\2 is Currently Active");
    }

    /* KILLWATCH Check */
    if (!kill_watch) {
        privmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 is Not Currently Active");
    }
    if (kill_watch) {   
        privmsg(u->nick, s_ConnectServ, "\2KILL WATCH\2 is Currently Active");
    }

    /* MODEWATCH Check */
    if (!mode_watch) {
        privmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 is Not Currently Active");
    }
    if (mode_watch) {   
        privmsg(u->nick, s_ConnectServ, "\2MODE WATCH\2 is Currently Active");
    }

}

/* Load ConnectServ Config file and set defaults if does not exist */
void Loadconfig()
{
    FILE *fp = fopen("data/connect.db", "r");
    char buf[BUFSIZE];
    char *tmp;
    segv_location = sstrdup("cs_Loadconfig");

	if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);
            tmp = strtok(buf, " ");

            if (!strcasecmp(tmp, "SIGNWATCH")) {
                sign_watch = atoi(strtok(NULL, " "));
            } else if (!strcasecmp(tmp, "KILLWATCH")) {
                                kill_watch = atoi(strtok(NULL, " "));
            } else if (!strcasecmp(tmp, "MODEWATCH")) {
                                mode_watch = atoi(strtok(NULL, " "));
            } else {
                cslog("%s is not a valid connect.db option!",tmp);
                notice(s_Services, "%s is not a valid connect.db option! Please check your data/connect.db file!",tmp);
            }
    }
        fclose(fp);
    } else {
        notice(s_Services, "No Database Found! Creating one with Defaults!");
        sign_watch=1;
        kill_watch=1;
        mode_watch=1; 
    }
}
