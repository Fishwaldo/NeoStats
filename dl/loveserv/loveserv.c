/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module:  LoveServ
** Version: 1.0
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

void lslog(char *, ...);

Module_Info my_info[] = { {
	"LoveServ",
	"A Network Love Service",
	"1.1"
} };


int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module LoveServ Loaded, Version %s %s %s",me.name, av,my_info[0].module_version,loveversion_date,loveversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
	char *cmd;
	u = finduser(origin);
/*	if (!u) {
		log("Unable to find user %s (loveserv)", origin);
		return -1;
	} */

	if (coreLine == NULL) return -1;
	cmd = strtok(coreLine, " ");

	if (!strcasecmp(cmd, "HELP")) {
		coreLine = strtok(NULL, " ");
		if (!coreLine) {
			privmsg_list(u->nick, s_LoveServ, ls_help);
			return 1;
		} else if (!strcasecmp(coreLine, "ROSE")) {
			privmsg_list(u->nick, s_LoveServ, ls_help_rose);
			return 1;
                } else if (!strcasecmp(coreLine, "KISS")) {
			lslog("HELP FOR KISS");
			return 1;
		} else 
			privmsg(u->nick, s_LoveServ, "Unknown Help Topic: \2%s\2",
				coreLine);
        }

	if (!strcasecmp(cmd, "ROSE")) {
                cmd = strtok(NULL, " ");
                ls_rose(u, cmd);
	} else if (!strcasecmp(cmd, "KISS")) {
		cmd = strtok(NULL, " ");
		ls_kiss(u, cmd);
	} else if (!strcasecmp(cmd, "TONSIL")) {
		cmd = strtok(NULL, " ");
		ls_tonsil(u, cmd);
	} else if (!strcasecmp(cmd, "HUG")) {
		cmd = strtok(NULL, " ");
		ls_hug(u, cmd);
	} else if (!strcasecmp(cmd, "ADMIRER")) {
		cmd = strtok(NULL, " ");
		ls_admirer(u, cmd);
	} else if (!strcasecmp(cmd, "CHOCOLATE")) {
		cmd = strtok(NULL, " ");
		ls_choco(u,cmd);
	} else if (!strcasecmp(cmd, "CANDY")) {
		cmd = strtok(NULL, " ");
		ls_candy(u,cmd);
	} else if (!strcasecmp(cmd, "LOVENOTE")) {
                char *m;
                cmd = strtok(NULL, " ");
                m = strtok(NULL, "");
                ls_lovenote(u, cmd, m);

/*		cmd = strtok(NULL, " ");
		ls_lovenote(u, cmd); */
	} else {
		privmsg(u->nick, s_LoveServ, "Unknown Command: \2%s\2, maybe you love me?",
			cmd);
	}
	return 1;


}

int Online(Server *data) {

	if (init_bot(s_LoveServ,"love",me.name,"Network Love Service", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_LoveServ = strcat(s_LoveServ, "_");
		init_bot(s_LoveServ,"love",me.name,"Network Love Service", "+xd", my_info[0].module_name);
	}
	return 1;
};


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
	{ NULL, 	NULL}
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
	sts(":%s GLOBOPS :LoveServ Module Loaded", me.name);
}


void _fini() {
	sts(":%sGLOBOPS :LoveServ Module Unloaded", me.name);

};


void lslog(char *fmt, ...)
{
        va_list ap;
        FILE *lovefile = fopen("loveserv.log", "a");
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
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s ROSE NICK", s_LoveServ);
		privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

	privmsg(u->nick, s_LoveServ, "Your rose has been sent to %s!",cmd);
        privmsg(cmd, s_LoveServ, "%s has sent you this beautiful rose! 3--<--<--<{4@",u->nick);
	lslog("%s sent a ROSE to %s",u->nick,cmd);
}

static void ls_kiss(User *u, char *cmd) {
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s KISS NICK", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "You have virtually kissed %s!",cmd);
        privmsg(cmd, s_LoveServ, "%s has virtually kissed you!",u->nick);
        lslog("%s sent a KISS to %s",u->nick,cmd);
}

static void ls_tonsil(User *u, char *cmd) {
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s TONSIL NICK", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "You have virtually tonsilly kissed %s!",cmd);
        privmsg(cmd, s_LoveServ, "%s would like to send a SLoW..LoNG..DeeP..PeNeTRaTiNG..ToNSiL-TiCKLiNG.. HaiR STRaiGHTeNiNG..Toe-CuRLiNG..NeRVe-JaNGLiNG..LiFe-aLTeRiNG.. FaNTaSY-CauSiNG..i JuST SaW GoD!..GoSH, DiD MY CLoTHeS FaLL oFF?.. YeS, i'M GLaD i CaMe oN iRC..KiSS oN Da LiPS!!!",u->nick);
        lslog("%s sent a TONSIL KISS to %s",u->nick,cmd);
}


static void ls_hug(User *u, char *cmd) {
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s HUG NICK", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "%s has received your hug! :)",cmd);
        privmsg(cmd, s_LoveServ, "%s has sent you a *BIG WARM HUG*!",u->nick);
        lslog("%s sent a HUG to %s",u->nick,cmd);
}

static void ls_admirer(User *u, char *cmd) {
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s ADMIRER NICK", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "Anonymous admire sent to %s :)",cmd);
        privmsg(cmd, s_LoveServ, "You have a secret admirer! ;)");
        lslog("%s sent a ADMIRER to %s",u->nick,cmd);
}

static void ls_choco(User *u, char *cmd) {
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s CHOCOLATE NICK", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "A box of cholocates has been sent to %s :)",cmd);
        privmsg(cmd, s_LoveServ, "%s would like you to have this YUMMY box of chocolates!",u->nick);
        lslog("%s sent a Box of Chocolates to %s",u->nick,cmd);
}

static void ls_candy(User *u, char *cmd) {
        if (!cmd) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s CANDY NICK", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "A bag of yummy heart shaped candies has been sent to %s :)",cmd);
        privmsg(cmd, s_LoveServ, "%s would like you to have this big YUMMY bag of heart shaped candies!",u->nick);
        lslog("%s sent a BAG OF HEART SHAPED CANDIES to %s",u->nick,cmd);
}

static void ls_lovenote(User *u, char *cmd, char *m) {
        if (!m) {
                privmsg(u->nick, s_LoveServ, "Syntax: /msg %s LOVENOTE NICK NOTE", s_LoveServ);
                privmsg(u->nick, s_LoveServ, "For addtional help: /msg %s HELP", s_LoveServ);
                return;
        }

        privmsg(u->nick, s_LoveServ, "Your lovenote to %s has been sent! :)", cmd);
        privmsg(cmd, s_LoveServ, "%s has sent you a LoveNote which reads: \2%s\2", u->nick, m);
        lslog("%s sent a LOVE NOTE to %s which reads %s", u->nick, cmd, m);
}

