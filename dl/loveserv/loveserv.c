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
** $Id: loveserv.c,v 1.11 2003/01/21 13:15:33 fishwaldo Exp $
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "ls_help.c"

const char loveversion_date[] = __DATE__;
const char loveversion_time[] = __TIME__;
char *s_LoveServ;
extern const char *ls_help[];
static void ls_rose(User *u, char *cmd);
static void ls_kiss(User *u, char *cmd);
static void ls_tonsil(User *u, char *cmd);
static void ls_hug(User *u, char *cmd);
static void ls_admirer(User *u, char *cmd);
static void ls_choco(User *u, char *cmd);
static void ls_candy(User *u, char *cmd);
static void ls_lovenote(User *u, char *cmd, char *m);
static void ls_apology(User *u, char *cmd, char *m);
static void ls_thankyou(User *u, char *cmd, char *m);
static void ls_version(User *u);
static void ls_viewlogs(User *u);
static int new_m_version(char *origin, char **av, int ac);

void lslog(char *, ...);

Module_Info my_info[] = { {
    "LoveServ",
    "A Network Love Service",
    "1.5"
} };

int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module LoveServ Loaded, Version: %s %s %s",my_info[0].module_version,loveversion_date,loveversion_time);
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
            privmsg_list(u->nick, s_LoveServ, ls_help);
            privmsg_list(u->nick, s_LoveServ, ls_help_admin);
            return 1;
        } else if (ac <= 2) {
            privmsg_list(u->nick, s_LoveServ, ls_help);
            return 1;
        } else if (!strcasecmp(av[2], "ROSE")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_rose);
            return 1;
        } else if (!strcasecmp(av[2], "KISS")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_kiss);
            return 1;
        } else if (!strcasecmp(av[2], "TONSIL")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_tonsil);
            return 1;
        } else if (!strcasecmp(av[2], "HUG")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_hug);
            return 1;
        } else if (!strcasecmp(av[2], "ADMIRER")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_admirer);
            return 1;
        } else if (!strcasecmp(av[2], "CHOCOLATE")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_chocolate);
            return 1;
        } else if (!strcasecmp(av[2], "CANDY")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_candy);
            return 1;
        } else if (!strcasecmp(av[2], "LOVENOTE")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_lovenote);
            return 1;
        } else if (!strcasecmp(av[2], "APOLOGY")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_apology);
            return 1;
        } else if (!strcasecmp(av[2], "THANKYOU")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_thankyou);
            return 1;
        } else if (!strcasecmp(av[2], "VERSION")) {
            privmsg_list(u->nick, s_LoveServ, ls_help_version);
            return 1;
        } else if (!strcasecmp(av[2], "VIEWLOGS") && (UserLevel(u) >= 180)) {
            privmsg_list(u->nick, s_LoveServ, ls_help_viewlogs);
            return 1;
        } else 
            prefmsg(u->nick, s_LoveServ, "Unknown Help Topic: \2%s\2", av[2]);
        }

    if (!strcasecmp(av[1], "ROSE")) {
         if (ac < 3) {
             prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s ROSE NICK", s_LoveServ);
             prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
             return -1;
             }
         ls_rose(u, av[2]);
    } else if (!strcasecmp(av[1], "KISS")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s KISS NICK", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                ls_kiss(u, av[2]);
    } else if (!strcasecmp(av[1], "TONSIL")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s TONSIL NICK", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                ls_tonsil(u, av[2]);
    } else if (!strcasecmp(av[1], "HUG")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s HUG NICK", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                ls_hug(u, av[2]);
    } else if (!strcasecmp(av[1], "ADMIRER")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s ADMIRER NICK", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                ls_admirer(u, av[2]);
    } else if (!strcasecmp(av[1], "CHOCOLATE")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s CHOCOLATE NICK", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                ls_choco(u, av[2]);
    } else if (!strcasecmp(av[1], "CANDY")) {
                if (ac < 3) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s CANDY NICK", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                ls_candy(u, av[2]);
    } else if (!strcasecmp(av[1], "LOVENOTE")) {
                if (ac < 4) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s LOVENOTE NICK NOTE", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                cmd = joinbuf(av, ac, 3);
                ls_lovenote(u, av[2], cmd);
                free(cmd);
    } else if (!strcasecmp(av[1], "APOLOGY")) {
                if (ac < 4) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s APOLOGY NICK APOLOGY-REASON", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                cmd = joinbuf(av, ac, 3);
                ls_apology(u, av[2], cmd);
                free(cmd);
    } else if (!strcasecmp(av[1], "THANKYOU")) {
                if (ac < 4) {
                    prefmsg(u->nick, s_LoveServ, "Syntax: /msg %s THANKYOU NICK THANKYOU-REASON", s_LoveServ);
                    prefmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                    return -1;
                }
                cmd = joinbuf(av, ac, 3);
                ls_thankyou(u, av[2], cmd);
                free(cmd);
    } else if (!strcasecmp(av[1], "VERSION")) {
                ls_version(u);
    } else if (!strcasecmp(av[1], "VIEWLOGS") && (UserLevel(u) >= 180)) {
                chanalert(s_LoveServ,"%s Requested to Look at Loveserv's Logs", u->nick);
               ls_viewlogs(u);
    } else {
        prefmsg(u->nick, s_LoveServ, "Unknown Command: \2%s\2, maybe you love me?", av[1]);
    }
    return 1;
}


int Online(char **av, int ac) {
    if (init_bot(s_LoveServ,"love",me.name,"Network Love Service", "+Sq-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use!!!! */
        s_LoveServ = strcat(s_LoveServ, "_");
        init_bot(s_LoveServ,"love",me.name,"Network Love Service", "+Sq-x", my_info[0].module_name);
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
    s_LoveServ = "LoveServ";
}


void _fini() {
};


void lslog(char *fmt, ...)
{
		va_list ap;
        FILE *lovefile = fopen("logs/loveserv.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!lovefile) {
        log("Unable to open loveserv.log for writing.");
        return;
    }

    fprintf(lovefile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(lovefile);
}


static void ls_rose(User *u, char *cmd) {
    strcpy(segv_location, "ls_rose");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "Your rose has been sent to %s!",cmd);
    prefmsg(cmd, s_LoveServ, "%s has sent you this beautiful rose! 3--<--<--<{4@",u->nick);
    lslog("%s sent a ROSE to %s",u->nick,cmd);
}


static void ls_kiss(User *u, char *cmd) {
    strcpy(segv_location, "ls_kiss");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "You have virtually kissed %s!",cmd);
    prefmsg(cmd, s_LoveServ, "%s has virtually kissed you!",u->nick);
    lslog("%s sent a KISS to %s",u->nick,cmd);
}


static void ls_tonsil(User *u, char *cmd) {
    strcpy(segv_location, "ls_tonsil");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "You have virtually tonsilly kissed %s!",cmd);
    prefmsg(cmd, s_LoveServ, "%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",u->nick);
    lslog("%s sent a TONSIL KISS to %s",u->nick,cmd);
}


static void ls_hug(User *u, char *cmd) {
    strcpy(segv_location, "ls_hug");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "%s has received your hug! :)",cmd);
    prefmsg(cmd, s_LoveServ, "%s has sent you a *BIG WARM HUG*!",u->nick);
    lslog("%s sent a HUG to %s",u->nick,cmd);
}


static void ls_admirer(User *u, char *cmd) {
    strcpy(segv_location, "ls_admirer");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "Anonymous admire sent to %s :)",cmd);
    prefmsg(cmd, s_LoveServ, "You have a secret admirer! ;)");
    lslog("%s sent a ADMIRER to %s",u->nick,cmd);
}


static void ls_choco(User *u, char *cmd) {
    strcpy(segv_location, "ls_choco");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "A box of cholocates has been sent to %s :)",cmd);
    prefmsg(cmd, s_LoveServ, "%s would like you to have this YUMMY box of chocolates!",u->nick);
    lslog("%s sent a Box of Chocolates to %s",u->nick,cmd);
}


static void ls_candy(User *u, char *cmd) {
    strcpy(segv_location, "ls_candy");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "A bag of yummy heart shaped candies has been sent to %s :)",cmd);
    prefmsg(cmd, s_LoveServ, "%s would like you to have this big YUMMY bag of heart shaped candies!",u->nick);
    lslog("%s sent a BAG OF HEART SHAPED CANDIES to %s",u->nick,cmd);
}


static void ls_lovenote(User *u, char *cmd, char *m) {
    strcpy(segv_location, "ls_lovenote");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "Your lovenote to %s has been sent! :)", cmd);
    prefmsg(cmd, s_LoveServ, "%s has sent you a LoveNote which reads: \2%s\2", u->nick, m);
    lslog("%s sent a LOVE NOTE to %s which reads %s", u->nick, cmd, m);
}


static void ls_apology(User *u, char *cmd, char *m) {
    strcpy(segv_location, "ls_apology");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "Your apology has been sent to %s", cmd);
    prefmsg(cmd, s_LoveServ, "%s is sorry, and would like to apologise for \2%s\2", u->nick, m);
    lslog("%s sent an APOLOGY to %s for %s", u->nick, cmd, m);
}


static void ls_thankyou(User *u, char *cmd, char *m) {
    strcpy(segv_location, "ls_thankyou");
	if (!strcasecmp(cmd, s_LoveServ)) {
        prefmsg(u->nick, s_LoveServ, "Surley we have better things to do with our time than make a service message itself?");
        return;
    }
    if (!finduser(cmd)) {
        prefmsg(u->nick, s_LoveServ, "That user cannot be found on IRC. As a result, your message was not sent. Please check the spelling and try again!");
        return;
    }

    prefmsg(u->nick, s_LoveServ, "Your Thank You has been sent to %s", cmd);
    prefmsg(cmd, s_LoveServ, "%s wishes to thank you for \2%s\2", u->nick, m);
    lslog("%s sent a THANKYOU to %s for %s", u->nick, cmd, m);
}


static void ls_version(User *u)
{
        strcpy(segv_location, "ls_version");
        prefmsg(u->nick, s_LoveServ, "\2%s Version Information\2", s_LoveServ);
        prefmsg(u->nick, s_LoveServ, "%s Version: %s - running on: %s", s_LoveServ, my_info[0].module_version, me.name);
        prefmsg(u->nick, s_LoveServ, "%s Author: Shmad <shmad@neostats.net>", s_LoveServ);
        prefmsg(u->nick, s_LoveServ, "Neostats Satistical Software: http://www.neostats.net");

}


static void ls_viewlogs(User *u)
{
    FILE *fp;
    char buf[512];

    strcpy(segv_location, "ls_viewlogs");
    if (!(UserLevel(u) >= 180)) {
        lslog("Access Denied (LOGS) to %s", u->nick);
        prefmsg(u->nick, s_LoveServ, "Access Denied.");
        return;
    }
    fp = fopen("logs/loveserv.log", "r");
    if (!fp) {
        prefmsg(u->nick, s_LoveServ, "Unable to open logs/loveserv.log");
        return;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf)] = '\0';
        prefmsg(u->nick, s_LoveServ, "%s", buf);
    }
    fclose(fp);
    lslog("%s viewed LoveServ's logs", u->nick);
}

