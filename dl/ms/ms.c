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
** $Id: ms.c,v 1.8 2002/09/04 08:40:28 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "ms_help.c"

const char msversion_date[] = __DATE__;
const char msversion_time[] = __TIME__;
char *s_MoraleServ;
extern const char *ms_help[];
static void ms_hail(User *u, char *cmd, char *m);
static void ms_ode(User *u, char *cmd, char *m);
static void ms_lapdance(User *u, char *cmd);
static void ms_join(User *u, char *);
static void ms_viewlog(User *u);
static void ms_version(User *u);
static void ms_poem(User *u, char *cmd, char *m);
static void ms_swhois(User *u, char *cmd, char *m);
static void ms_redneck(User *u, char *cmd);
static void ms_svsnick(User *u, char *cmd, char *m);
static void ms_part(User *u, char *);
static void ms_msg(User *u, char *cmd, char *m);
static void ms_loveservlogs(User *u);
static void ms_cheerup(User *u, char *cmd);
static void ms_svsjoin(User *u, char *cmd, char *m);
static void ms_svspart(User *u, char *cmd, char *m);
static void ms_kick(User *u, char *cmd, char *m);
static void ms_behappy(User *u, char *cmd);
static void ms_wonderful(User *u, char *cmd);
static void ms_reset(User *u);
static void ms_logbackup(User *u);
static void ms_printfile(User *u, char *cmd);
static int new_m_version(char *origin, char **av, int ac);

void mslog(char *, ...);
void lovelogs(char *, ...);

Module_Info my_info[] = { {
    "MoraleServ",
    "A Network Morale Service",
    "2.18"
} };


int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module MoraleServ Loaded, Version: %s %s %s",my_info[0].module_version,msversion_date,msversion_time);
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
    char *cmd;
    u = finduser(origin);

    if (!strcasecmp(av[1], "HELP")) {
        if (ac <= 2 && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_tech);
            return 1;
        } else if (ac <= 2) {
            privmsg_list(u->nick, s_MoraleServ, ms_help);
            return 1;
        } else if (!strcasecmp(av[2], "HAIL")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_hail);
            return 1;
        } else if (!strcasecmp(av[2], "ODE")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_ode);
            return 1;
        } else if (!strcasecmp(av[2], "LAPDANCE")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_lapdance);
            return 1;
        } else if (!strcasecmp(av[2], "JOIN") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_join);
            return 1;
        } else if (!strcasecmp(av[2], "VIEWLOG") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_viewlog);
            return 1;
        } else if (!strcasecmp(av[2], "VERSION")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_version);
            return 1;
        } else if (!strcasecmp(av[2], "POEM")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_poem);
            return 1;
        } else if (!strcasecmp(av[2], "SWHOIS") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_swhois);
            return 1;
        } else if (!strcasecmp(av[2], "REDNECK")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_redneck);
            return 1;
        } else if (!strcasecmp(av[2], "SVSNICK") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_svsnick);
            return 1;
        } else if (!strcasecmp(av[2], "PART") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_part);
            return 1;
        } else if (!strcasecmp(av[2], "MSG")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_msg);
            return 1;
        } else if (!strcasecmp(av[2], "LOVESERVLOGS") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_loveservlogs);
            return 1;
        } else if (!strcasecmp(av[2], "CHEERUP")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_cheerup);        
            return 1;
        } else if (!strcasecmp(av[2], "SVSJOIN") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_svsjoin);
            return 1;
        } else if (!strcasecmp(av[2], "SVSPART") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_svspart);
            return 1;
        } else if (!strcasecmp(av[2], "KICK") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_kick);
            return 1;
        } else if (!strcasecmp(av[2], "BEHAPPY")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_behappy);        
            return 1;
        } else if (!strcasecmp(av[2], "WONDERFUL")) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_wonderful);        
            return 1;
        } else if (!strcasecmp(av[2], "RESET") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_reset);
            return 1;
        } else if (!strcasecmp(av[2], "LOGBACKUP") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_logbackup);
            return 1;
        } else if (!strcasecmp(av[2], "PRINTFILE") && (UserLevel(u) >= 185)) {
            privmsg_list(u->nick, s_MoraleServ, ms_help_printfile);
            return 1;
        } else 
            prefmsg(u->nick, s_MoraleServ, "Unknown Help Topic: \2%s\2", av[2]);
        }

    if (!strcasecmp(av[1], "HAIL")) {
         if (ac < 4) {
             prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s HAIL <WHO TO HAIL> <NICK TO SEND HAIL TO>", s_MoraleServ);
             prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
             return -1;
             }
         ms_hail(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "ODE")) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
                    return -1;
                }
                ms_ode(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "LAPDANCE")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s LAPDANCE <NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
                    return -1;
                }
                ms_lapdance(u, av[2]);
    } else if (!strcasecmp(av[1], "JOIN") && (UserLevel(u) >= 185)) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s JOIN <CHANNEL>",s_MoraleServ);
                    return -1;
                }
                ms_join(u, av[2]);
    } else if (!strcasecmp(av[1], "VIEWLOG") && (UserLevel(u) >= 185)) {
                chanalert(s_Services,"%s Requested to Look at today's %s Log",u->nick,s_MoraleServ);
                ms_viewlog(u);
    } else if (!strcasecmp(av[1], "VERSION")) {
                chanalert(s_Services,"%s Wanted to know the current version information for %s",u->nick,s_MoraleServ);
                ms_version(u);
    } else if (!strcasecmp(av[1], "POEM")) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
                    return -1;
                }
                ms_poem(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "SWHOIS") && (UserLevel(u) >= 185)) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s SWHOIS <NICK> <TEXT>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
                    return -1;
                }
				cmd = joinbuf(av, ac, 3);
                ms_swhois(u, av[2], cmd);
                free(cmd);
    } else if (!strcasecmp(av[1], "REDNECK")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s REDNECK <NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
                    return -1;
                }
				ms_redneck(u, av[2]);
    } else if (!strcasecmp(av[1], "SVSNICK") && (UserLevel(u) >= 185)) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s SVSNICK <OLD NICK> <NEW NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
                    return -1;
                }
				ms_svsnick(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "PART") && (UserLevel(u) >= 185)) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s PART <CHANNEL>",s_MoraleServ);
                    return -1;
                }
				ms_part(u, av[2]);
    } else if (!strcasecmp(av[1], "MSG")) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s MSG <PERSON OR CHAN TO MESSAGE> <MESSAGE>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				cmd = joinbuf(av, ac, 3);
                ms_msg(u, av[2], cmd);
                free(cmd);
    } else if (!strcasecmp(av[1], "LOVESERVLOGS") && (UserLevel(u) >= 185)) {
                chanalert(s_Services,"%s Requested to Look at Loveserv's Logs",u->nick);
                ms_loveservlogs(u);
    } else if (!strcasecmp(av[1], "CHEERUP")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s CHEERUP <NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_cheerup(u, av[2]);
    } else if (!strcasecmp(av[1], "SVSJOIN") && (UserLevel(u) >= 185)) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s SVSJOIN <NICK> <CHANNEL>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_svsjoin(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "SVSPART") && (UserLevel(u) >= 185)) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s SVSPART <NICK> <CHANNEL>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_svspart(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "KICK") && (UserLevel(u) >= 185)) {
                if (ac < 4) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s KICK <CHANNEL> <NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_kick(u, av[2], av[3]);
    } else if (!strcasecmp(av[1], "BEHAPPY")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s BEHAPPY <NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_behappy(u, av[2]);
    } else if (!strcasecmp(av[1], "WONDERFUL")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s WONDERFUL <NICK>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_wonderful(u, av[2]);
    } else if (!strcasecmp(av[1], "RESET") && (UserLevel(u) >= 185)) {
                chanalert(s_Services,"%s Requested %s to be RESET!",u->nick,s_MoraleServ);
                ms_reset(u);
    } else if (!strcasecmp(av[1], "LOGBACKUP") && (UserLevel(u) >= 185)) {
                chanalert(s_Services,"%s Requested %s to conduct a LOGBACKUP!",u->nick,s_MoraleServ);
                ms_logbackup(u);
    } else if (!strcasecmp(av[1], "PRINTFILE") && (UserLevel(u) >= 185)) {
				chanalert(s_Services,"%s Requested the use of the PRINTFILE facility from %s",u->nick,s_MoraleServ);
                if (ac < 3) {
                    prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s PRINTFILE <PATH TO FILE>", s_MoraleServ);
                    prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
					return -1;
                }
				ms_printfile(u, av[2]);
    } else {
        chanalert(s_Services, "%s requested the unknown command of: %s", u->nick, av[1]);
        prefmsg(u->nick, s_MoraleServ, "Unknown Command: \2%s\2, perhaps you need some HELP?", av[1]);
    }
    return 1;


}

int Online(char **av, int ac) {
    if (init_bot(s_MoraleServ,"MoraleServ",me.name,"A Network Morale Service", "+Sqd-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_MoraleServ = strcat(s_MoraleServ, "_");
        init_bot(s_MoraleServ,"MoraleServ",me.name,"A Network Morale Service", "+Sqd-x", my_info[0].module_name);
    }
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
    s_MoraleServ = "MoraleServ";
    globops(me.name, "MoraleServ Network Morale Service Module Loaded",me.name);
}


void _fini() {
    globops(me.name, "MoraleServ Network Morale Service Module Unloaded",me.name);
};


/* Routine for logging items with the 'mslog' */
void mslog(char *fmt, ...)
{
        va_list ap;
        FILE *msfile = fopen("logs/MoraleServ.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!msfile) {
        log("Unable to open logs/MoraleServ.log for writing.");
        return;
    }

    fprintf(msfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(msfile);

}


/* Routine for HAIL */
static void ms_hail(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_hail");
        if (!strcasecmp(m, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(m)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(u->nick, s_MoraleServ, "Your \"HAIL\" song greeting has been sent to %s!",m);
    chanalert(s_Services, "%s Wanted %s to be hailed by sending the song to %s", u->nick,cmd,m);
    prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:",u->nick);
    prefmsg(m, s_MoraleServ, "*sings* Hail to the %s, they're the %s and they need hailing, hail to the %s so you better all hail like crazy...",cmd,cmd,cmd);
    mslog("%s sent a HAIL to the %s song to %s",u->nick,cmd,m);

}


/* Routine for LAPDANCE */
static void ms_lapdance(User *u, char *cmd) {
        strcpy(segv_location, "ms_lapdance");
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(cmd, s_MoraleServ, "*%s Seductively walks up to %s and gives %s a sly look*",s_MoraleServ,cmd,cmd);
    prefmsg(cmd, s_MoraleServ, "*%s Sits across %s's legs and gives %s the best Lap Dance of their life*",s_MoraleServ,cmd,cmd);
    prefmsg(cmd, s_MoraleServ, "*I Think we both need a cold shower now*... *wink*",s_MoraleServ,cmd);
    chanalert(s_Services, "%s Wanted a LAPDANCE to be preformed on %s", u->nick,cmd);
    mslog("%s Wanted a LAPDANCE to be preformed on %s",u->nick,cmd);

}


/* Routine for ODE */
static void ms_ode(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_ode");
        if (!m) {
            prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>", s_MoraleServ);
            prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
            return;
        }
        if (!strcasecmp(m, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(m)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(u->nick, s_MoraleServ, "Your ODE to %s has been sent to %s!",cmd,m);
    chanalert(s_Services, "%s Wanted an ODE to %s to be recited to %s", u->nick,cmd,m);
    prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:",u->nick);
    prefmsg(m, s_MoraleServ, "*recites*",u->nick);
    prefmsg(m, s_MoraleServ, "How I wish to be a %s,",cmd);
    prefmsg(m, s_MoraleServ, "a %s I would like to be.",cmd);
    prefmsg(m, s_MoraleServ, "For if I was a %s,",cmd);
    prefmsg(m, s_MoraleServ, "I'd watch the network hail thee.",cmd);
    prefmsg(m, s_MoraleServ, "*bows*",u->nick);
    mslog("%s sent an ODE to %s to be recited to %s",u->nick,cmd,m);

}


/* Routine for 'MoraleServ' to JOIN a channel */
static void ms_join(User *u, char *chan)
{
    strcpy(segv_location, "ms_join");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (JOIN) to %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        chanalert(s_MoraleServ,"%s Requested JOIN, but is not a TechAdmin!",u->nick);
        return;
    }
     /* The user has passed the minimum requirements for input */
    
    prefmsg(me.chan, s_MoraleServ, "%s Asked me to Join %s", u->nick, chan);
    mslog("%s!%s@%s Asked me to Join %s", u->nick, u->username, u->hostname, chan);
    sjoin_cmd(s_MoraleServ, chan);

}


/* Routine for VIEWLOG - printing of today's log file */
static void ms_viewlog(User *u)
{
    FILE *fp;
    char buf[512];

    strcpy(segv_location, "ms_viewlog");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (VIEWLOG) to %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        return;
    }

    fp = fopen("logs/MoraleServ.log", "r");
    if (!fp) {
        prefmsg(u->nick, s_MoraleServ, "Unable to open MoraleServ.log");
        return;
    }

    mslog("%s viewed today's %s's logs", u->nick, s_MoraleServ);
    
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';
        prefmsg(u->nick, s_MoraleServ, "%s", buf);
    }
    fclose(fp);
}


/* Routine for VERSION */
static void ms_version(User *u)
{
        strcpy(segv_location, "ms_version");
        prefmsg(u->nick, s_MoraleServ, "\2%s Version Information\2", s_MoraleServ);
        prefmsg(u->nick, s_MoraleServ, "%s Version: %s - running on: %s", s_MoraleServ, my_info[0].module_version, me.name);
        prefmsg(u->nick, s_MoraleServ, "%s Author: ^Enigma^ <enigma@neostats.net>", s_MoraleServ);
        prefmsg(u->nick, s_MoraleServ, "Neostats Satistical Software: http://www.neostats.net");
}


/* Routine for POEM */
static void ms_poem(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_poem");
        if (!m) {
            prefmsg(u->nick, s_MoraleServ, "Syntax: /msg %s POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>", s_MoraleServ);
            prefmsg(u->nick, s_MoraleServ, "For addtional help: /msg %s HELP", s_MoraleServ);
            return;
        }
        if (!strcasecmp(m, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(m)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(u->nick, s_MoraleServ, "Your POEM about %s has been sent to %s!",cmd,m);
    chanalert(s_Services, "%s Wanted a POEM about %s to be recited to %s", u->nick,cmd,m);
    prefmsg(m, s_MoraleServ, "Courtesy of your friend %s:",u->nick);
    prefmsg(m, s_MoraleServ, "*recites*");
    prefmsg(m, s_MoraleServ, "I wish I was a %s,",cmd);
    prefmsg(m, s_MoraleServ, "A %s is never glum,",cmd);
    prefmsg(m, s_MoraleServ, "Coz how can you be grumpy,");
    prefmsg(m, s_MoraleServ, "When the sun shines out your bum.");
    prefmsg(m, s_MoraleServ, "*bows*");
    mslog("%s sent a POEM about %s to %s",u->nick,cmd,m);

}


/* Routine for SWHOIS */
static void ms_swhois(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_swhois");
        if (!(UserLevel(u) >= 185)) {
            prefmsg(u->nick, s_MoraleServ, "Permission Denied, you need to be a TechAdmin to do that!");
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */
    
    mslog("%s!%s@%s Issued a SWHOIS command - SWHOIS %s :%s", u->nick, u->username, u->hostname, cmd, m);
    chanalert(s_Services, "%s Just used thier TechAdmin-ship to preform a SWHOIS on %s (%s)", u->nick, cmd, m);
    sswhois_cmd(cmd, m);

}


/* Routine for REDNECK */
static void ms_redneck(User *u, char *cmd) {
        strcpy(segv_location, "ms_redneck");
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(u->nick, s_MoraleServ, "Your redneck message has been sent to %s!",cmd);
    chanalert(s_Services, "%s Wanted a REDNECK \"dubbing\" to be preformed on %s", u->nick,cmd);
    prefmsg(cmd, s_MoraleServ, "Courtesy of your friend %s:",u->nick);
    prefmsg(cmd, s_MoraleServ, "*recites*",u->nick);
    prefmsg(cmd, s_MoraleServ, "I dub thee \"Redneck\", May you enjoy your coons and over sexation and many hours of wierd contemplation. If its dead you eat it, ifs living kill it than eat it. This is the redneck way. Country Music all the time no rap no jive no rock no hop this is the redneck way, now go forth into a redneck world and don't forget your boots.",u->nick);
    prefmsg(cmd, s_MoraleServ, "*bows*",u->nick);
    mslog("%s sent a REDNECK \"dubbing\" to %s",u->nick,cmd);

}


/* Routine for SVSNICK */
static void ms_svsnick(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_svsnick");
        if (!(UserLevel(u) >= 185)) {
            prefmsg(u->nick, s_MoraleServ, "Permission Denied, you need to be a TechAdmin to do that!");
            return;
        }
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service change its nick?");
            chanalert(s_Services,"Prevented %s from making %s change its own nick",u->nick,s_MoraleServ);
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */


    mslog("%s!%s@%s Issued a SVSNICK command - SVSNICK %s %s :0", u->nick, u->username, u->hostname, cmd, m);
    chanalert(s_Services, "%s Just used thier TechAdmin-ship to preform a SVSNICK on %s (modified to %s)", u->nick, cmd, m);
    ssvsnick_cmd(cmd, m);

}


/* Routine for 'MoraleServ' to PART of channel */
static void ms_part(User *u, char *chan)
{
    strcpy(segv_location, "ms_part");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (PART) to %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        chanalert(s_MoraleServ,"%s Requested PART, but is not a TechAdmin!",u->nick);
        return;
    }
    /* The user has passed the minimum requirements for input */

    prefmsg(me.chan, s_MoraleServ, "%s Asked me to Part %s", u->nick, chan);
    mslog("%s!%s@%s Asked me to Part %s", u->nick, u->username, u->hostname, chan);
    spart_cmd(s_MoraleServ, chan);

}


/* Routine for 'MoraleServ' to MSG a user/channel */
static void ms_msg(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_msg");
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
/*
        if ((strchr(cmd, '#') != NULL) && (!findchan(cmd))) {
            prefmsg(u->nick, s_MoraleServ, "That channel cannot be found. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        if ((strchr(cmd, '#') == NULL) && (!finduser(cmd))) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
*/
        /* The user has passed the minimum requirements for input */
    
    mslog("%s made %s MSG '%s' to %s",u->nick, s_MoraleServ, m, cmd);
    chanalert(s_MoraleServ, "%s made %s MSG '%s' to %s", u->nick, s_MoraleServ, m, cmd);

    if (UserLevel(u) >= 185) {
        prefmsg(cmd, s_MoraleServ, "%s", m);
        return;
    } else {
        prefmsg(cmd, s_MoraleServ, "%s [message issued by %s]", m, u->nick);
        return;
    }

}


/* Routine for 'MoraleServ' to print out Loveserv's Logs */
static void ms_loveservlogs(User *u)
{
    FILE *fp;
    char buf[512];

    strcpy(segv_location, "ms_lovelogs");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (LOGS) to %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        return;
    }

    fp = fopen("logs/loveserv.log", "r");
    if (!fp) {
        prefmsg(u->nick, s_MoraleServ, "Unable to open logs/loveserv.log");
        return;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';
        prefmsg(u->nick, s_MoraleServ, "%s", buf);
    }
    fclose(fp);

    mslog("%s viewed LoveServ's logs", u->nick);
}


/* Routine for CHEERUP */
static void ms_cheerup(User *u, char *cmd) {
        strcpy(segv_location, "ms_cheerup");
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(cmd, s_MoraleServ, "Cheer up %s .....",cmd);
    prefmsg(cmd, s_MoraleServ, "All of us on the network love you! 3--<--<--<{4@",u->nick);
    chanalert(s_Services, "%s Wanted %s to CHEERUP", u->nick,cmd);
    mslog("%s Wanted %s to CHEERUP",u->nick,cmd);

}


/* Routine for SVSJOINing of a user */
static void ms_svsjoin(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_svsjoin");
        if (!(UserLevel(u) >= 185)) {
            prefmsg(u->nick, s_MoraleServ, "Permission Denied, you need to be a TechAdmin to do that!");
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent.  Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    mslog("%s!%s@%s Issued a SVSJOIN command - SVSJOIN %s %s", u->nick, u->username, u->hostname, cmd, m);
    chanalert(s_Services, "%s Just used thier TechAdmin-ship to preform a SVSJOIN on %s to %s", u->nick, cmd, m);
    ssvsjoin_cmd(cmd, m);

}


/* Routine for SVSPARTing of a user */
static void ms_svspart(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_svspart");
        if (!(UserLevel(u) >= 185)) {
            prefmsg(u->nick, s_MoraleServ, "Permission Denied, you need to be a TechAdmin to do that!");
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    mslog("%s!%s@%s Issued a SVSPART command - SVSPART %s %s", u->nick, u->username, u->hostname, cmd, m);
    chanalert(s_Services, "%s Just used thier TechAdmin-ship to preform a SVSPART on %s to %s", u->nick, cmd, m);
    ssvspart_cmd(cmd, m);

}


/* Routine for a KICK of a user */
static void ms_kick(User *u, char *cmd, char *m) {
        strcpy(segv_location, "ms_kick");
        if (!(UserLevel(u) >= 185)) {
            prefmsg(u->nick, s_MoraleServ, "Permission Denied, you need to be a TechAdmin to do that!");
            return;
        }
        if (!strcasecmp(m, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service kick itself from a channel?");
            chanalert(s_Services,"Prevented %s from making %s kick itself from %s",u->nick,s_MoraleServ,cmd);
            return;
        }
        if (!finduser(m)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        if (!findchan(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That channel cannot be found. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }        
        /* The user has passed the minimum requirements for input */

    mslog("%s Issued a KICK command - KICK %s %s", u->nick, cmd, m);
    chanalert(s_Services, "%s Just used thier TechAdmin-ship to preform a KICK on %s from %s", u->nick, m, cmd);
    skick_cmd(s_MoraleServ, cmd, m, "");

}


/* Routine for BEHAPPY */
static void ms_behappy(User *u, char *cmd) {
        strcpy(segv_location, "ms_behappy");
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(cmd, s_MoraleServ, "%s thinks that your a little sad.....",u->nick);
    prefmsg(cmd, s_MoraleServ, "*starts singing*",u->nick);
    prefmsg(cmd, s_MoraleServ, "Here's a little song I wrote, You might want to sing it note for note",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, " ",u->nick);
    prefmsg(cmd, s_MoraleServ, "In every life we have some trouble, But when you worry you make it Double",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy, Don't Worry - Be Happy now",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy, Don't Worry - Be Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, " ",u->nick);
    prefmsg(cmd, s_MoraleServ, "Ain't got no place to lay your head, Somebody came and took your bed",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, " ",u->nick);
    prefmsg(cmd, s_MoraleServ, "The landlord say your rent is late, He may have to litigate",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy, Look at Me - I'm Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, "Here I give you my phone number, When you worry call me, I make you happy",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, " ",u->nick);
    prefmsg(cmd, s_MoraleServ, "Ain't got not cash, ain't got no style, Ain't got no gal to make you smile",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy",u->nick);
    prefmsg(cmd, s_MoraleServ, " ",u->nick);
    prefmsg(cmd, s_MoraleServ, "'Cause when you worry your face will frown, and that will bring everybody down",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy, Don't Worry, Don't Worry - Don't do it",u->nick);
    prefmsg(cmd, s_MoraleServ, "Be Happy - Put a smile on your face, Don't bring everybody down",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry, it will soon pass, whatever it is",u->nick);
    prefmsg(cmd, s_MoraleServ, "Don't Worry - Be Happy, I'm not worried, I'm happy . . . .",u->nick);

    chanalert(s_Services, "%s Wanted %s to BEHAPPY", u->nick,cmd);
    mslog("%s Wanted %s to BEHAPPY",u->nick,cmd);

}


/* Routine for WONDERFUL */
static void ms_wonderful(User *u, char *cmd) {
        strcpy(segv_location, "ms_wonderful");
        if (!strcasecmp(cmd, s_MoraleServ)) {
            prefmsg(u->nick, s_MoraleServ, "Surley we have better things to do with our time than make a service message itself?");
            chanalert(s_Services,"Prevented %s from making %s message %s",u->nick,s_MoraleServ,s_MoraleServ);
            return;
        }
        if (!finduser(cmd)) {
            prefmsg(u->nick, s_MoraleServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
            return;
        }
        /* The user has passed the minimum requirements for input */

    prefmsg(cmd, s_MoraleServ, "Courtesy of your friend %s:",u->nick);
    prefmsg(cmd, s_MoraleServ, "*starts singing*",u->nick);
    prefmsg(cmd, s_MoraleServ, "So excuse me forgetting but these things I do",u->nick);
    prefmsg(cmd, s_MoraleServ, "You see I've forgotten if they're green or they're blue",u->nick);
    prefmsg(cmd, s_MoraleServ, "Anyway the thing is what I really mean",u->nick);
    prefmsg(cmd, s_MoraleServ, "Yours are the sweetest eyes I've ever seen",u->nick);
    prefmsg(cmd, s_MoraleServ, "And you can tell everybody this is your song",u->nick);
    prefmsg(cmd, s_MoraleServ, "It may be quite simple but now that it's done",u->nick);
    prefmsg(cmd, s_MoraleServ, "I hope you don't mind, I hope you don't mind that I put down in words",u->nick);
    prefmsg(cmd, s_MoraleServ, "How wonderful life is while %s is in the world",cmd);

    chanalert(s_Services, "%s Wanted to express how WONDERFUL %s is", u->nick,cmd);
    mslog("%s Wanted to express how WONDERFUL %s is",u->nick,cmd);

}


/* Routine for MoraleServ to RESET */
static void ms_reset(User *u)
{
    strcpy(segv_location, "ms_reset");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (RESET) from %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        return;
    }
    /* The user has passed the minimum requirements for RESET */

    remove("logs/MoraleServ.log");
    mslog("%s RESET %s's logs",u->nick,s_MoraleServ);
    prefmsg(u->nick, s_MoraleServ, "%s has been sucessfuly RESET", s_MoraleServ);

}


/* Routine for MoraleServ to conduct a LOGBACKUP */
static void ms_logbackup(User *u)
{
    char tmp[27];
    time_t t = time(NULL);
    
    strcpy(segv_location, "ms_logbackup");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (LOGBACKUP) from %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        return;
    }
    /* The user has passed the minimum requirements for RESET */

    mslog("%s ended this log session with a LOGBACKUP %s's logs",u->nick, s_MoraleServ, tmp);    
    strftime(tmp, 27, "logs/MoraleServ-%m-%d.log", localtime(&t));
    rename("logs/MoraleServ.log", tmp);
    mslog("%s created a LOGBACKUP %s's logs - %s",u->nick, s_MoraleServ, tmp);
    prefmsg(u->nick, s_MoraleServ, "%s has sucessfuly had a LOGBACKUP occur", s_MoraleServ);

}


/* Routine for PRINTFILE - file print to PRIVMSG/NOTICE */
static void ms_printfile(User *u, char *cmd)
{
    FILE *fp;
    char buf[512];

    strcpy(segv_location, "ms_printfile");
    if (!(UserLevel(u) >= 185)) {
        mslog("Access Denied (PRINTFILE) to %s", u->nick);
        prefmsg(u->nick, s_MoraleServ, "Access Denied.");
        return;
    }

    fp = fopen(cmd, "r");
    if (!fp) {
        prefmsg(u->nick, s_MoraleServ, "Unable to open requested file");
        return;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';
        prefmsg(u->nick, s_MoraleServ, "%s", buf);
    }
    fclose(fp);

    mslog("%s used the PRINTFILE facility to access the contents of: %s", u->nick, cmd);

}
