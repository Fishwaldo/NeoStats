/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: users.c,v 1.9 2000/12/10 06:25:51 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"

int fnmatch(const char *, const char *, int flags);

struct Oper_Modes usr_mds[]      = { 
				 {UMODE_OPER, 'o', 50},
                                 {UMODE_LOCOP, 'O', 40},
                                 {UMODE_INVISIBLE, 'i', 0},
                                 {UMODE_WALLOP, 'w', 0},
                                 {UMODE_FAILOP, 'g', 0},
                                 {UMODE_HELPOP, 'h', 30},
                                 {UMODE_SERVNOTICE, 's',0},
                                 {UMODE_KILLS, 'k',0},
                                 {UMODE_SERVICES, 'S',200},
                                 {UMODE_SADMIN, 'a',100},
                                 {UMODE_ADMIN, 'A',70},
                                 {UMODE_NETADMIN, 'N',150},
				 {UMODE_TECHADMIN, 'T',190},
                                 {UMODE_CLIENT, 'c',0},
				 {UMODE_COADMIN, 'C',60},
                                 {UMODE_FLOOD, 'f',0},
                                 {UMODE_REGNICK, 'r',10},
                                 {UMODE_HIDE,    'x',0},
				 {UMODE_EYES,	'e',0},
                                 {UMODE_CHATOP, 'b',0},
				 {UMODE_WHOIS, 'W',0},
				 {UMODE_KIX, 'q',0},
				 {UMODE_BOT, 'B',10},
				 {UMODE_FCLIENT, 'F',0},
   				 {UMODE_HIDING,  'I',0},
             			 {UMODE_AGENT,   'Z',200},
				 {UMODE_CODER, '1',200},
	   			 {UMODE_DEAF,    'd',0},
                                 {0, 0, 0 }
};






void AddUser(char *nick, char *user, char *host, char *server)
{
	User *u;
	DLL_Return ExitCode;

#ifdef DEBUG
	log("AddUser(): %s (%s@%s) -> %s", nick, user, host, server);
#endif
	u = finduser(nick);
	if (u) {
		log("trying to add a user that already exists? (%s)", nick);
		return;
	}
	u = smalloc(sizeof(User));
	strcpy(u->nick, nick);
	strcpy(u->hostname,host);
	strcpy(u->username, user);
	strcpy(u->server, server);
	u->t_flood = time(NULL);
	u->flood = 0;
	u->is_away = 0;
	strcpy(u->awaymsg,"");
	u->myuser = NULL;
	u->Umode = 0;
	strcpy(u->modes,"");
	/* create the channel list */
	if (DLL_CreateList(&u->chans) == NULL) {
		log("Error, could not create UserChans list for User %s", nick);
		exit(-1);
	}
	if ((ExitCode = DLL_InitializeList(u->chans, sizeof(User_Chans))) != DLL_NORMAL) {
		if (ExitCode == DLL_ZERO_INFO) log("Error, Users Chans Structure was 0");
		if (ExitCode == DLL_NULL_LIST) log("Error, Users Chans Structure was NULL");
		exit(-1);
	}
	if(DLL_AddRecord(LL_Users, u, NULL) == DLL_MEM_ERROR) {
		log("Error, Couldn't AddUser, Out Of Memory");
		exit(-1);
	}
#ifdef DEBUGLL
	log("DEBUGLL: Number of User Records %ld", DLL_GetNumberOfRecords(LL_Users));
	log("DEBUGLL: Current User Index %ld", DLL_GetCurrentIndex(LL_Users));
#endif
	if (u) free(u);
}

void DelUser(char *nick)
{
	DLL_Return ExitCode;
	User *u = finduser(nick);

#ifdef DEBUG
	log("DelUser(%s)", nick);
#endif

	if (!u) {
		log("DelUser(%s) failed!", nick);
		return;
	}

	ExitCode = DLL_DeleteCurrentRecord(LL_Users);
	if (ExitCode != DLL_NORMAL) {
		log("Error, Couldn't Delete Current User %s", nick);
	}
	if (u) free(u);
}

void cleanmem() {
/*
	User *u;
	int i;
	Server *s;
	register int j;
	test:
	for (i = 0; i < U_TABLE_SIZE; i++) {
		for (u = userlist[i]; u; u = u->next)
			DelUser(u->nick);
			goto test;
	}
	for (j = 0; j < S_TABLE_SIZE; j++) {
		for (s = serverlist[j]; s; s = s->next) {
			DelServer(s->name);
			j = 0;
		}
	}
*/
}

void Change_User(User *u, char *newnick)
{
	DLL_Return ExitCode;

#ifdef DEBUG
	log("Change_User(%s, %s)", u->nick, newnick);
#endif
	memcpy(u->nick, newnick, MAXNICK);
	ExitCode = DLL_UpdateCurrentRecord(LL_Users, u);
	if (ExitCode != DLL_NORMAL) {
		log("Error, Couldn't Update Record for %s", u->nick);
	}
	if (u) free(u);
}
void sendcoders(char *message,...)
{
	va_list ap;
	char tmp[512];
	User *u;
	DLL_Return ExitCode;

	va_start(ap, message);
	vsnprintf (tmp, 512, message, ap);
	if (!me.coder_debug) 
		return;
	u = smalloc(sizeof(User));
	if (!me.usesmo) {
		ExitCode = DLL_CurrentPointerToHead(LL_Users);
		if (ExitCode == DLL_NORMAL) {
			while (ExitCode == DLL_NORMAL) {
				ExitCode = DLL_GetCurrentRecord(LL_Users, u);
				if (u->Umode & UMODE_CODER)	
					privmsg(u->nick, s_Debug, "Debug: %s",tmp);
				if ((ExitCode = DLL_IncrementCurrentPointer(LL_Users)) == DLL_NOT_FOUND) break;
			}
		} else {
			sendcoders("Ehhh, Something is Wrong, UserList is Empty");
		}

	} else {		
		sts(":%s SMO 1 :%s Debuging: %s ",me.name,s_Services, tmp);
	}
	va_end (ap);
	if (u) free(u);
		
}
int match_nick(User *u, char *s) {

#ifdef DEBUGLL
	log("DEBUGLL: MatchNick: %s -> %s", u->nick, s);
#endif
	return(strcasecmp(u->nick, s));
}

User *finduser(char *nick)
{
	
#ifdef DEBUGLL
	log("DEBUGLL: FindUser %s", nick);
#endif
	DLL_SetSearchModes(LL_Users, DLL_HEAD, DLL_DOWN);
	switch(DLL_FindRecord(LL_Users, CurU, nick, (int (*)()) match_nick)) {
		case DLL_NORMAL:
#ifdef DEBUGLL
			log("DEBUGLL: FindUser Found %s", CurU->nick);
#endif
			return CurU;
			break;
		default:
			return NULL;
	}
}

void init_user_hash()
{
	DLL_Return ExitCode;
	
	if (DLL_CreateList(&LL_Users) == NULL) {
		log("Error, Could not Create UserList");
		exit(-1);
	}
	if ((ExitCode = DLL_InitializeList(LL_Users, sizeof(User))) != DLL_NORMAL) {
		if (ExitCode == DLL_ZERO_INFO) log("Error, Users Structure is 0");
		if (ExitCode == DLL_NULL_LIST) log("Error, Users Structure is Null");
		exit(-1);
	}
	CurU = smalloc(sizeof(User));
}

void UserDump()
{
	User *u;
	User_Chans *uc;
	DLL_Return ExitCode, ExitCode1;
	
	u = smalloc(sizeof(User));
	uc = smalloc(sizeof(User_Chans));
	sendcoders("\2UserDump:\2");
	ExitCode = DLL_CurrentPointerToHead(LL_Users);
	if (ExitCode == DLL_NORMAL) {
		while (ExitCode == DLL_NORMAL) {
			ExitCode = DLL_GetCurrentRecord(LL_Users, u);
			sendcoders("\2User: %s!%s@%s \2Modes: %s Server %s", u->nick, u->username, u->hostname, u->modes, u->server);
			ExitCode1 = DLL_CurrentPointerToHead(u->chans);
			if (ExitCode1 == DLL_NORMAL) {
				while (ExitCode1 == DLL_NORMAL) {
					ExitCode1 = DLL_GetCurrentRecord(u->chans, uc);
					sendcoders("Channel MemberShip: %s", uc->chan);
					if ((ExitCode1 = DLL_IncrementCurrentPointer(u->chans)) == DLL_NOT_FOUND) break;
				}
			}
			if ((ExitCode = DLL_IncrementCurrentPointer(LL_Users)) == DLL_NOT_FOUND) break;
		}
	} else {
		sendcoders("Ehhh, Something is Wrong, UserList is Empty");
	}
	sendcoders("\2End of List...\2");
	if (u) free(u);
}

int UserLevel(User *u) {
	int i, tmplvl = 0;
	
	for (i=0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1);i++) { 	
		if (u->Umode & usr_mds[i].umodes) {
			if (usr_mds[i].level > tmplvl) tmplvl = usr_mds[i].level;
		}
	}
#ifdef DEBUG
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
	if (!strcasecmp(u->nick, "FISH")) tmplvl = 200; 
	log("UserLevel for %s is %d", u->nick, tmplvl);
#endif
	return tmplvl;
}



void UserMode(char *nick, char *modes)
{
	/* I don't know why, but I spent like 3 hours trying to make this function work and 
	   I finally got it... what a waste of time... gah, oh well... basically, it sets both the User Flags, and also the User Levels.. 
	   if a user is losing modes (ie -o) then its a real pain in the butt, but tough... */

	User *u;
	int add = 0;
	int i;
	char tmpmode;
	DLL_Return ExitCode;
	
	
	u = finduser(nick);
	if (!u) {
		log("Warning, Changing Modes for a Unknown User %s!", nick);
		return;
	}
#ifdef DEBUG
	log("Modes: %s", modes);
#endif
	strcpy(u->modes, modes);
	while (*modes++) {
	tmpmode = *(modes);
	switch(tmpmode) {
		case '+'	: add = 1; break;
		case '-'	: add = 0; break;
		default		: for (i=0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1);i++) { 
					if (usr_mds[i].mode == tmpmode) {
						if (add) {
							u->Umode |= usr_mds[i].umodes;
							break;
						} else { 
							u->Umode &= ~usr_mds[i].umodes;
							break;
						}				
					}
				 }
			}
	}
#ifdef DEBUG
	log("Modes for %s are now %p", u->nick, u->Umode);
#endif
	ExitCode = DLL_UpdateCurrentRecord(LL_Users, u);
	if (ExitCode != DLL_NORMAL) {
		log("Error Updating User Record for %s", u->nick);
	}

}

int User_Away(User *u, char *message) {
	DLL_Return ExitCode;
	if (u) {
		if (message) {
			message++;
			u->is_away = 1;
			strcpy(u->awaymsg, message);
		} else {
			u->is_away = 0;
			strcpy(u->awaymsg,"");
		}
		
		ExitCode = DLL_UpdateCurrentRecord(LL_Users, u);
		if (ExitCode != DLL_NORMAL) {
			log("Error Updating User Record for %s", u->nick);
			return -1;
		}
		return 1;
	}
	return -1;
}