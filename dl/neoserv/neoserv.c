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
** $Id: neoserv.c,v 1.8 2003/01/21 13:15:34 fishwaldo Exp $
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
static int new_m_version(char *origin, char **av, int ac);

Module_Info my_info[] = { {
    "NeoServ",
    "Network NeoStats Help Service",
    "1.3"
} };


int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module NeoServ Loaded, Version: %s %s %s",my_info[0].module_version,neoservversion_date,neoservversion_time);
    return 0;
}


Functions my_fn_list[] = {
        { MSG_VERSION,  new_m_version,  1 },
#ifdef HAVE_TOKEN_SUP
        { TOK_VERSION,  new_m_version,  1 },
#endif
	{ NULL,        NULL,        0 }
};


int __Bot_Message(char *origin, char **av, int ac)
{
    User *u;
    u = finduser(origin);

    if (!strcasecmp(av[1], "HELP")) {
        if (ac <= 2) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC1")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic1);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC2")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic2);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC3")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic3);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC4")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic4);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC5")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic5);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC6")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic6);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC7")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic7);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC8")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic8);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC9")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic9);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC10")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic10);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC11")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic11);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC12")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic12);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC13")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic13);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC14")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic14);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC15")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic15);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC16")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic16);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC17")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic17);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC18")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic18);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC19")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic19);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC20")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic20);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC21")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic21);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC22")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic22);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC23")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic23);
            return 1;
        } else if (!strcasecmp(av[2], "TOPIC24")) {
            privmsg_list(u->nick, s_NeoServ, neoserv_help_topic24);
            return 1;
        } else 
            privmsg(u->nick, s_NeoServ, "Unknown Help Topic: \2%s\2", av[2]);
        }

	if ((fnmatch(strlower(av[1]), "viewlog", 0) != 0) || (fnmatch(strlower(av[1]), "send", 0) != 0) || (fnmatch(strlower(av[1]), "logbackup", 0) != 0)) {
		neoservlog("%s requested: %s", u->nick, av[1]);
    }

    if (!strcasecmp(av[1], "TOPIC1")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic1);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC2")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic2);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC3")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic3);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC4")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic4);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC5")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic5);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC6")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic6);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC7")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic7);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC8")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic8);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC9")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic9);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC10")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic10);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC11")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic11);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC12")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic12);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC13")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic13);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC14")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic14);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC15")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic15);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC16")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic16);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC17")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic17);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC18")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic18);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC19")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic19);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC20")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic20);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC21")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic21);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC22")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic22);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC23")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic23);
        return 1;
    } else if (!strcasecmp(av[1], "TOPIC24")) {
        privmsg_list(u->nick, s_NeoServ, neoserv_help_topic24);
        return 1;
    } else if (!strcasecmp(av[1], "VIEWLOG") && (UserLevel(u) >= 185)) {
               neoserv_viewlog(u);
    } else if (!strcasecmp(av[1], "LOGBACKUP") && (UserLevel(u) >= 185)) {
               neoserv_logbackup(u);
    } else if (!strcasecmp(av[1], "SEND") && (UserLevel(u) >= 185)) {
               if (ac < 4) {
                   privmsg(u->nick, s_NeoServ, "Syntax: /msg %s SEND <NICK> <TOPIC#>", s_NeoServ);
                   privmsg(u->nick, s_NeoServ, "For addtional help: /msg %s HELP", s_NeoServ);
                   return -1;
               }
               neoserv_send(u, av[2], av[3]);
    } else {
		privmsg(u->nick, s_NeoServ, "%s Unknown To The NeoStats Help System... please /msg %s HELP", av[1], s_NeoServ);
    }
    return 1;
}


int Online(char **av, int ac) {
    if (init_bot(s_NeoServ,"NeoServ",me.name,"Network NeoStats Help Service", "+Sqd-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use!!!! */
        s_NeoServ = strcat(s_NeoServ, "_");
        init_bot(s_NeoServ,"NeoServ",me.name,"Network NeoStats Help Service", "+Sqd-x", my_info[0].module_name);
    }
#ifdef ULTIMATE3
    sjoin_cmd(s_NeoServ, "#NeoStats", MODE_CHANOP);
#else
    sjoin_cmd(s_NeoServ, "#NeoStats");
#endif
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
}


void _fini() {
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
   strcpy(segv_location, "neoserv_send");

    /* Topic Sanity Check */

/*   if ((!strstr(m, "topic")) && (!strstr(m, "TOPIC"))) { 
   privmsg(u->nick, s_NeoServ, "The topic you attempted to send is not vaild and was therefore not sent.");
   return;
   }
*/

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

    strcpy(segv_location, "neoserv_viewlog");
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
	
	strcpy(segv_location, "neoserv_logbackup");
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
