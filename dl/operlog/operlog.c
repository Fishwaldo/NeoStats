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
** $Id: operlog.c,v 1.3 2002/09/04 08:40:29 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "ol_help.c"

const char olversion_date[] = __DATE__;
const char olversion_time[] = __TIME__;
char *s_OperLog;

extern const char *ol_help[];
static int ol_user_modes(User *);
static int ol_del_user(User *);
static int ol_user_kill(User *);
static int ol_chatops(User *);
static int ol_wallops(User *);
static int ol_nachat(User *);
static int ol_adchat(User *);
static int ol_globops(User *);

static void ol_status(User *);
static void ol_viewlog(User *u);
static void ol_reset(User *u);
static void ol_chatwatch(User *u);
static void ol_killwatch(User *u);
static void ol_modewatch(User *u);

int chat_watch;
int kill_watch;
int mode_watch;

void ollog(char *, ...);
void SaveSettings();
void Loadconfig();


Module_Info my_info[] = { {
    "OperLog",
    "Network Command Logging Service",
    "1.0"
} };

int new_m_version(char *av, char *tmp) {
    sts(":%s 351 %s :Module OperLog Loaded, Version: %s %s %s",me.name,av,my_info[0].module_version,olversion_date,olversion_time);
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
            privmsg(u->nick, s_OperLog, "Permission Denied, you need to be a Network Administrator to do that!");
            return 1;
        } else if (!coreLine && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_OperLog, ol_help); 
            return 1;
        } else if (!strcasecmp(coreLine, "CHATWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_OperLog, ol_help_chatwatch);
            return 1;
        } else if (!strcasecmp(coreLine, "KILLWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_OperLog, ol_help_killwatch);
            return 1;
        } else if (!strcasecmp(coreLine, "MODEWATCH") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_OperLog, ol_help_modewatch);
            return 1;
        } else if (!strcasecmp(coreLine, "STATUS")) {
            privmsg_list(u->nick, s_OperLog, ol_help_status);
            return 1;
        } else if (!strcasecmp(coreLine, "VIEWLOG")) {
            privmsg_list(u->nick, s_OperLog, ol_help_viewlog);
            return 1;		
        } else if (!strcasecmp(coreLine, "RESET")) {
            privmsg_list(u->nick, s_OperLog, ol_help_reset);
            return 1;
		} else if (!strcasecmp(coreLine, "ABOUT")) {
                privmsg_list(u->nick, s_OperLog, ol_help_about);
            return 1;        
        } else 
            privmsg(u->nick, s_OperLog, "Unknown Help Topic: %s",coreLine);
        }

    if (!strcasecmp(cmd, "ABOUT")) {
                privmsg_list(u->nick, s_OperLog, ol_help_about);
    } else if (!strcasecmp(cmd, "CHATWATCH") && (UserLevel(u) >= 185)) {
                ol_chatwatch(u);
    } else if (!strcasecmp(cmd, "KILLWATCH") && (UserLevel(u) >= 185)) {
                ol_killwatch(u);
    } else if (!strcasecmp(cmd, "MODEWATCH") && (UserLevel(u) >= 185)) {
                ol_modewatch(u);
    } else if (!strcasecmp(cmd, "STATUS")) {
                ol_status(u);
    } else if (!strcasecmp(cmd, "VIEWLOG")) {
                ol_viewlog(u);	
    } else if (!strcasecmp(cmd, "RESET")) {
                ol_reset(u);
	} else {
        privmsg(u->nick, s_OperLog, "Unknown Command: %s, perhaps you need some HELP?",cmd);
    }
    return 1;

}

int Online(Server *data) {

    if (init_bot(s_OperLog,"OperLog",me.name,"Network Command Logging Service", "+oikSNdwgleq-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_OperLog = strcat(s_OperLog, "_");
        init_bot(s_OperLog,"OperLog",me.name,"Network Command Logging Service", "+oikSNdwgleq-x", my_info[0].module_name);
    }
    return 1;
};


EventFnList my_event_list[] = {
    { "ONLINE", Online},
    { "UMODE", ol_user_modes},
    { "KILL", ol_user_kill},
    { "SIGNOFF", ol_del_user},
    { "CHATOPS", ol_chatops},
    { "WALLOPS", ol_wallops},
    { "NACHAT", ol_nachat},
    { "ADCHAT", ol_adchat},
    { "GLOBOPS", ol_globops},
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

    s_OperLog = "OperLog";
    sts(":%s GLOBOPS :OperLog Module Loaded",me.name);

         for (i=0; i < U_TABLE_SIZE; i++) {
              for (u = userlist[i]; u; u = u->next) {
              }
         }
}

void _fini() {
    sts(":%s GLOBOPS :OperLog Module Unloaded",me.name);

};


/* Routine for logging items with the 'ollog' */
void ollog(char *fmt, ...)
{
        va_list ap;
        FILE *olfile = fopen("logs/Operators.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!olfile) {
        log("Unable to open logs/Operators.log for writing.");
        return;
        }

    fprintf(olfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(olfile);

}


/* Routine for saving settings to data/OperLog.db */
void SaveSettings()
{
        FILE *fp = fopen("data/OperLog.db", "w");
        fprintf(fp, "CHATWATCH %i\n", chat_watch);
        fprintf(fp, "KILLWATCH %i\n", kill_watch);
        fprintf(fp, "MODEWATCH %i\n", mode_watch);
        fclose(fp);
}


/* Routine for CHATOPS to be logged */
static int ol_chatops(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_chatops");

	cmd = sstrdup(recbuf);
ollog("CHATOPS Buffer: %s", cmd);	
	who = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    tmp = strtok(NULL, "");
    cmd++;
    who++;

    /* Log Operator Chat */
    if (chat_watch) {
    ollog("CHATOPS by %s: %s", who, tmp);
    }
    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for WALLOPS to be logged */
static int ol_wallops(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_wallops");

	cmd = sstrdup(recbuf);
ollog("WALLOPS Buffer: %s", cmd);
	who = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    tmp = strtok(NULL, "");
    cmd++;
    who++;

    /* Log Operator Chat */
    if (chat_watch) {
    ollog("WALLOPS by %s: %s", who, tmp);
    }
    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for NACHAT to be logged */
static int ol_nachat(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_nachat");

	cmd = sstrdup(recbuf);
ollog("NACHAT Buffer: %s", cmd);
	who = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    tmp = strtok(NULL, "");
    cmd++;
    who++;

    /* Log Operator Chat */
    if (chat_watch) {
    ollog("NACHAT by %s: %s", who, tmp);
    }
    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for ADCHAT to be logged */
static int ol_adchat(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_adchat");

	cmd = sstrdup(recbuf);
ollog("ADCHAT Buffer: %s", cmd);
	who = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    tmp = strtok(NULL, "");
    cmd++;
    who++;

    /* Log Operator Chat */
    if (chat_watch) {
    ollog("ADCHAT by %s: %s", who, tmp);
    }
    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for GLOBOPS to be logged */
static int ol_globops(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_globops");

	cmd = sstrdup(recbuf);
ollog("GLOBOPS Buffer: %s", cmd);
	who = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, " ");
    tmp = strtok(NULL, "");
    cmd++;
    who++;

    /* Log Operator Chat */
    if (chat_watch) {
    ollog("GLOBOPS by %s: %s", who, tmp);
    }
    if (findbot(u->nick)) return 1;
  return 1;
}


/* Routine for SIGNOFF message to be echoed */
static int ol_del_user(User *u) {
    char *cmd, *lcl, *who;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_del_user");

    cmd = sstrdup(recbuf);
    lcl = sstrdup(recbuf);
    cmd = strtok(cmd, " ");
    cmd = strtok(NULL, " ");
    cmd = strtok(NULL, "");
    cmd++;

    if (findbot(u->nick)) return 1;

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

          ollog("LOCAL KILL %s(%s@%s) was Killed by %s - Reason sighted: %s", u->nick, u->username, u->hostname, who, lcl);
          return 1;

       }
    }

  return 1;
}


/* Routine for MODES message to be echoed */
static int ol_user_modes(User *u) {
    int add = 0;
    char *modes;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_user_modes");

    if (mode_watch != 1) return -1;

    if (!u) {
        ollog("Changing modes for unknown user: %s", u->nick);
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
                    ollog("NetAdmin %s is Now a Network Administrator (+N)", u->nick);
                } else {
                    ollog("NetAdmin %s is No Longer a Network Administrator (-N)", u->nick);
                }
                break;
            case 'S':
                if (add) {
                    ollog("Services %s is Now a Network Service (+S)", u->nick);
                } else {
                    ollog("Services %s is No Longer a Network Service (-S)", u->nick);
                }
                break;
            case 'T':
                if (add) {
                    ollog("TechAdmin %s is Now a Network Technical Administrator (+T)", u->nick);
                } else {
                    ollog("TechAdmin %s is No Longer a Network Technical Administrator (-T)", u->nick);
                }
                break;
            case 'a':
                if (add) {
                    ollog("ServicesAdmin %s is Now a Services Administrator (+a)", u->nick);
                } else {
                    ollog("ServicesAdmin %s is No Longer a Services Administrator (-a)", u->nick);
                }
                break;
            case 'I':
                if (add) {
                   ollog("%s Is Using Invisible Mode (+I)",u->nick);
                } else {
                   ollog("%s Is no longer using Invisible Mode (-I)",u->nick);
                }
                break;
            default: 
                break; 
        }
    }
    return 1;
}


/* Routine for KILL message to be echoed */
static int ol_user_kill(User *u) {
    char *cmd, *who, *tmp;

    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_user_kill");

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
        if (kill_watch) ollog("GLOBAL KILL %s(%s@%s) was Killed by %s - Reason sighted: %s", u->nick, u->username, u->hostname, who, tmp);
    } else if (findserver(who)) {
        if (kill_watch) ollog("SERVER KILL %s was Killed by the Server %s - Reason sighted: %s", u->nick, who, cmd);
    }
    return 1;
}


/* Routine for Signon/Signoff ENABLE or DISABLE */
static void ol_chatwatch(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_chatwatch");
    if (!(UserLevel(u) >= 185)) {
        privmsg(u->nick, s_OperLog, "Permission Denied, you need to be a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!chat_watch) {
        chat_watch = 1;
        ollog("%s!%s@%s Activated CHAT WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_OperLog, "CHAT WATCH Activated");
   } else {
        chat_watch = 0;
        ollog("%s!%s@%s Deactivated CHAT WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_OperLog, "CHAT WATCH Deactivated");
    }

}


/* Routine for kill watch ENABLE or DISABLE */
static void ol_killwatch(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_killwatch");
    if (!(UserLevel(u) >= 185)) {
        privmsg(u->nick, s_OperLog, "Permission Denied, you need to be a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!kill_watch) {
        kill_watch = 1;
        ollog("%s!%s@%s Activated KILL WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_OperLog, "KILL WATCH Activated");
   } else {
        kill_watch = 0;
        ollog("%s!%s@%s Deactivated KILL WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_OperLog, "KILL WATCH Deactivated");
    }

}


/* Routine for mode watch ENABLE or DISABLE */
static void ol_modewatch(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_modewatch");
    if (!(UserLevel(u) >= 185)) {
        privmsg(u->nick, s_OperLog, "Permission Denied, you need to be a Network Administrator to do that!");
        return;
    }
    /* The user has passed the minimum requirements for the ENABLE/DISABLE */

    if (!mode_watch) {
        mode_watch = 1;
        ollog("%s!%s@%s Activated MODE WATCH", u->nick, u->username, u->hostname);
        SaveSettings();    
        privmsg(u->nick, s_OperLog, "MODE WATCH Activated");
   } else {
        mode_watch = 0;
        ollog("%s!%s@%s Deactivated MODE WATCH", u->nick, u->username, u->hostname);
        SaveSettings();
        privmsg(u->nick, s_OperLog, "MODE WATCH Deactivated");
    }

}


/* Routine for STATUS echo */
static void ol_status(User *u)
{
    /* Approximate Segfault Location */
    segv_location = sstrdup("ol_status");

    privmsg(u->nick, s_OperLog, "Current %s Settings:", s_OperLog);

    /* CHATWATCH Check */
    if (!chat_watch) {
        privmsg(u->nick, s_OperLog, "CHAT WATCH is Not Currently Active");
    }
    if (chat_watch) {   
        privmsg(u->nick, s_OperLog, "CHAT WATCH is Currently Active");
    }

    /* KILLWATCH Check */
    if (!kill_watch) {
        privmsg(u->nick, s_OperLog, "KILL WATCH is Not Currently Active");
    }
    if (kill_watch) {   
        privmsg(u->nick, s_OperLog, "KILL WATCH is Currently Active");
    }

    /* MODEWATCH Check */
    if (!mode_watch) {
        privmsg(u->nick, s_OperLog, "MODE WATCH is Not Currently Active");
    }
    if (mode_watch) {   
        privmsg(u->nick, s_OperLog, "MODE WATCH is Currently Active");
    }

}

/* Load OperLog Config file and set defaults if does not exist */
void Loadconfig()
{
    FILE *fp = fopen("data/OperLog.db", "r");
    char buf[BUFSIZE];
    char *tmp;
    segv_location = sstrdup("ol_Loadconfig");

    if (fp) {
        while (fgets(buf, BUFSIZE, fp)) {
            strip(buf);
            tmp = strtok(buf, " ");

            if (!strcasecmp(tmp, "CHATWATCH")) {
                chat_watch = atoi(strtok(NULL, " "));
            } else if (!strcasecmp(tmp, "KILLWATCH")) {
                                kill_watch = atoi(strtok(NULL, " "));
            } else if (!strcasecmp(tmp, "MODEWATCH")) {
                                mode_watch = atoi(strtok(NULL, " "));
            } else {
                ollog("%s is not a valid OperLog.db option!",tmp);
                chanalert(s_Services, "%s is not a valid OperLog.db option! Please check your data/OperLog.db file!",tmp);
            }
    }
        fclose(fp);
    } else {
        chanalert(s_Services, "No Database For %s Found! Creating one with Defaults!", s_OperLog);
        chat_watch=1;
        kill_watch=1;
        mode_watch=1; 
    }
}


/* Print "OperLog's" logs to the user */
static void ol_viewlog(User *u)
{
	FILE *fp;
	char buf[512];

	segv_location = sstrdup("ol_viewlog");
	if (!(UserLevel(u) >= 185)) {
		ollog("Access Denied to %s (VIEWLOG)", u->nick);
		privmsg(u->nick, s_OperLog, "Access Denied.");
		return;
	}
	fp = fopen("logs/Operators.log", "r");
	if (!fp) {
		privmsg(u->nick, s_OperLog, "Unable to open logs/Operators.log");
		return;
	}
	while (fgets(buf, sizeof(buf), fp)) {
		buf[strlen(buf)] = '\0';
		privmsg(u->nick, s_OperLog, "%s", buf);
	}
	fclose(fp);
	ollog("%s viewed OperLog's logs", u->nick);
}


/* Routine for OperLog to reset it's logs */
static void ol_reset(User *u)
{
	char tmp[27];
	time_t t = time(NULL);
	
	segv_location = sstrdup("ol_reset");
	if (!(UserLevel(u) >= 185)) {
		ollog("Access Denied to %s (RESET)", u->nick);
		privmsg(u->nick, s_OperLog, "Access Denied.");
		return;
	}
	/* The user has passed the minimum requirements for RESET */

	ollog("%s ended this log session with a RESET %s's logs",u->nick, s_OperLog, tmp);	
	strftime(tmp, 27, "logs/Operators-%m-%d.log", localtime(&t));
	rename("logs/Operators.log", tmp);
	ollog("%s RESET %s's logs - %s",u->nick, s_OperLog, tmp);
	privmsg(u->nick, s_OperLog, "%s has sucessfuly RESET it's log file.", s_OperLog);

}
