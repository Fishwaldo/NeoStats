/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module: NeoServ
** Description: Network Village Idiot Help System
** Version: 1.2
** Date:    28/12/2001
** Author: ^Enigma^
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"
#include "neoserv_help.c"

const char neoservversion_date[] = __DATE__;
const char neoservversion_time[] = __TIME__;
char *s_NeoServ;
extern const char *neoserv_help[];
static void neoserv_send(User *u, char *cmd, char *m);
void neoservlog(char *, ...);
static void neoserv_viewlog(User *u);
static void neoserv_logbackup(User *u);

Module_Info my_info[] = { {
    "NeoServ",
    "Network NeoStats Help Service",
    "1.2"
} };


int new_m_version(char *av, char *tmp) {
    sts(":%s 351 %s :Module NeoServ Loaded, Version %s %s %s",me.name, av,my_info[0].module_version,neoservversion_date,neoservversion_time);
    return 0;
}

Functions my_fn_list[] = {
    { "VERSION",    new_m_version,    1 },
    { NULL,        NULL,        0 }
};


int __Bot_Message(char *origin, char *coreLine, int type)
{
    User *u;
    char *cmd;
/*    char *tmp; */

	u = finduser(origin);

    if (coreLine == NULL) return -1;
    cmd = strtok(coreLine, " ");
/*    strcpy(cmd, tmp); */
/*    neoservlog("%s %s", cmd, tmp); */

    if (!strcasecmp(cmd, "HELP")) {
        coreLine = strtok(NULL, " ");

        if (coreLine) {
        neoservlog("%s requested: %s", u->nick, coreLine);
        }

        if (!coreLine) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC1")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic1);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC2")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic2);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC3")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic3);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC4")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic4);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC5")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic5);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC6")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic6);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC7")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic7);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC8")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic8);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC9")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic9);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC10")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic10);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC11")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic11);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC12")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic12);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC13")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic13);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC14")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic14);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC15")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic15);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC16")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic16);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC17")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic17);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC18")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic18);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC19")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic19);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC20")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic20);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC21")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic21);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC22")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic22);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC23")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic23);
            return 1;
        } else if (!strcasecmp(coreLine, "TOPIC24")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic24);
            return 1;
		} else 
            privmsg(u->nick, s_NeoServ, "\2%s\2 is An Unknown Help Topic. Please  /msg %s HELP", coreLine, s_NeoServ);
        }

/*	if ((strlower(tmp) != strlower("VIEWLOG")) || (strlower(tmp) != strlower("SEND")) || (strlower(tmp) != strlower("LOGBACKUP"))) { */
/*	if ((strlower(cmd) != strlower("VIEWLOG")) || (strlower(cmd) != strlower("SEND")) || (strlower(cmd) != strlower("LOGBACKUP"))) { */
/*  if ((strcasecmp(cmd, "VIEWLOG")) || (strcasecmp(cmd, "SEND")) || (strcasecmp(cmd, "LOGBACKUP"))) { */
	if ((fnmatch(strlower(cmd), "viewlog", 0) != 0) || (fnmatch(strlower(cmd), "send", 0) != 0) || (fnmatch(strlower(cmd), "logbackup", 0) != 0)) {
		neoservlog("%s requested: %s", u->nick, cmd);
    }
	

    if (!strcasecmp(cmd, "TOPIC1")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic1);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC2")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic2);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC3")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic3);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC4")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic4);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC5")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic5);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC6")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic6);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC7")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic7);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC8")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic8);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC9")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic9);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC10")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic10);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC11")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic11);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC12")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic12);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC13")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic13);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC14")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic14);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC15")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic15);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC16")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic16);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC17")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic17);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC18")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic18);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC19")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic19);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC20")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic20);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC21")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic21);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC22")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic22);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC23")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic23);
        return 1;
    } else if (!strcasecmp(cmd, "TOPIC24")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic24);
        return 1;
    } else if (!strcasecmp(cmd, "VIEWLOG") && (UserLevel(u) >= 185)) {
               neoserv_viewlog(u);
    } else if (!strcasecmp(cmd, "LOGBACKUP") && (UserLevel(u) >= 185)) {
               neoserv_logbackup(u);
    } else if (!strcasecmp(cmd, "SEND") && (UserLevel(u) >= 185)) {
               char *m;
               cmd = strtok(NULL, " ");
               m = strtok(NULL, " ");
               neoserv_send(u, cmd, m);
    } else {
        privmsg(u->nick, s_NeoServ, "%s Unknown To The NeoStats Help System... please /msg %s HELP", cmd, s_NeoServ);
    }
    return 1;
}


int Online(Server *data) {

    if (init_bot(s_NeoServ,"NeoServ",me.name,"Network NeoStats Help Service", "+Sqd-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use!!!! */
        s_NeoServ = strcat(s_NeoServ, "_");
        init_bot(s_NeoServ,"NeoServ",me.name,"Network NeoStats Help Service", "+Sqd-x", my_info[0].module_name);
    }

    sts(":%s JOIN #NeoStats",s_NeoServ);
    sts(":%s MODE #NeoStats +o %s",me.name, s_NeoServ);
    return 1;
};


EventFnList my_event_list[] = {
    { "ONLINE",     Online},
    { NULL,     NULL}
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
    s_NeoServ = "NeoServ";
    sts(":%s GLOBOPS :NeoServ Help System Module Loaded", me.name);
}


void _fini() {
    sts(":%sGLOBOPS :NeoServ Help System Module Unloaded", me.name);

};


/* Routine For Logging With The 'neoservlog' */
void neoservlog(char *fmt, ...)
{
        va_list ap;
        FILE *neoservfile = fopen("logs/NeoServ.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!neoservfile) {
        log("Unable to open logs/NeoServ.log for writing.");
        return;
        }

    fprintf(neoservfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(neoservfile);

}


/* Routine for SEND */
static void neoserv_send(User *u, char *cmd, char *m) {

    if (!cmd) {
       privmsg(u->nick, s_NeoServ, "Syntax: /msg %s SEND <NICK> <TOPIC#>", s_NeoServ);
       privmsg(u->nick, s_NeoServ, "For addtional help: /msg %s HELP", s_NeoServ);
       return;
   }
   if (!m) {
      privmsg(u->nick, s_NeoServ, "Syntax: Syntax: /msg %s SEND <NICK> <TOPIC#>", s_NeoServ);
      privmsg(u->nick, s_NeoServ, "For addtional help: /msg %s HELP", s_NeoServ);
      return;
   }
   /* The user has passed the minimum requirements for input */
    
   segv_location = sstrdup("neoserv_send");

    /* If The User Is Online Send The Correct Topic*/        
    if (finduser(cmd)) {
       privmsg(u->nick, s_NeoServ, "%s was sent to %s", m, cmd);
       neoservlog("%s was sent to %s", m, cmd);

       u = finduser(cmd);
       privmsg(u->nick, s_NeoServ, "%s, Regarding your NeoStats question, this topic of help seemed to be very relevant to you:", cmd);

       if (!strcasecmp(m, "TOPIC1")) {
          privmsg_list(u->nick, s_NeoServ, neoserv_help_topic1);
          return;
       } else if (!strcasecmp(m, "TOPIC2")) {
          privmsg_list(u->nick, s_NeoServ, neoserv_help_topic2);
          return;
       } else if (!strcasecmp(m, "TOPIC3")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic3);
           return;
       } else if (!strcasecmp(m, "TOPIC4")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic4);
           return;
       } else if (!strcasecmp(m, "TOPIC5")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic5);
           return;
       } else if (!strcasecmp(m, "TOPIC6")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic6);
           return;
       } else if (!strcasecmp(m, "TOPIC7")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic7);
           return;
       } else if (!strcasecmp(m, "TOPIC8")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic8);
           return;
       } else if (!strcasecmp(m, "TOPIC9")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic9);
           return;
       } else if (!strcasecmp(m, "TOPIC10")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic10);
           return;
       } else if (!strcasecmp(m, "TOPIC11")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic11);
           return;
       } else if (!strcasecmp(m, "TOPIC12")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic12);
           return;
       } else if (!strcasecmp(m, "TOPIC13")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic13);
           return;
       } else if (!strcasecmp(m, "TOPIC14")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic14);
           return;
       } else if (!strcasecmp(m, "TOPIC15")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic15);
           return;
       } else if (!strcasecmp(m, "TOPIC16")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic16);
           return;
       } else if (!strcasecmp(m, "TOPIC17")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic17);
           return;
       } else if (!strcasecmp(m, "TOPIC18")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic18);
           return;
       } else if (!strcasecmp(m, "TOPIC19")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic19);
           return;
       } else if (!strcasecmp(m, "TOPIC20")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic20);
           return;
       } else if (!strcasecmp(m, "TOPIC21")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic21);
           return;
       } else if (!strcasecmp(m, "TOPIC22")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic22);
           return;
       } else if (!strcasecmp(m, "TOPIC23")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic23);
           return;
       } else if (!strcasecmp(m, "TOPIC24")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help_topic24);
           return;
	   } else if (!strcasecmp(m, "HELP")) {
           privmsg_list(u->nick, s_NeoServ, neoserv_help);
           return;
       }
    }
    return;
}


/* Routine for VIEWLOG - Printing Of The NeoServ Logfile */
static void neoserv_viewlog(User *u)
{
    FILE *fp;
    char buf[512];

    segv_location = sstrdup("neoserv_viewlog");
    if (!(UserLevel(u) >= 185)) {
        neoservlog("Access Denied (VIEWLOG) to %s", u->nick);
        privmsg(u->nick, s_NeoServ, "Access Denied.");
        return;
    }

    fp = fopen("logs/NeoServ.log", "r");
    if (!fp) {
        privmsg(u->nick, s_NeoServ, "Unable to open NeoServ.log");
        return;
    }

    neoservlog("%s viewed %s's logs", u->nick, s_NeoServ);
    
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';
        privmsg(u->nick, s_NeoServ, "%s", buf);
    }
    fclose(fp);
}


/* Routine for NeoServ to conduct a LOGBACKUP */
static void neoserv_logbackup(User *u)
{
	char tmp[27];
	time_t t = time(NULL);
	
	segv_location = sstrdup("neoserv_logbackup");
	if (!(UserLevel(u) >= 185)) {
		neoservlog("Access Denied (LOGBACKUP) from %s", u->nick);
		privmsg(u->nick, s_NeoServ, "Access Denied.");
		return;
	}
	/* The user has passed the minimum requirements for RESET */

	neoservlog("%s ended this log session with a LOGBACKUP %s's logs",u->nick, s_NeoServ, tmp);	
	strftime(tmp, 27, "logs/NeoServ-%m-%d.log", localtime(&t));
	rename("logs/NeoServ.log", tmp);
	neoservlog("%s created a LOGBACKUP %s's logs - %s",u->nick, s_NeoServ, tmp);
        privmsg(u->nick, s_NeoServ, "%s has sucessfuly had a LOGBACKUP occur", s_NeoServ);
}
