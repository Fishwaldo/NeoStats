#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "struct.h"
#include "dl.h"
#include "msg.h"
#include "user.h"
#include "channels.h"
#include "errors.h"
#include "defines.h"

#ifdef USEPTHREADS
#include <pthread.h>
#endif

Module_Info services_module_info[] = {
	{ "services",
	  "service module for ircd 0.2",
	  "0.3"
	}
};

int privmsg(struct fd_list *, char **, int);
int nick(struct fd_list *, char **, int);
int nickserv(struct fd_list *, char **, int);
int whois(struct fd_list *, char **, int);
int links(struct fd_list *, char **, int);
int who(struct fd_list *, char **, int);
int version(struct fd_list *, char **, int);

const static char *s_NickServ = "NickServ";
const static char *ns_userid = "nickserv";
const static char *ns_hostname = "services.net";
const static char *ns_realname = "Nickname Service";
const static char *ns_server = "services.net";
const static char *ns_server_desc = "IRC Services";

const static char *s_OperServ = "OperServ";
const static char *os_userid = "operserv";
const static char *os_hostname = "services.net";
const static char *os_realname = "Operator Service";
const static char *os_server = "services.net";
const static char *os_server_desc = "IRC Services";

Functions nickserv_function_list[] = {
	{ "PRIVMSG",	privmsg,	1,	0,	0 },
	{ "NICK",	nick,		1,	1,	0 },
	{ "NICKSERV",	nickserv,	1,	0,	0 },
	{ "WHOIS",	whois,		1,	0,	0 },
	{ "WHO",	who,		1,	0,	0 },
	{ "LINKS",	links,		1,	0,	0 },
	{ "NOTICE",	privmsg,	1,	0,	0 },
	{ "VERSION",	version,	1,	0,	0 },
	{ NULL,		NULL,		0,	0,	0 }
};

Module_Info *__module_get_info() {
	return services_module_info;
}

Functions *__module_get_functions() {
	return nickserv_function_list;
}

struct ns_func {
	char *cmd;
	void (*function)(struct fd_list *, char **, int);
	int need_identify;
};

typedef struct ns_func NS_Functions;

struct ns_user {
	struct ns_user *prev;
	struct ns_user *next;
	char nick[15];
	char pass[15];
	int language;
	char url[100];
	char email[100];
	int kill;
	int secure;
	int private;
	struct {
		char usermask:1;
		char email:1;
		char quit:1;
	} hide;
	int registered_at;
	int last_seen_at;
	char last_mask[100];
	char last_quit[200];
	char last_realname[100];
};

typedef struct ns_user NS_User;

struct ns_access {
	struct ns_access *prev;
	struct ns_access *next;
	NS_User *user;
	char hostmask[100];
};

typedef struct ns_access NS_AccessList;

struct ns_links {
	struct ns_links *prev;
	struct ns_links *next;
	NS_User *user;
	char nick[15];
};

typedef struct ns_links NS_LinkList;

NS_User *registered_nicks = NULL;
NS_AccessList *access_list = NULL;
NS_LinkList *link_list = NULL;

/* db format
[type]
key = value

[type]:
  [user] - ahiklmnpqs
	n=nickname
	p=password
	l=language key
	a=address (url)
	m=email
	i=kill protect
	s=secure
	p=private
	h=bitmask -
		0x001 usermask
		0x010 email
		0x100 quit msg
	k=last usermask
	q=last quit msg
  [accs] -
	nick=nickname
	host=usermask
  [link] -
	nick=nickname
	lnik=linked nick
*/

int valid_pass(char *pass, char *nick, char *userid) {
	if (!strcasecmp(pass, nick) || !strcasecmp(pass, userid)) {
		return -1;
	}

	if (strlen(pass) < 5) {
		return -1;
	}

	{
		int i, j, k;
		i = 0; j = 0; k = 0;
		j = pass[0];
		for (i=1; i<strlen(pass); i++) {
			if (j == pass[i] || j == pass[i]+1 || j == pass[i]-1) {
				k++;
			}
		}

		if (k > strlen(pass) / 2) {
			return -1;
		}
	}

	{
		int num_caps, num_lwr, num_symbol, num_numeric, i, j;
		char *caps = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		     *lwr  = "abcdefghijklmnopqrstuvwxyz",
		     *sym  = "`~!@#$%^&*()-_=+[{]}|';:/?.>,<",
		     *num  = "1234567890";

		num_caps = 0; num_lwr = 0; num_symbol = 0; num_numeric = 0;
		i = 0;
		j = 0;
		for (i=0 ; i<strlen(pass); i++) {
			if (strchr(caps, pass[i])) {
				num_caps++;
			} else if(strchr(lwr, pass[i])) {
				num_lwr++;
			} else if(strchr(sym, pass[i])) {
				num_symbol++;
			} else if(strchr(num, pass[i])) {
				num_numeric++;
			}
		}

		if ((num_caps + num_symbol) < 1 || num_numeric < 2) {
			return -1;
		}
	}

	return 0;
}

void substr(char *dest, char *str, int begin, int len) {
	int i;

	for(i=begin; i<(begin+len); i++) {
		dest[i-begin] = str[i];
	}
}

NS_User *find_user(char *nick) {
	NS_User *list; 
	NS_LinkList *list2;
	list = registered_nicks;
	list2 = link_list;

	while(list2 != NULL) {
		if (!strcasecmp(list2->nick, nick)) {
			return list2->user;
		}
		list2 = list2->next;
	}

	while(list != NULL) {
		if (!strcasecmp(list->nick, nick)) {
			return list;
		}
		list = list->next;
	}

	return NULL;
}

/* void ns_load_db() {
	char *db_file = "nickserv.db";
	char buf[512];
	FILE *db;

	db = fopen(db_file, "r");
	if (db == NULL) {
		return;
	}

	while(!feof(db)) {
		fgets(buf, 512, db);
		if (buf[0] == '[') { 
			if (buf[1] == 'u') { 
				NS_User *user;
				user = (NS_User *)malloc(sizeof(NS_User));
				fgets(buf, 512, db);
				switch(*buf) {
				case 'n': 
					{
						char *p = &(buf[2]); int i;
						strncpy(user->nick, p, 15);
						for (i=0; i<strlen(user->nick, i++) {
							if (user->nick[i] == '\n') user->nick[i] = 0;
						}
					}
					break;
				case '
					
			} else if (buf[1] == 'a') {
				NS_AccessList *access;
				access = (NS_AccessList *)malloc(sizeof(NS_AccessList));
				bzero(access, sizeof(NS_AccessList));
				{
					char tmp[15];
					substr(tmp, buf, 3, 15);
*/
					
void ns_dummy(struct fd_list *fd, char **args, int nargs) {
	fprintf(fd->sok, ":%s!%s@%s NOTICE %s :%s: %s\r\n", s_NickServ, ns_userid, ns_hostname, fd->nick, args[1],
		"I don't know how to do that yet.");
	return;
}

void ns_register(struct fd_list *fd, char **args, int nargs) {
	if (nargs >= 2) {
		{
			NS_User *list;
			list = registered_nicks;
			while (list != NULL) {
				if (!strcasecmp(list->nick, fd->nick)) {
					char tmp[255];
					snprintf(tmp, 255, "Nickname \002%s\002 is already registered!", fd->nick);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						tmp);
					return;
				}
				list = list->next;
			}
		}
		{
			NS_User *new_user = (NS_User *)malloc(sizeof(NS_User));
			NS_AccessList *new_acl = (NS_AccessList *)malloc(sizeof(NS_AccessList));
			char userid[15], hostmask[100]; int i, j=0; 
			bzero(new_user, sizeof(NS_User));
			bzero(new_acl, sizeof(NS_AccessList));
			bzero(hostmask, 100);
			strncpy(new_user->nick, fd->nick, 15);
			strncpy(new_user->pass, args[1], 15);
			new_user->registered_at = time(NULL);
			strncpy(new_user->last_realname, fd->realname, 100);

			strncpy(userid, fd->userid, 15);
				
			for (i=0; i<strlen(fd->hostname); i++) {
				if (!j && fd->hostname[i] == '.') {
					j = i+1;
				} else if (j) {
					hostmask[i-j+1] = fd->hostname[i];
				}
			}
			hostmask[0] = '*';

			snprintf(new_acl->hostmask, 100, "%s@%s", userid, hostmask);
			new_acl->user = new_user;

			if (registered_nicks == NULL) {
				registered_nicks = new_user;
			} else {
				NS_User *list;
				list = registered_nicks;
				while (list->next != NULL) list = list->next;
				list->next = new_user;
				new_user->prev = list;
			}

			if (access_list == NULL) {
				access_list = new_acl;
			} else {
				NS_AccessList *list;
				list = access_list;
				while (list->next != NULL) list = list->next;
				list->next = new_acl;
				new_acl->prev = list;
			}

			{
				char tmp[255];
				snprintf(tmp, 255, "Nickname \002%s\002 registered under your account: %s", fd->nick, new_acl->hostmask);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
				snprintf(tmp, 255, "Your password is \002%s\002 - remember this for later use.", new_user->pass);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			}
		}
	} else {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Syntax: \002REGISTER\002 password");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"\002/msg NickServ HELP REGISTER\002 for more information.");
	}
}

#define NS_IS_IDENTIFIED 0x10000000

void ns_identify(struct fd_list *fd, char **args, int nargs) {
	if (nargs > 1) {
		NS_User *user = find_user(fd->nick);
		if (user == NULL) {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "Your nick isn't registered.");
			return;
		} else {
			if (!strcmp(args[1], user->pass)) {
				fd->registered |= NS_IS_IDENTIFIED;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Password accepted - you are now recognized.");
				snprintf(user->last_realname, 100, "%s", fd->realname);
				snprintf(user->last_mask, 200, "%s@%s", fd->userid, fd->hostname);
				user->last_seen_at = time(NULL);
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Password incorrect.");
			}
		}
	} else {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Syntax: \002IDENTIFY\002 password");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"\002/msg NickServ HELP IDENTIFY\002 for more information.");
	}
}

void ns_access(struct fd_list *fd, char **args, int nargs) {
	if (nargs == 1) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Syntax: \002ACCESS { ADD | DEL | LIST } [\002mask\002]\002");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"\002/msg NickServ HELP ACCESS\002 for more information.");
	} else if (nargs == 2) {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (!strcasecmp(args[1], "LIST")) {
				NS_AccessList *list;
				NS_User *user = find_user(fd->nick);
				list = access_list;
				fprintf(fd->sok, ":%s!%s@%s NOTICE %s :Access list:\r\n", s_NickServ, ns_userid, ns_hostname,
					fd->nick);
				while (list != NULL) {
					if (list->user == user) {
						fprintf(fd->sok, ":%s!%s@%s NOTICE %s :    %s\r\n", s_NickServ, ns_userid,
							ns_hostname, fd->nick, list->hostmask);
					}
					list = list->next;
				}
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Syntax: \002ACCESS { ADD | DEL | LIST } [\002mask\002]\002");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"\002/msg NickServ HELP ACCESS\002 for more information.");
			}
		} else {
			if (!strcasecmp(args[1], "LIST")) {
				if (find_user(fd->nick)) {
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Password authentication required for that command.");
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Retry after typing \002/msg NickServ IDENTIFY\002 password.");
				} else {
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your nick isn't registered.");
				}
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Syntax: \002ACCESS { ADD | DEL | LIST } [\002mask\002]\002");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"\002/msg NickServ HELP ACCESS\002 for more information.");
			}
		}
	} else if (nargs >= 3) {
		if (!strcasecmp(args[1], "ADD")) {
			if (fd->registered & NS_IS_IDENTIFIED) {
				NS_AccessList *new_acl;
				NS_User *user;
				user = find_user(fd->nick);
				if (user == NULL) {
					fd->registered |= ~NS_IS_IDENTIFIED;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Your nick isn't registered.");
				} else {
					NS_AccessList *list = access_list;
					new_acl = (NS_AccessList *)malloc(sizeof(NS_AccessList));
					bzero(new_acl, sizeof(NS_AccessList));
					strncpy(new_acl->hostmask, args[2], 100);
					new_acl->user = user;
					while (list->next != NULL) list = list->next;
					list->next = new_acl;
					new_acl->prev = list;
					{
						char tmp[255];
						snprintf(tmp, 255, "\002%s\002 added to your access list.", new_acl->hostmask);
						fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
					}
				}
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
	 				"Password authentication required for that command.");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Retry after typing \002/msg NickServ IDENTIFY\002 password.");
			}
		} else if (!strcasecmp(args[1], "DEL")) {
			if (fd->registered & NS_IS_IDENTIFIED) {
				NS_AccessList *acl;
				NS_User *user;
				user = find_user(fd->nick);
				if (user == NULL) {
					fd->registered |= ~NS_IS_IDENTIFIED;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Your nick isn't registered.");
				} else {
					int removed = 0;
					acl = access_list;
					while (acl != NULL) {
						if (acl->user == user) {
							if (!strcasecmp(acl->hostmask, args[2])) {
								if (acl->prev == NULL && acl->next == NULL) {
									access_list = NULL;
									free(acl);
									acl = NULL;
								} else if (acl->prev == NULL && acl->next != NULL) {
									access_list = acl->next;
									(acl->next)->prev = NULL;
									free(acl);
									acl = NULL;
								} else if (acl->prev != NULL && acl->next == NULL) {
									(acl->prev)->next = NULL;
									free(acl);
									acl = NULL;
								} else {
									(acl->prev)->next = acl->next;
									(acl->next)->prev = acl->prev;
									free(acl);
									acl = NULL;
								}
								{
									char tmp[255];
									snprintf(tmp, 255, 
										"\002%s\002 deleted from your access list.", 
										args[2]);
									fprintf(fd->sok, NOTICE, s_NickServ, ns_userid,
										ns_hostname, fd->nick, tmp);
								}
								removed = 1;
							}
						}
						if (acl) acl = acl->next;
					}

					if (!removed) {
						char tmp[255];
						snprintf(tmp, 255, "\002%s\002 not found on your access list.", args[2]);
						fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
							tmp);
					}
				}
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
	 				"Password authentication required for that command.");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Retry after typing \002/msg NickServ IDENTIFY\002 password.");
			}
		} else if (!strcasecmp(args[0], "LIST")) {
			NS_AccessList *acl;
			NS_User *user;
			if (fd->registered & NS_IS_IDENTIFIED) {
				user = find_user(fd->nick);
				if (user == NULL) {
					fd->registered |= ~NS_IS_IDENTIFIED;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Your nick isn't registered.");
				} else {
					acl = access_list;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Access list:");
					while (acl != NULL) {
						if (acl->user == user) {
							fprintf(fd->sok, ":%s!%s@%s NOTICE %s :    %s\r\n",
								s_NickServ, ns_userid, ns_hostname, fd->nick,
								acl->hostmask);
						}
						acl = acl->next;
					}
				}
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
	 				"Password authentication required for that command.");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Retry after typing \002/msg NickServ IDENTIFY\002 password.");
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Syntax: \002ACCESS { ADD | DEL | LIST } [\002mask\002]\002");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP ACCESS\002 for more information.");
		}
	}
}

void ns_link(struct fd_list *fd, char **args, int nargs) {
	if (nargs < 3) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Syntax: \002LINK\002 nick password");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"\002/msg NickServ HELP LINK\002 for more information.");
	} else {
		NS_User *user = NULL, *me, *list;
		NS_LinkList *new_link;

		me = find_user(fd->nick);
		if (me == NULL) {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Your nick isn't registered.");
			return;
		}

		list = registered_nicks;
		while (list != NULL) {
			if (!strcasecmp(list->nick, args[1])) {
				user = list;
				list = NULL;
			}
			if(list) list=list->next;
		}

		if (user == NULL) {
			char tmp[255];
			snprintf(tmp, 255, "Nick \002%s\002 isn't registered.", args[1]);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			return;
		}

		if (!strcasecmp(user->nick, me->nick)) {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"You can't link to your current nick!");
			return;
		}

		if (!strcasecmp(user->pass, args[2])) {
			NS_LinkList *list = link_list;
			new_link = (NS_LinkList *)malloc(sizeof(NS_LinkList));
			new_link->user = user;
			strncpy(new_link->nick, fd->nick, 15);
			if (list == NULL) {
				link_list = new_link;
			} else {
				while (list->next != NULL) list=list->next;
				new_link->prev = list;
				list->next = new_link;
			}
			{
				char tmp[255];
				snprintf(tmp, 255, "Your nick has been linked to \002%s\002.", user->nick);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			}
			return;
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password incorrect.");
			return;
		}
	} 
}

void ns_set_password(struct fd_list *fd, char **args, int nargs) {
	NS_User *user;
	user = find_user(fd->nick);

	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (strlen(args[2]) < 5) {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Please try again with a more obscure password.  Passwords should be at least "
					"five characters long, should not be something easily guessed (e.g. your "
					"real name or your nick), and cannot contain the space or tab characters.");
				return;
			} else if (!strcasecmp(args[2], fd->nick)) {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Please try again with a more obscure password.  Passwords should be at least "
					"five characters long, should not be something easily guessed (e.g. your "
					"real name or your nick), and cannot contain the space or tab characters.");
				return;
			} else if (!valid_pass(args[2], fd->nick, fd->userid)) {
				char tmp[255];
				snprintf(user->pass, 15, "%s", args[2]);
				snprintf(tmp, 255, "Password changed to \002%s\002.", args[2]);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
				return;
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Please try again with a more obscure password.  Passwords must be at least 5 "
					"characters long, should not be something easily guessed (e.g. your "
					"real name or your nick), cannot consist of a string consisting of more than half "
					"repeating or consecutive characters (i.e. no passwords like 'abcdef' or 'abc000'), "
					"must contain at least either 1 uppercase or symbol character, or 2 numeric characters, "
					"i.e. 'blah12' or 'abQhty', and cannot contain the space or tab characters.");
				return;
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

struct ns_lang {
	int num;
	char *name;
};

typedef struct ns_lang NS_Languages;

NS_Languages language[] = {
	{ 1,	"English" },
	{ 0,	NULL }
};

int max_lang = 1;

void ns_set_language(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		int n;
		n = atoi(args[2]);
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (n < 1 || n > max_lang) {
				char tmp[255];
				snprintf(tmp, 255, "Unknown language number \002%i\002. "
					"Type \002/msg NickServ HELP SET LANGUAGE\002 for a list of languages.", n);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
				return;
			} else {
				NS_Languages *lang = language;
				while (lang->name != NULL) {
					if (lang->num == n) {
						char tmp[255];
						snprintf(tmp, 255, "Language changed to \002%s\002.", lang->name);
						fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
						user->language = n;
						return;
					}
					lang++;
				}

				{
					char tmp[255];
					snprintf(tmp, 255, "Unknown language number \002%i\002. "
						"Type \002/msg NickServ HELP SET LANGUAGE\002 for a list of languages.", n);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
				}
				return;
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

void ns_set_url(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (nargs == 2) {
				bzero(user->url, 100);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"URL unset.");
			} else {
				bzero(user->url, 100);
				strncpy(user->url, args[2], 100);
				{
					char tmp[255];
					snprintf(tmp, 255, "URL changed to \002%s\002.", user->url);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
				}
			}
			return;
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

void ns_set_email(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (nargs == 2) {
				bzero(user->email, 100);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"E-mail unset.");
			} else {
				bzero(user->email, 100);
				strncpy(user->email, args[2], 100);
				{
					char tmp[255];
					snprintf(tmp, 255, "E-mail changed to \002%s\002.", user->email);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
				}
			}
			return;
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

void ns_set_kill(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (!strcasecmp(args[2], "ON")) {
				user->kill = 1;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Kill protection is now \002ON\002.");
				return;
			} else if (!strcasecmp(args[2], "QUICK")) {
				user->kill = 2;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Kill protection is now \002ON\002, with a reduced kill delay.");
				return;
			} else if (!strcasecmp(args[2], "IMMED")) {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"The \002IMMED\002 option is not availiable on this network.");
				return;
			} else if (!strcasecmp(args[2], "OFF")) {
				user->kill = 0;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Kill protection is now \002OFF\002.");
				return;
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Syntax: \002SET KILL {ON | QUICK | OFF}\002");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"\002/msg NickServ HELP SET KILL\002 for more information.");
				return;
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

void ns_set_secure(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (!strcasecmp(args[2], "ON")) {
				user->secure = 1;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Secure option is now \002ON\002.");
				return;
			} else if (!strcasecmp(args[2], "OFF")) {
				user->secure = 0;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Secure option is now \002OFF\002.");
				return;
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Syntax: \002SET SECURE {ON | OFF}\002");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"\002/msg NickServ HELP SET SECURE\002 for more information.");
				return;
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

void ns_set_private(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (!strcasecmp(args[2], "ON")) {
				user->private = 1;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Private option is now \002ON\002.");
				return;
			} else if (!strcasecmp(args[2], "OFF")) {
				user->private = 0;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Private option is now \002OFF\002.");
				return;
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Syntax: \002SET PRIVATE {ON | OFF}\002");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"\002/msg NickServ HELP SET PRIVATE\002 for more information.");
				return;
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

void ns_set_hide(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	} else {
		if (fd->registered & NS_IS_IDENTIFIED) {
			if (!strcasecmp(args[3], "ON")) {
				if (!strcasecmp(args[2], "EMAIL")) {
					user->hide.email = 1;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your E-mail address will now be hidden from NickServ INFO displays.");
					return;
				} else if (!strcasecmp(args[2], "USERMASK")) {
					user->hide.usermask = 1;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your last seen user@host mask will now be hidden from NickServ INFO displays.");
					return;
				} else if (!strcasecmp(args[2], "QUIT")) {
					user->hide.quit = 1;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your last quit message will now be hidden from NickServ INFO displays.");
					return;
				} else {
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Syntax: \002SET HIDE {EMAIL | USERMASK | QUIT} {ON | OFF}");
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"\002/msg NickServ HELP SET HIDE\002 for more information.");
					return;
				}
			} else if (!strcasecmp(args[3], "OFF")) {
				if (!strcasecmp(args[2], "EMAIL")) {
					user->hide.email = 0;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your E-mail address will now be shown in NickServ INFO displays.");
					return;
				} else if (!strcasecmp(args[2], "USERMASK")) {
					user->hide.usermask = 0;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your last seen user@host mask will now be shown in NickServ INFO displays.");
					return;
				} else if (!strcasecmp(args[2], "QUIT")) {
					user->hide.quit = 0;
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Your last quit message will now be shown in NickServ INFO displays.");
					return;
				} else {
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Syntax: \002SET HIDE {EMAIL | USERMASK | QUIT} {ON | OFF}");
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"\002/msg NickServ HELP SET HIDE\002 for more information.");
					return;
				}
			} else {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Syntax: \002SET HIDE {EMAIL | USERMASK | QUIT} {ON | OFF}\002");
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"\002/msg NickServ HELP SET HIDE\002 for more information.");
				return;
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"Password authentication required for that command.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
				"\002/msg NickServ HELP SET\002 for more information.");
			return;
		}
	}
}

struct ns_set_options {
	char *option;
	void (*handler)(struct fd_list *, char **, int);
	int nargs;
};

typedef struct ns_set_options NS_SetOptions;

NS_SetOptions opts[] = {
	{ "PASSWORD",	ns_set_password,	3 },
	{ "LANGUAGE",	ns_set_language,	3 },
	{ "URL",	ns_set_url,		2 },
	{ "EMAIL",	ns_set_email,		2 },
	{ "KILL",	ns_set_kill,		3 },
	{ "SECURE",	ns_set_secure,		3 },
	{ "PRIVATE",	ns_set_private,		3 },
	{ "HIDE",	ns_set_hide,		4 },
	{ NULL,		NULL,			1 }
};

void ns_set(struct fd_list *fd, char **args, int nargs) {
	NS_SetOptions *p;
	p = opts;

	if (nargs > 1) {
		while (p->option != NULL) {
			if (!strcasecmp(p->option, args[1])) {
				if (nargs >= p->nargs) {
					p->handler(fd, args, nargs);
					return;
				} else {
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"Syntax: \002SET\002 option parameters");
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
						"\002/msg NickServ HELP SET\002 for more information.");
					return;
				}
			}
			p++;
		}

		{
			char tmp[255];
			snprintf(tmp, 255, "Unknown SET option \002%s\002.", args[1]);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
		}
		return;
	} else {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Syntax: \002SET\002 option parameters");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"\002/msg NickServ HELP SET\002 for more information.");
		return;
	}
}

void ns_drop(struct fd_list *fd, char **args, int nargs) {
	NS_User *user = find_user(fd->nick);
	if (user == NULL) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nick isn't registered.");
		return;
	}

	if (! (fd->registered & NS_IS_IDENTIFIED)) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Password authentication required for that command.");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"\002/msg NickServ HELP DROP\002 for more information.");
		return;
	}

	if (!strcasecmp(fd->nick, user->nick)) {
		{
			NS_AccessList *acl = access_list;
			while (acl != NULL) {
				if (acl->user == user) {
					if (acl->prev == NULL && acl->next == NULL) {
						access_list = NULL;
						free(acl);
						acl = NULL;
					} else if (acl->prev == NULL && acl->next != NULL) {
						access_list = acl->next;
						(acl->next)->prev = NULL;
						free(acl);
						acl = access_list;
					} else if (acl->prev != NULL && acl->next == NULL) {
						(acl->prev)->next = NULL;
						free(acl);
						acl = NULL;
					} else if (acl->prev != NULL && acl->next != NULL) {
						NS_AccessList *p = acl->next;
						(acl->prev)->next = acl->next;
						(acl->next)->prev = acl->prev;
						free(acl);
						acl = p;
					}
				} else {
					acl = acl->next;
				}
			}
		}
		{
			NS_LinkList *links;
			links = link_list;
			while (links != NULL) {
				if (links->user == user) {
					if (links->prev == NULL && links->next == NULL) {
						link_list = NULL;
						free(links);
						links = NULL;
					} else if (links->prev == NULL && links->next != NULL) {
						link_list = links->next;
						(links->next)->prev = NULL;
						free(links);
						links = link_list;
					} else if (links->prev != NULL && links->next == NULL) {
						(links->prev)->next = NULL;
						free(links);
						links = NULL;
					} else if (links->prev != NULL && links->next != NULL) {
						NS_LinkList *p = links->next;
						(links->prev)->next = links->next;
						(links->next)->prev = links->prev;
						free(links);
						links = p;
					}
				} else {
					links = links->next;
				}
			}
		}
		if (user->prev == NULL && user->next == NULL) {
			registered_nicks = NULL;
			free(user);
		} else if (user->prev == NULL && user->next != NULL) {
			registered_nicks = user->next;
			(user->next)->prev = NULL;
			free(user);
		} else if (user->prev != NULL && user->next == NULL) {
			(user->prev)->next = NULL;
			free(user);
		} else if (user->prev != NULL && user->next != NULL) {
			(user->prev)->next = user->next;
			(user->next)->prev = user->prev;
			free(user);
		}
		fd->registered = 1;
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
			"Your nickname has been dropped.");
		return;
	} else {
		NS_LinkList *link = link_list;
		while (link != NULL) {
			if (!strcasecmp(link->nick, fd->nick)) {
				if (!link->prev && !link->next) {
					link_list = NULL;
					free(link);
				} else if (link->prev && !link->next) {
					(link->prev)->next = NULL;
					free(link);
				} else if (!link->prev && link->next) {
					(link->next)->prev = NULL;
					link_list = link->next;
					free(link);
				} else if (link->prev && link->next) {
					(link->prev)->next = link->next;
					(link->next)->prev = link->prev;
					free(link);
				}
				fd->registered = 1;
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick,
					"Your nickname has been dropeed.");
				return;
			}
			link = link->next;
		}
	}
}

void ns_help(struct fd_list *fd, char **args, int nargs) {
	if (nargs > 1) {
		if (!strcasecmp(args[1], "REGISTER")) {
			char	*line1  = "Syntax: \002REGISTER\002 password",
				*line2  = " ",
				*line3  = "Registers your nickname in the NickServ database.  Once",
				*line4  = "your nick is registered, you can use the \002SET\002 and \002ACCESS\002",
				*line5  = "commands to configure your nick's settings as you like",
				*line6  = "them.  Make sure you remember the password you use when",
				*line7  = "registering - you'll need it to make changes to your nick",
				*line8  = "later. (Note that \002case matters!  FIDO\002, \002Fido\002, and \002fido\002",
				*line9  = "are all different passwords!)",
				*line10 = " ",
				*line11 = "Guidelines on choosing passwords:",
				*line12 = " ",
				*line13 = "Passwords should not be easily guessable.  For example,",
				*line14 = "using your real name as a password is a bad idea.  Using",
				*line15 = "your nickname as a password is a much worse idea ;) and,",
				*line16 = "in fact, NickServ will not allow it. Also, short",
				*line17 = "passwords are vulnerable to trial-and-error searches, so",
				*line18 = "you should choose a password at least 5 characters long.",
				*line19 = "Finally, the space character cannot be used in passwords.";
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line8);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line9);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line10);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line11);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line12);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line13);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line14);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line15);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line16);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line17);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line18);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line19);
		} else if (!strcasecmp(args[1], "IDENTIFY")) {
			char	*line1 = "Syntax: \002IDENTIFY\002 password",
				*line2 = " ",
				*line3 = "Tells NickServ that you really are the owner of this",
				*line4 = "nick.  Many commands require you to authenticate yourself",
				*line5 = "with this command before you use them.  The password",
				*line6 = "should be the same one you sent with the \002REGISTER\002",
				*line7 = "command.";
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
		} else if (!strcasecmp(args[1], "ACCESS")) {
			char	*line1  = "Syntax: \002ACCESS ADD\002 mask",
				*line2  = "        \002ACCESS DEL\002 mask",
				*line3  = "        \002ACCESS LIST\002",
				*line4  = " ",
				*line5  = "Modifies or displays the access list for your nick.  This",
				*line6  = "is the list of addresses which will be automatically",
				*line7  = "recognized by NickServ as allowed to use the nick.  If",
				*line8  = "you want to use the nick from a different address, you",
				*line9  = "need to send an \002IDENTIFY\002 command to make NickServ",
				*line10 = "recognize you.",
				*line11 = " ",
				*line12 = "Examples:",
				*line13 = " ",
				*line14 = "    \002ACCESS ADD achurch@*.dragonfire.net\002",
				*line15 = "        Allows access to user \002achurch\002 from any machine in",
				*line16 = "        the \002dragonfire.net\002 domain.",
				*line17 = " ",
				*line18 = "    \002ACCESS DEL achurch@*.dragonfire.net\002",
				*line19 = "        Reverses the previous command.",
				*line20 = " ",
				*line21 = "    \002ACCESS LIST\002",
				*line22 = "        Displays the current access list.";
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line8);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line9);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line10);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line11);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line12);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line13);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line14);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line15);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line16);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line17);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line18);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line19);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line20);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line21);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line22);
		} else if (!strcasecmp(args[1], "LINK")) {
			char 	*line1  = "Syntax: \002LINK\002 nick password",
				*line2  = " ",
				*line3  = "Links your nickname to another, effectively making the nick",
				*line4  = "you are using an alias for the given nick.  When",
				*line5  = "you give this command, the access list for the nick you are",
				*line6  = "using is deleted and replaced by that of the nick you are",
				*line7  = "linking to; all memos for your current nick are added to",
				*line8  = "the list of memos for the target nick (this may cause the",
				*line9  = "nick to exceed it's limit of memos, in which case you will",
				*line10 = "need to delete some before you can receive more for either",
				*line11 = "nick).",
				*line12 = " ",
				*line13 = "Once the link has been established, your nick will be",
				*line14 = "transparently converted into the target nick everywhere in",
				*line15 = "Services, except for the NickServ \002UNLINK\002 and \002DROP\002 commands.  For",
				*line16 = "example, you can use either nick to read the same set of",
				*line17 = "memos (and memos sent to either nick will go into the same",
				*line18 = "list).  If you identify for one nick, you will",
				*line19 = "automatically be identified for the other as well.  If",
				*line20 = "either nick is on a channel's access list, then both nicks",
				*line21 = "will get the same access priveliges.",
				*line22 = " ",
				*line23 = "In order to use this command, you must identify for your",
				*line24 = "current nick (using the \002IDENTIFY\002 command), and you must",
				*line25 = "supply the password for the nick you wish to link to.";
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line8);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line9);	
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line10);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line11);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line12);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line13);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line14);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line15);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line16);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line17);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line18);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line19);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line20);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line21);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line22);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line23);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line24);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line25);
		} else if (!strcasecmp(args[1], "SET")) {
			if (nargs > 2) {
				if (!strcasecmp(args[2], "PASSWORD")) {
					char 	*line1 = "Syntax: \002SET PASSWORD\002 new-password",
						*line2 = " ",
						*line3 = "Changes the password you use to identify as the nick's",
						*line4 = "owner.";
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
				} else if (!strcasecmp(args[2], "LANGUAGE")) {
					char	*line1 = "Syntax: \002SET LANGUAGE\002 number",
						*line2 = " ",
						*line3 = "Changes the language Services uses when sending messages to",
						*line4 = "you (for example, when replying to a command you send).",
						*line5 = "\002number\002 should be chosen from the following list of",
						*line6 = "supported languages:",
						*line7 = "     1) English";
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
				} else if (!strcasecmp(args[2], "URL")) {
					char	*line1 = "Syntax: \002SET URL\002 url",
						*line2 = " ",
						*line3 = "Associates the given URL with your nickname.  This URL",
						*line4 = "will be displayed whenever someone requests information",
						*line5 = "on your nick with the \002INFO\002 command.";
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
				} else if (!strcasecmp(args[2], "EMAIL")) {
					char	*line1 = "Syntax: \002SET EMAIL\002 address",
						*line2 = " ",
						*line3 = "Associates the given E-mail address with your nickname.",
						*line4 = "This information will be displayed whenever someone requests",
						*line5 = "information on the channel with the \002INFO\002 command.";
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
				} else if (!strcasecmp(args[2], "KILL")) {
					char	*line1  = "Syntax: \002SET KILL {ON | QUICK | IMMED | OFF}\002",
						*line2  = " ",
						*line3  = "Turns the automatic kill protection option for your nick",
						*line4  = "on or off.  With kill protection on, if another user",
						*line5  = "tries to take your nick, they will be given one minute to",
						*line6  = "change to another nick, after which they will be forcibly",
						*line7  = "removed from IRC by NickServ.",
						*line8  = " ",
						*line9  = "If you select \002QUICK\002, the user will be given only 20 seconds",
						*line10 = "to change nicks instead of the usual 60.  If you select",
						*line11 = "\002IMMED\002, the user will be killed immediately \002without\002 being",
						*line12 = "warned first or given a chance to change their nick; please",
						*line13 = "do not use this option unless necessary.  Also, your",
						*line14 = "network's administrators may have disabled this option.";
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line8);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line9);	
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line10);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line11);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line12);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line13);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line14);
				} else if (!strcasecmp(args[1], "SECURE")) {
					char 	*line1 = "Syntax: \002SET SECURE {ON | OFF}\002",
						*line2 = " ",
						*line3 = "Turns NickServ's security features on or off for your",
						*line4 = "nick.  With \002SECURE\002 set, you must enter your password",
						*line5 = "before you will be recognized as owner of the nick,",
						*line6 = "regardless of whether your address is on the access",
						*line7 = "list.  However, if you are on the access list, NickServ",
						*line8 = "will not auto-kill you regardless of the setting of the",
						*line9 = "\002KILL\002 option.";
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line8);
					fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line9);	
				} 
			}
		} else {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "Oops.");
		}
	} else {
		char *line1  = "\002NickServ\002 allows you to \"register\" a nickname and",
		     *line2  = "prevent others from using them. If the nick is not used",
		     *line3  = "for 30 days, the registration will expire.  The following",
		     *line4  = "commands allow registration and maintenance of",
		     *line5  = "nicknames; to use them, use \002/MSG NickServ <command>\002.",
		     *line6  = "For more information on a specific command, type",
		     *line7  = "\002/MSG NickServ HELP <command>\002.",
		     *line8  = " ",
		     *line9  = "   REGISTER   Register a nickname",
		     *line10 = "   IDENTIFY   Identify yourself with your password",
		     *line11 = "   ACCESS     Modify the list of authorized addresses",
		     *line12 = "   LINK       Link another nickname to your primary one",
	             *line13 = "   SET        Set options, including kill protection",
        	     *line14 = "   DROP       Cancel the registration of a nickname",
	             *line15 = "   RECOVER    Reclaim your nick after another user has taken it",
		     *line18 = " ",
	             *line19 = "Other commands: UNLINK, GHOST, INFO, LIST, STATUS",
        	     *line20 = " ",
	             *line21 = "\002NOTICE:\x02 This service is intended to provide a way for",
        	     *line22 = "IRC users to ensure their identity is not compromised.",
        	     *line23 = "It is \002NOT\002 intended to facilitate \"stealing\" of",
        	     *line24 = "nicknames or other malicious actions. Abuse of NickServ",
        	     *line25 = "will result in, at minimum, loss of the abused nickname(s).";
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line1);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line2);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line3);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line4);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line5);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line6);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line7);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line8);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line9);	
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line10);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line11);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line12);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line13);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line14);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line15);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line18);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line19);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line20);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line21);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line22);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line23);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line24);
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, line25);
	}
};

void ns_info(struct fd_list *fd, char **args, int nargs) {
	if (nargs < 2) {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "Syntax: \002INFO nick [ALL]\002");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, 
			"\002/msg NickServ HELP INFO\002 for more information.");
		return;
	} else {
		NS_User *user = find_user(args[1]);
		if (user == NULL) {
			char tmp[255];
			snprintf(tmp, 255, "Nick \002%s\002 isn't registered.", args[1]);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			return;
		} else {
			char tmp[255];
			char *nick;
			bzero(tmp, 255);
			if (!strcasecmp(args[1], user->nick)) {
				nick = user->nick;
			} else {
				nick = args[1];
			}
			snprintf(tmp, 255, "%s is %s", nick, user->last_realname);
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			{
				struct fd_list *p;
				int is_online = 0;
				p = __head->next;
				while (p != NULL) {
					if (!strcasecmp(p->nick, nick)) {
						if(fd->registered & NS_IS_IDENTIFIED) {
							is_online = 1;
							p = NULL;
						} else {
							p = NULL;
						}
					}
					if(p) p = p->next;
				}

				if (user->last_mask[0] != 0) {
					if(is_online) {
						snprintf(tmp, 255, "   Is online from: %s", user->last_mask);
						fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
					} else {
						snprintf(tmp, 255, "Last seen address: %s", user->last_mask);
						fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
						snprintf(tmp, 255, "   Last seen time: %s", ctime((time_t *) &(user->last_seen_at)));
						fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
					}
				}
			}
			snprintf(tmp, 255, "  Time registered: %s", ctime((time_t *) &(user->registered_at)));
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			if (user->last_quit[0] != 0) {
				snprintf(tmp, 255, "Last quit message: %s", user->last_quit);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			}
			{
				char options[255];
				bzero(options, 255);
				if (user->kill) {
					snprintf(options, 255, "Kill protection");
				}
				if (user->secure) {
					if (options[0] == 0) {
						snprintf(options, 255, "Security");
					} else {
						snprintf(options, 255, "%s, Security", options);
					}
				}
				if (user->private) {
					if (options[0] == 0) {
						snprintf(options, 255, "Private");
					} else {
						snprintf(options, 255, "%s, Private", options);
					}
				}
				if (options[0] == 0) {
					snprintf(options, 255, "None");
				}

				snprintf(tmp, 255, "          Options: %s", options);
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, tmp);
			}
			return;
		}
	}
}

void ns_version(struct fd_list *fd, char **args, int nargs) {
	fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname,
		fd->nick, "\001VERSION Services v0.3 for ircd 0.2\001");
	return;
}

int guest_num = 0;

extern int *die, *quit;
extern char *parse(char *, struct fd_list *, int *, int *);

/* recover <nick> <pass> */
void ns_recover(struct fd_list *fd, char **args, int nargs) {
	if (nargs >= 3) {
		struct fd_list *u = __head->next;

		if (!strcasecmp(args[1], fd->nick)) {
			fprintf(fd->sok, ":%s!%s@%s NOTICE %s :You can't recover yourself!\r\n", s_NickServ, ns_userid,
				ns_hostname, fd->nick);
			return;
		}

		while(u) {
			if (!strcasecmp(u->nick, args[1])) goto found_user;
			u = u->next;
		}
		goto no_such_user;

		found_user: {
			NS_User *ul = find_user(args[1]);
			if (!ul) goto not_registered;
			
			if (!strcmp(args[2], ul->pass)) goto pass_ok;
			goto bad_pass;

			pass_ok: {
				/* change u->nick to a guest nick */
				char tmpnick[21]; int nuh, *my_quit, *my_die;
				snprintf(tmpnick, 21, "NICK Guest%i\r", guest_num++);
				fprintf(u->sok, ":%s!%s@%s NOTICE %s :This nick was recovered by %s!\r\n", s_NickServ,
					ns_userid, ns_hostname, u->nick, fd->nick);
				my_quit = quit;
				my_die = die;
				parse(tmpnick, u, &nuh, &nuh);
				/* change fd->nick to the recovered nick */
				snprintf(tmpnick, 21, "NICK %s\r", args[1]);
				parse(tmpnick, fd, &nuh, &nuh);
				my_quit = quit;
				my_die = die;
				fd->registered |= NS_IS_IDENTIFIED;
				u->registered &= ~NS_IS_IDENTIFIED;
				return;
			}

			bad_pass: {
				fprintf(fd->sok, ":%s!%s@%s NOTICE %s :Bad password!\r\n", s_NickServ, ns_userid,
					ns_hostname, fd->nick);
				return;
			}
	
			not_registered: {
				fprintf(fd->sok, ":%s!%s@%s NOTICE %s :The nickname \002%s\002 is not registered!\r\n",
					s_NickServ, ns_userid, ns_hostname, fd->nick, args[1]);
				return;
			}
		}

		no_such_user: {
			fprintf(fd->sok, ":%s!%s@%s NOTICE %s :The nickname \002%s\002 is not in use!\r\n",
				s_NickServ, ns_userid, ns_hostname, fd->nick, args[1]);
			return;
		}
	} else {
		fprintf(fd->sok, ":%s!%s@%s NOTICE %s :Syntax: \002RECOVER\002 nickname \002[\002password\002]\002\r\n",
			s_NickServ, ns_userid, ns_hostname, fd->nick);
		fprintf(fd->sok, ":%s!%s@%s NOTICE %s :\002/msg NickServ HELP RECOVER\002 for more information.\r\n",
			s_NickServ, ns_userid, ns_hostname, fd->nick);
		return;
	}
}

extern void m_kill(struct fd_list *, char **, int);

void ns_ghost(struct fd_list *fd, char **args, int nargs) {
	if (nargs >= 3) {
		NS_User *u = find_user(args[1]);
		if (!u) {
			fprintf(fd->sok, ":%s!%s@%s NOTICE %s :The nickname \002%s\002 has not been registered!\r\n",
				s_NickServ, ns_userid, ns_hostname, fd->nick, args[1]);
			return;
		}

		if (!strcasecmp(fd->nick, args[1])) {
			fprintf(fd->sok, ":%s!%s@%s NOTICE %s :You can't GHOST yourself!\r\n", s_NickServ, ns_userid, ns_hostname, fd->nick);
			return;
		}

		if (!strcmp(u->pass, args[2])) {
			struct fd_list p;
			char kmsg[100];
			char *argv[3];
			int argc = 3;
			bzero(&p, sizeof(p));
			snprintf(p.nick, 15, "%s", s_NickServ);
			snprintf(p.userid, 15, "%s", ns_userid);
			snprintf(p.hostname, 15, "%s", ns_hostname);
			bzero(kmsg, 100);
			argv[0] = "KILL";
			argv[1] = args[1];
			argv[2] = kmsg;
			snprintf(kmsg, 100, "GHOST command used by %s", fd->nick);
			m_kill(&p, argv, argc);
			return;
		} else {
			fprintf(fd->sok, ":%s!%s@%s NOTICE %s :Incorrect password!\r\n", s_NickServ, ns_userid, ns_hostname, fd->nick);
			return;
		}
	} else {
		fprintf(fd->sok, ":%s!%s@%s NOTICE %s :Syntax: \002GHOST nick password\002\r\n", s_NickServ, ns_userid, ns_hostname,
			fd->nick);
		fprintf(fd->sok, ":%s!%s@%s NOTICE %s :\002/msg NickServ HELP GHOST\002 for more information.\r\n", s_NickServ, ns_userid,
			ns_hostname, fd->nick);
		return;
	}
}

NS_Functions fn_list[] = {
	{ "REGISTER",	ns_register,	0 },
	{ "IDENTIFY",	ns_identify,	0 },
	{ "ACCESS",	ns_access,	1 },
	{ "LINK",	ns_link,	1 },
	{ "SET",	ns_set,		1 },
	{ "DROP",	ns_drop,	1 },
	{ "RECOVER",	ns_recover,	0 },
	{ "UNLINK",	ns_dummy,	1 },
	{ "GHOST",	ns_ghost,	0 },
	{ "INFO",	ns_info,	0 },
	{ "LIST",	ns_dummy,	0 },
	{ "STATUS",	ns_dummy,	0 },
	{ "HELP",	ns_help,	0 },
	{ "\001VERSION\001", ns_version, 0 },
	{ NULL,		NULL,		0 }
};

int num_funcs = 14;

void nickserv_msg_handle(struct fd_list *fd, char **av, int ac) {
	char **args, *input, *tmp;
	int i, nargs;

	if (ac > 2) {
		input = av[2];
		tmp = input;
		args = NULL;
		nargs = 0;
		while (*input != 0) {
			if (*input == ' ') {
				args = (char **)realloc(args, sizeof(char **) * ++nargs);
				args[nargs - 1] = tmp;
				*input = 0;
				input++;
				tmp = input;
			} else {
				input++;
			}
		}
		args = (char **)realloc(args, sizeof(char **) * ++nargs);
		args[nargs - 1] = tmp;
		args = (char **)realloc(args, sizeof(char **) * nargs+1);
		args[nargs] = NULL;
	} else {
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "I need a command to parse.");
		fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, 
			"\x02/MSG NickServ HELP\x02 for more information.");
		return;
	}

	for (i=0; i < num_funcs; i++) {
		if (!strcasecmp(args[0], fn_list[i].cmd)) {
			fn_list[i].function(fd, args, nargs);
			free(args);
			return;
		}
	}

	fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "Unknown command.");
	fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, 
		"\x02/MSG NickServ HELP\x02 for more information.");
	free(args);
	return;
}

struct os_func {
	char *cmd;
	void (*function)(struct fd_list *, char **, int);
	int nargs;
	int need_oper;
	int need_admin;
	int need_root;
};

typedef struct os_func OS_Functions;

void os_global(struct fd_list *fd, char **args, int nargs) {
	char *message = NULL;
	if (nargs > 1) {
		int i, len = 0;
		for (i=1; i<nargs; i++) {
			len = len + strlen(args[i]) + 1;
		}

		message = (char *) malloc(len);
		len = 0;
		for (i=1; i<nargs; i++) {
			sprintf(&(message[len]), "%s ", args[i]);
			len = len + strlen(args[i]) + 1;
		}

		{
			struct fd_list *users;
			users = __head->next;
			while (users != NULL) {
				fprintf(users->sok, NOTICE, "Global", "global", "services.net", users->nick, message);
				users = users->next;
			}
		}

		free(message);	
	} else {
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "Syntax: \002GLOBAL message\002");
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick,
			"\002/msg OperServ HELP GLOBAL\002 for more information.");
	}
	return;
}

void os_stats(struct fd_list *fd, char **args, int nargs) {
	if (nargs > 1) {
		if (!strcasecmp(args[1], "AKILL")) {
			char tmp[255];
			snprintf(tmp, 255, "Current number of AKILLs: \002%i\002", 0);
			fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, tmp);
			snprintf(tmp, 255, "Default AKILL expiry time: \002%i days\002", 30);
			fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, tmp);
			return;
		} else if (!strcasecmp(args[1], "ALL")) {
			fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick,
				"OperServ \002STATS ALL\002 is broken right now.");
			return;
		} else {
			fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick,
				"Syntax: \002STATS [AKILL | ALL]\002");
			fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick,
				"\002/msg OperServ HELP STATS\002 for more information.");
			return;
		}
	} else {
		char tmp[255];
		struct fd_list *list = __head ->next;
		int p = 0, ops = 0;
		while (list != NULL) {
			p++;
			if (list->oper) ops++;
			list = list->next;
		}
		snprintf(tmp, 255, "Current users: \002%i\002 (\002%i\002 ops)", p, ops);
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, tmp);
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "Maximum users: Unknown");
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "Services up for a while now.");
		return;
	}
}

void os_dummy(struct fd_list *fd, char **args, int nargs) {
	char tmp[255];
	snprintf(tmp, 255, "OperServ \002%s\002 doesn't work yet..", args[0]);
	fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, tmp);
	return;
}

#define os_oper os_dummy
#define os_admin os_dummy
#define os_mode os_dummy
#define os_kick os_dummy
#define os_akill os_dummy
#define os_session os_dummy
#define os_exception os_dummy
#define os_killclones os_dummy
#define os_logonnews os_dummy
#define os_opernews os_dummy
#define os_jupe os_dummy
#define os_raw os_dummy
#define os_set os_dummy
#define os_update os_dummy
#define os_quit os_dummy
#define os_restart os_dummy

void os_help (struct fd_list *fd, char **av, int ac) {
	fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "I don't know how to do /operserv HELP yet.");
	return;
}

OS_Functions os_func_list[] = {
	{ "GLOBAL",	os_global,	1,	0,	0 },
	{ "STATS",	os_stats,	1,	0,	0 },
	{ "OPER",	os_oper,	1,	0,	0 },
	{ "ADMIN",	os_admin,	1,	0,	0 },
	{ "MODE",	os_mode,	1,	0,	0 },
	{ "KICK",	os_kick,	1,	0,	0 },
	{ "AKILL",	os_akill,	1,	0,	0 },
	{ "SESSION",	os_session,	0,	1,	0 },
	{ "EXCEPTION",	os_exception,	0,	1,	0 },
	{ "KILLCLONES",	os_killclones,	0,	1,	0 },
	{ "LOGONNEWS",	os_logonnews,	0,	1,	0 },
	{ "OPERNEWS",	os_opernews,	0,	1,	0 },
	{ "JUPE",	os_jupe,	0,	1,	0 },
	{ "RAW",	os_raw,		0,	1,	0 },
	{ "SET",	os_set,		0,	1,	0 },
	{ "UPDATE",	os_update,	0,	1,	0 },
	{ "QUIT",	os_quit,	0,	1,	0 },
	{ "RESTART",	os_restart,	0,	1,	0 },
	{ "HELP",	os_help,	0,	0,	0 },
	{ NULL,		NULL,		0,	0,	0 }
};

int os_num_funcs = 19;

void operserv_msg_handle(struct fd_list *fd, char **av, int ac) {
	char **args, *input, *tmp;
	int i, nargs;

	if (ac > 2) {
		input = av[2];
		tmp = input;
		args = NULL;
		nargs = 0;
		while (*input != 0) {
			if (*input == ' ') {
				args = (char **)realloc(args, sizeof(char **) * ++nargs);
				args[nargs - 1] = tmp;
				*input = 0;
				input++;
				tmp = input;
			} else {
				input++;
			}
		}
		args = (char **)realloc(args, sizeof(char **) * ++nargs);
		args[nargs - 1] = tmp;
		args = (char **)realloc(args, sizeof(char **) * nargs+1);
		args[nargs] = NULL;
	} else {
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "I need a command to parse.");
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, 
			"\x02/MSG OperServ HELP\x02 for more information.");
		return;
	}

	if (!fd->oper) {
		fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "Access Denied - OperServ is only "
			"availiable to IRC Operators!");
		return;
	}

	for (i=0; i < os_num_funcs; i++) {
		if (!strcasecmp(args[0], os_func_list[i].cmd)) {
			os_func_list[i].function(fd, args, nargs);
			free(args);
			return;
		}
	}

	fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, "Unknown command.");
	fprintf(fd->sok, NOTICE, s_OperServ, os_userid, os_hostname, fd->nick, 
		"\x02/MSG OperServ HELP\x02 for more information.");
	free(args);
	return;
}

int version(struct fd_list *fd, char **av, int ac) {
	if (ac > 1) {
		char regex_buf[strlen(av[1])+2];
		regex_t *preg;
		int rval;

		preg = (regex_t *)malloc(sizeof(regex_t));

		sprintf(&(regex_buf[2]), "%s", av[1]);
		regex_buf[0] = '(';
		regex_buf[1] = ')';

		regcomp(preg, regex_buf, REG_ICASE|REG_NOSUB);
		
		rval = regexec(preg, "services.net", 0, NULL, 0);
		if (rval) { /* reg_nomatch */
			regfree(preg);
			return 1;
		} else { /* match found */
			regfree(preg);
			free(preg);
			
			fprintf(fd->sok, RPL_VERSION, "services.net", fd->nick, "0.3.", "services.net", "IRC Services 0.3 for ircd 0.2");
			return 0;
		}
	} else {
		return 1;
	}
}

int privmsg(struct fd_list *fd, char **av, int ac) {
	if (ac > 1) {
		if (!strcasecmp(av[1], s_NickServ)) {
			nickserv_msg_handle(fd, av, ac);
			return 0;
		} else if (!strcasecmp(av[1], s_OperServ)) {
			operserv_msg_handle(fd, av, ac);
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
	return 0;
}

int nick(struct fd_list *fd, char **av, int ac) {
	NS_User *user = find_user(av[1]);
	if (user == NULL) {
		return 1;
	} else {
		m_nick(fd, av, ac);
		fd->registered &= ~NS_IS_IDENTIFIED;
		if (user->secure || user->kill) {
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "This nickname is owned by someone else.");
			fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "If this is your nickname, please identify - \002/msg NickServ IDENTIFY [pass]\002");
			if (user->kill) {
				fprintf(fd->sok, NOTICE, s_NickServ, ns_userid, ns_hostname, fd->nick, "If you do not comply within 60 seconds, you will be disconnected.");
			}
		}
		return 0;
	}
}

int nickserv(struct fd_list *fd, char **av, int ac) {
	char **new_av; int new_ac;
	char *arg0 = "PRIVMSG",
	     *arg1 = "NickServ",
	     *arg2;
	int need_free = 0;
	if (ac > 2) {
		char tmp[255];
		int n;
		bzero(tmp, 255);
		for (n=1; n<ac; n++) {
			snprintf(tmp, 255, "%s %s", tmp, av[n]);
		}
		arg2 = (char *)malloc(strlen(tmp));
		strncpy(arg2, tmp, strlen(tmp));
		need_free = 1;
	} else {
		arg2 = av[1];
		need_free = 0;
	}
	new_av = (char **) malloc(sizeof(char **) * 4);
	new_av[0] = arg0;
	new_av[1] = arg1;
	new_av[2] = arg2;
	new_av[3] = NULL;
	
	new_ac = 3;
	privmsg(fd, new_av, new_ac);

	if (need_free) free(arg2);
	free(new_av);

	return 0;
}

void nickserv_whois_reply(struct fd_list *fd) {
	fprintf(fd->sok, RPL_WHOISUSER, server_name, fd->nick, s_NickServ, ns_userid, ns_hostname, ns_realname);
	fprintf(fd->sok, RPL_WHOISSERVER, server_name, fd->nick, s_NickServ, ns_server, ns_server_desc);
	fprintf(fd->sok, RPL_WHOISOPERATOR, server_name, fd->nick, s_NickServ);
	fprintf(fd->sok, RPL_ENDOFWHOIS, server_name, fd->nick, s_NickServ);
	return;
}

void operserv_whois_reply(struct fd_list *fd) {
	fprintf(fd->sok, RPL_WHOISUSER, server_name, fd->nick, s_OperServ, os_userid, os_hostname, os_realname);
	fprintf(fd->sok, RPL_WHOISSERVER, server_name, fd->nick, s_OperServ, os_server, os_server_desc);
	fprintf(fd->sok, RPL_WHOISOPERATOR, server_name, fd->nick, s_OperServ);
	fprintf(fd->sok, RPL_ENDOFWHOIS, server_name, fd->nick, s_OperServ);
}

int whois(struct fd_list *fd, char **av, int ac) {
	if (ac < 2) {
		__not_enough_parameters(fd, av[0]);
	} else {
		if (ac == 2) {
			if (!strcasecmp(av[1], s_NickServ)) {
				nickserv_whois_reply(fd);
			} else if (!strcasecmp(av[1], s_OperServ)) {
				operserv_whois_reply(fd);
			} else {
				__whois_reply(fd, av[1]);
			}
		} else {
			if (!strcasecmp(av[2], s_NickServ)) {
				nickserv_whois_reply(fd);
			} else if (!strcasecmp(av[2], s_OperServ)) {
				operserv_whois_reply(fd);
			} else {
				__whois_reply(fd, av[2]);
			}
		}
	}
	return 0;
}

int links(struct fd_list *fd, char **av, int ac) {
	fprintf(fd->sok, RPL_LINKS, server_name, fd->nick, server_name, server_name, 0, server_desc);
	fprintf(fd->sok, RPL_LINKS, server_name, fd->nick, ns_server, server_name, 1, ns_server_desc);
	fprintf(fd->sok, RPL_ENDOFLINKS, server_name, fd->nick, "*");
	return 0;
}

void nickserv_who(struct fd_list *fd) {
	fprintf(fd->sok, RPL_WHOREPLY, server_name, fd->nick, "*", ns_userid, ns_hostname, ns_server, s_NickServ, 'H', "*", 1,
		ns_realname);
	fprintf(fd->sok, RPL_ENDOFWHO, server_name, fd->nick, s_NickServ);
	return;
}

int who(struct fd_list *fd, char **av, int ac) {
	if (ac > 1) {
		if(av[1][0] == '#') {
			__who_channel(fd, av[1]);
		} else {
			if (!strcasecmp(av[1], s_NickServ)) {
				nickserv_who(fd);
			} else {
				__who_user(fd, av[1]);
			}
		}
	} else {
		__not_enough_parameters(fd, av[0]);
	}
	return 0;
}

void sigprof(int signum) {
	struct fd_list *u = __head->next;
	NS_User *q;
	while(u) {
		if ((q = find_user(u->nick)) != NULL) {
			if (!(u->registered & NS_IS_IDENTIFIED)) {
				if(q->kill) {
					struct fd_list tmp;	
					char *argv[4];
					argv[0] = "KILL";
					argv[1] = u->nick;
					argv[2] = "Never identified!";
					argv[3] = NULL;
					bzero(&tmp, sizeof(tmp));
					snprintf(tmp.nick, 15, "%s", s_NickServ);
					snprintf(tmp.userid, 15, "%s", ns_userid);
					snprintf(tmp.hostname, 50, "%s", ns_hostname);
					m_kill(&tmp, argv, 3);
				}
			}
		}
		u = u->next;
	}

	signal(SIGPROF, sigprof);
}
 
void _services_init_signals() {
	struct itimerval val;
	struct timeval tv;
	tv.tv_sec = 60;
	tv.tv_usec = 0;
	val.it_interval = tv;
	val.it_value = tv;
	setitimer(ITIMER_PROF, &val, NULL);
	signal(SIGPROF, sigprof);
}

void _init() {
	if (__head != NULL) {
		struct fd_list *list = __head->next;

		while (list != NULL) {
			if (list->registered) list->registered = 1;
			list = list->next;
		}
	}
	_services_init_signals();
}

void _fini() {
	{
		NS_User *list, *p;
		list = registered_nicks;
		while (list != NULL) {
			p = list->next;
			free(list);
			list = p;
		}
	}
	{
		NS_AccessList *list, *p;
		list = access_list;
		while (list != NULL) {
			p = list->next;
			free(list);
			list = p;
		}
	}
	{
		NS_LinkList *list, *p;
		list = link_list;
		while (list != NULL) {
			p = list->next;
			free(list);
			list = p;
		}
	}
	
}

