/* 
 *  icqserv
 *  Justin Hammond, 1999
 *  Dynamx Internet Services
 *  Comment: Added icqserv to statserv services for irc!
 ******************************************************************************
 *   Version: 0.01 
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "icqconfig.h"
#include "icqlib.h"
#include "icq.h"
#include "stats.h"
#include "dl.h"
#include "dotconf.h"

#include "icq_help.c"


struct uin_info
{
   unsigned long uin;
   int replied;
   char message[MAX_MSG_LENGTH];
   int category;
   long senttime;
};

char *msg, *msgx;

const char version_date[] = __DATE__;
const char version_time[] = __TIME__;
char s_Icq[MAXNICK] = "IcqServ";


int new_m_version(char *av, char *tmp);
void icq_send(User *u, long uin, char *msg);
int icq_start();
void ss_cb_Config(char *, int);

Module_Info my_info[] = { {
	"Icq",
	"Icq to Irc Gateway!",
	"0.1"
} };

Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

EventFnList my_event_list[] = {
	{ "ONLINE", 	icq_start}, 
	{ NULL, 	NULL}
};

EventFnList *__module_get_events() {
	return my_event_list;
};

Module_Info *__module_get_info() {
	return my_info;
}

Functions *__module_get_functions() {
	return my_fn_list;
};

static config_option options[] = {
{ "ICQSERV_NICK", ARG_STR, ss_cb_Config, 0},
{ "ICQSERV_USER", ARG_STR, ss_cb_Config, 1},
{ "ICQSERV_HOST", ARG_STR, ss_cb_Config, 2},
{ "ICQSERV_UIN", ARG_STR, ss_cb_Config, 3},
{ "ICQSERV_SERVER", ARG_STR, ss_cb_Config, 4},
{ "ICQSERV_PASSWORD", ARG_STR, ss_cb_Config, 5}
};

void ss_cb_Config(char *arg, int configtype) {
	if (configtype == 0) {
		/* Nick */
		memcpy(IcqServ.nick, arg, MAXNICK);
		memcpy(s_Icq, IcqServ.nick, MAXNICK);
#ifdef DEBUG
		log("IcqServ nick :%s ", arg);
#endif
	} else if (configtype == 1) {
		/* User */
		memcpy(IcqServ.user, arg, 8);
	} else if (configtype == 2) {
		/* host */
		memcpy(IcqServ.host, arg, MAXHOST);
	} else if (configtype == 3) {
		/* uin */
		IcqServ.uin = atoi(arg);
	} else if (configtype == 4) {
		/* Server */
		memcpy(IcqServ.server, arg, MAXHOST);
	}  else if (configtype == 5) {
		/* Server */
		memcpy(IcqServ.passwd, arg, MAXPASS);
	}
}






void _init() {
	sts(":%s GLOBOPS :ICQ Module Loaded",me.name);
}
void _fini() {
	icq_ChangeStatus(STATUS_OFFLINE);
	icq_Logout();
	icq_Disconnect();
	IcqServ.online=0;
	del_bot(s_Icq, "Module Unloaded");
	del_socket("ICQ Port 400UDP");
	sts(":%s GLOBOPS :ICQ Module Unloaded",me.name);
}






int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module IcqServ Loaded, v%s %s %s",me.name,av,my_info[0].module_version,version_date,version_time);
	return 0;
}


void icq_Logging(time_t time, unsigned char level, const char *str) {
if (IcqServ.online == 1) {
	if (level <= IcqServ.loglevel) {
		notice(s_Icq,"ICQServ Log %d: %s",level,str);
	}
}
}


void __Bot_Message(char *origin, char *coreLine)
{
	char *cmd, *rest;
	User *u;
	
	log("origin %s, coreline %s", origin, coreLine);
	u = finduser(origin);
	if (!u) {
		log("Unable to finduser %s (IcqServ)", origin);
		return;
	}

	me.requests++;

	if (flood(u))
		return;
	cmd = strtok(coreLine, " ");
	

	log("%s received message from %s: %s", s_Icq, origin, cmd);
 
	if (!strcasecmp(cmd, "HELP")) {
		cmd = strtok(NULL, " ");
		notice(s_Icq,"%s is a Dummy and wanted help on %s",u->nick, cmd);
		if (!cmd) {
			privmsg_list(origin, s_Icq, icq_help);

		}
	} else if (!strcasecmp(cmd, "SEND")) {
		cmd = strtok(NULL, " ");
		rest = strtok(NULL,"");
		icq_send(u, atol(cmd), rest);
	} else if (!strcasecmp(cmd, "DEBUG")) {
        	if (UserLevel(u) < 200) {
                	log("Access Denied (DEBUG) to %s", u->nick);
                        privmsg(u->nick, s_Icq, "Access Denied.");
                        notice(s_Icq,"%s Requested DEBUG, but is not a god!",u->nick);
                        return;
		}
		IcqServ.loglevel++;
		if (IcqServ.loglevel == 5 ) {
			IcqServ.loglevel = 2;
			notice(s_Icq, "%s has Changed my Logging level to Errors",u->nick);
		}
		if (IcqServ.loglevel == 4 )
			notice(s_Icq, "%s has Changed my Logging level to Messages",u->nick);
		if (IcqServ.loglevel == 3 )
			notice(s_Icq, "%s has Changed my Logging level to Warnings",u->nick);
	}			
			
}




int doICQProtocol()
{
icq_KeepAlive();
return 1;
}

int do_connect(const char *server, int port)
{
    log("Connecting to: %s:%d",server,port);
    icq_UnsetProxy();
    icq_Connect(server,port);
    add_socket("icq_HandleServerResponse", "ICQ Port 400UDP", icq_Sok, my_info[0].module_name);
    return 1;
}

void loginOK()
{
  init_bot(s_Icq, IcqServ.user, IcqServ.host, "/msg IcqServ HELP", "+xS", my_info[0].module_name);
#ifdef DEBUG
   IcqServ.loglevel=4;
#endif
   IcqServ.onchan=1;
   IcqServ.online=1;
}

void connectionLost()
{
	char *buf = NULL;
  	notice(s_Icq,"Lost Connection to ICQ Server");
	icq_ChangeStatus(STATUS_OFFLINE);
	icq_Logout();
	icq_Disconnect();
	IcqServ.online=0;
	del_bot(s_Icq, "Module Unloaded");
	del_socket("ICQ Port 400UDP");
	IcqServ.online=0;
  	IcqServ.loginok = 0;
	if(do_connect(IcqServ.server, IcqServ.port) != -1)
   		{
    		if (strlen(IcqServ.passwd) > 8)
       			strncpy(buf, IcqServ.passwd, 8);
    		else
       			strcpy(buf, IcqServ.passwd);
    		icq_Init(IcqServ.uin, buf);
    		icq_Login(STATUS_ONLINE);
    		IcqServ.loginok = 1;
   	}
   	else
   	{
      		log("Cant connect to server: %s:%d\n",IcqServ.server,IcqServ.port);
      		return;
   	}
}
void formatline(char *line) {
	char *tmp;
	tmp = strchr(line, '\n');
	while (tmp) {
		*tmp = ' ';
		tmp = strchr(line, '\n');
	}
	tmp = strchr(line, '\r');
	while (tmp) {
		*tmp = ' ';
		tmp = strchr(line, '\r');
	}
}



void got_icq_message(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, char *msg)
{
 char *nick;
 char *doaction;
 char *tmp;
 User *u;

 formatline(msg);
 doaction = strtok(msg, " ");
 if (!strcasecmp(doaction, "SEND")) {
 	nick = strtok(NULL, " ");
	tmp = strtok(NULL, "");
 	u = finduser(nick);
 	if (u) {
 		privmsg(u->nick, s_Icq, tmp);
 		notice(s_Icq, "Recieved a ICQ message for %s from %d", u->nick, uin);
	} else {
		icq_SendMessage(uin, "That user is not online at the moment");
		notice(s_Icq, "Recieved a ICQ Message for %s, but they are not online", nick);
	}
 } else if (!strcasecmp(doaction, "HELP")) {
 	icq_SendMessage(uin, "This is the Help!");
 } else {
 	icq_SendMessage(uin, "Woo, This is a Bot, send me a message containing HELP for Help");
 	notice(s_Icq, "Recieved a Unknown Command from %d on ICQ: %s", uin, msg );
 }	
}
void RespondAuthReq (unsigned long uin,
                     unsigned char hour,
                     unsigned char minute,
                     unsigned char day,
                     unsigned char month,
                     unsigned short year,
 		     const char *nick, 
 		     const char *first, 
 		     const char *last, 
 		     const char *email, 
                     const char *reason)



{
    fprintf(stderr, "Authorization request received from %ld. Reason: %s\n", uin, reason);
    icq_SendAuthMsg(uin);
}

int icq_start() {
    char buf[9];

    icq_Logged = loginOK;
    icq_LogLevel = 4;
    icq_Disconnected = connectionLost;
    icq_RecvAuthReq = RespondAuthReq;
    icq_RecvMessage = got_icq_message; 
    icq_Log = icq_Logging;
    IcqServ.port = 4000;

    IcqServ.onchan=0;
    
    memcpy(IcqServ.user, Servbot.user, 8);
    memcpy(IcqServ.host, Servbot.host, MAXHOST);
    if (!config_read("stats.cfg", options) == 0) {
    	log("Error, IcqServ could not be configured");
	return -1;
    }    
    /* Now setup the Timer to check the ICQ port */
    add_mod_timer("doICQProtocol", "ICQ Polling", my_info[0].module_name, 10);
    
    
   if(do_connect(IcqServ.server, IcqServ.port) != -1)
   {
    if (strlen(IcqServ.passwd) > 8)
       strncpy(buf, IcqServ.passwd, 8);
    else
       strcpy(buf, IcqServ.passwd);
    icq_Init(IcqServ.uin, buf);
    icq_Login(STATUS_ONLINE);
    IcqServ.loginok = 1;
   }
   else
   {
      log("Cant connect to server: %s:%d\n",IcqServ.server,IcqServ.port);
      return -1;
   }
   return 1;
}

void icq_send(User *u, long uin, char *msg)
{

  if (!msg) {
     privmsg(u->nick,s_Icq, "Incorrect Syntax: /msg %s send <uin> <message>", s_Icq);
     return;
  }
  strip(msg);
  msgx = malloc(strlen(msg)+strlen(u->nick)+55+strlen(me.netname));
  sprintf(msgx, "%s has Sent you a InterChat Message from the %s IRC network\r\n%s", u->nick, me.netname, msg);
  
   icq_SendMessage(uin, msgx);
   free(msgx); 
   notice(s_Icq,"%s sent a message to %d",u->nick, uin);
}
