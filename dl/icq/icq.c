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

#include "icq.h"
#include "icqconfig.h"
#include "stats.h"
#include "dl.h"

const char version_date[] = __DATE__;
const char version_time[] = __TIME__;
char s_Icq[MAXNICK] = "IcqServ";


int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module IcqServ Loaded, v%s %s %s",my_info[0].version me.name,av,v1version_date,v1version_time);
	return 0;
}

Module_Info my_info[] = { {
	"IcqServ",
	"Icq to Irc Gateway!",
	"0.1"
} };

Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

EventFnList my_event_list[] = {
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

void _init() {
	sts(":%s GLOBOPS :Version Module Loaded",me.name);
}
void _fini() {
	sts(":%s GLOBOPS :Version Module Unloaded",me.name);
}



struct uin_info
{
   unsigned long uin;
   int replied;
   char message[MAX_MSG_LENGTH];
   int category;
   long senttime;
};

char *msg, *msgx;


void icq_Logging(time_t time, unsigned char level, const char *str) {
if (IcqServ.online = 1) {
	if (level <= IcqServ.loglevel) {
		notice(s_Icq,"ICQServ Log %d: %s",level,str);
	}
}
}


void IcqServer(char *line)
{
	char *cmd, *nick = strtok(line, " ");
	User *u;
	char *rest;
	
	nick++;
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, " ");
	cmd++;
	u = finduser(nick);
	if (!u) {
		log("Unable to finduser %s (IcqServ)", nick);
		return;
	}

	me.requests++;

	if (flood(u))
		return;

	log("%s received message from %s: %s", s_Icq, nick, cmd);
 
	if (me.onlyopers && !u->is_oper) {
		privmsg(u->nick, s_Icq,
			"This service is only available to IRCops.");
		notice ("%s Requested %s, but he is Not a Operator!", u->nick, cmd);
		return;
	}

	if (!strcasecmp(cmd, "HELP")) {
		cmd = strtok(NULL, " ");
		notice(s_Icq,"%s is a Dummy and wanted help on %s",u->nick, cmd);
		if (!cmd) {
			privmsg_list(nick, s_Icq, icq_help);

		}
	} else if (!strcasecmp(cmd, "SEND")) {
		cmd = strtok(NULL, " ");
		rest = strtok(NULL,"");
		icq_send(u, atol(cmd), rest);
	} else if (!strcasecmp(cmd, "DEBUG")) {
        	if (!u->is_coder) {
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
    struct timeval tv;
    fd_set rfds;
    int    sok=icq_GetSok();
    tv.tv_sec  = 0;	
    tv.tv_usec = 0;
    
    FD_ZERO(&rfds);
    FD_SET(sok, &rfds);

    if (IcqServ.loginok == 1) {
	icq_Main();
	if (times > 5) {
	        icq_KeepAlive();
	        times = 0;
	}
	times++;
    }
}

static int do_connect(const char *server, int port)
{
    log("Connecting to: %s:%d",server,port);
    icq_UnsetProxy();
    return icq_Connect(server,port);
}

void loginOK()
{
   sts("NICK %s 1 1 IcqServ %s %s :ICQ Gateway Service", s_Icq, IcqServ.host, IcqServ.host);
   AddUser(s_Icq,"IcqServ",IcqServ.host,me.name);
   sts(":%s JOIN %s",s_Icq, me.chan);
   sts(":%s MODE %s +o %s",s_StatServ, me.chan, s_Icq); 
#ifdef DEBUG
   IcqServ.loglevel=4;
#endif
   IcqServ.onchan=1;
}

void connectionLost()
{
  notice(s_Icq,"Lost Connection to ICQ Server");
  icq_ChangeStatus(STATUS_OFFLINE);
  icq_Logout();
  icq_Disconnect();
  IcqServ.online=0;
  log("icqserv onchan %d",IcqServ.onchan);
  if (IcqServ.onchan == 1) 
  	sts(":%s QUIT :Quit: Lost Connection to ICQ Network ",s_Icq);
  IcqServ.loginok = 0;
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
    icq_Log = icq_Logging;

    IcqServ.onchan=0;
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
}

extern void icq_send(User *u, long uin, char *msg)
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
