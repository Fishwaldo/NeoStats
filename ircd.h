/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
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
** $Id$
*/
#ifndef IRCD_H
#define IRCD_H

#define MAX_CMD_LINE_LENGTH		350

typedef void (*int_cmd_handler) (char *origin, char **argv, int argc);

typedef struct IntCommands{
	const char *name;
#ifdef GOTTOKENSUPPORT
	const char *token;
#endif
	int_cmd_handler function;
	int srvmsg;		/* Should this be a Server Message(1), or a User Message?(0) */
	int usage;
}IntCommands;

typedef struct aCtab{
	long mode;
	char flag;
	unsigned nickparam:1;	/* 1 = yes 0 = no */
	unsigned parameters:1;
	char sjoin;
} aCtab;

typedef struct {
	unsigned long umodes;
	char mode;
	int level;
} Oper_Modes;

extern Oper_Modes usr_mds[];
#ifdef GOTUSERSMODES
extern Oper_Modes susr_mds[];
#endif

extern IntCommands cmd_list[];
extern aCtab cFlagTab[33];

char* UmodeMaskToString(long Umode);
long UmodeStringToMask(char* UmodeString);
int init_services_bot (void);
void ns_usr_motd (char *nick, char **argv, int argc);
void ns_usr_admin (char *nick, char **argv, int argc);
void ns_usr_credits (char *nick, char **argv, int argc);
void ns_usr_stats (char *origin, char **argv, int argc);
void ns_usr_pong (char *origin, char **argv, int argc);

/* Defined in ircd specific files but common to all */
int SignOn_NewBot (const char *nick, const char *user, const char *host, const char *rname, long Umode);
void init_ircd (void);

#endif
