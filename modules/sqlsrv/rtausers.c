/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "rta.h"

/* @brief Returns the users server in text form that they are connected too
*/

void *display_server(void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->user->server->name;                        
}                        

void *display_umode(void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return UmodeMaskToString(data->user->Umode);
}

void *display_vhost(void *tbl, char *col, char *sql, void *row) 
{
	Client *u = row;
#if 0
	if (ircd_srv.features&FEATURE_UMODECLOAK) {
		/* Do we have a hidden host? */
		if(u->user->Umode & UMODE_HIDE) {
			return u->user->vhost;
		}
		return "*";	
	}
#endif
	return u->user->hostname;
}

void *display_smode(void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return SmodeMaskToString(data->user->Smode);
}

static char userschannellist[MAXCHANLENLIST];

void *display_chans(void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	lnode_t *cn;
	userschannellist[0] = '\0';
	cn = list_first(data->user->chans);
	while (cn != NULL) {
		strlcat(userschannellist, lnode_get(cn), MAXCHANLENLIST);
		strlcat(userschannellist, " ", MAXCHANLENLIST);
		cn = list_next(data->user->chans, cn);
	}
	return userschannellist;
}

void *display_user_hostname (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->user->hostname;                        
}                        

void *display_user_username (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->user->username;                        
}                        

void *display_user_is_away (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->user->is_away;                        
}                        

void *display_user_awaymsg (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->user->awaymsg; 
}                        

void *display_user_swhois (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->user->swhois; 
}                        

COLDEF neo_userscols[] = {
	{
		"users",
		"nick",
		RTA_STR,
		MAXNICK,
		offsetof(Client, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The nickname of the user"
	},
	{
		"users",
		"hostname",
		RTA_STR,
		MAXHOST,
		0,
		RTA_READONLY,
		display_user_hostname, 
		NULL,
		"The real Hostname of the user"
	},
	{
		"users",
		"ident",
		RTA_STR,
		MAXUSER,
		0,
		RTA_READONLY,
		display_user_username,
		NULL,
		"The ident portion of the users connection"
	},
	{
		"users",
		"realname",
		RTA_STR,
		MAXREALNAME,
		offsetof(Client, info),
		RTA_READONLY,
		NULL,
		NULL,
		"The users realname/info message"
	},
	{	
		"users",
		"vhost",
		RTA_STR,
		MAXHOST,
		0,
		RTA_READONLY,
		display_vhost,
		NULL,
		"The users Vhost, if the IRCd supports VHOSTS"
	},
	{	
		"users",
		"away",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_user_is_away,
		NULL,
		"Boolean variable indiciating if the user is away"
	},
	{	
		"users",
		"modes",
		RTA_STR,
		64, 				/* as defined in ircd.c */
		0,
		RTA_READONLY,
		display_umode,
		NULL,
		"the users umodes. Does not include SMODES."
	},
	{	
		"users",
		"smodes",
		RTA_STR,
		64,
		0,
		RTA_READONLY,
		display_smode,
		NULL,
		"the users Smodes, if the IRCd supports it.  Does not include UMODES."
	},
	{	
		"users",
		"connected",
		RTA_INT,
		sizeof(int),
		offsetof(Client, tsconnect),
		RTA_READONLY,
		NULL,
		NULL,
		"When the User Connected"
	},
	{	
		"users",
		"flags",
		RTA_INT,
		sizeof(int),
		offsetof(Client, flags),
		RTA_READONLY,
		NULL,
		NULL,
		"Flags for this user"
	},
	{	
		"users",
		"server",
		RTA_STR,
		MAXHOST,
		offsetof(Client, server),
		RTA_READONLY,
		display_server,
		NULL,
		"the users server"
	},
	{	
		"users",
		"channels",
		RTA_STR,
		MAXCHANLENLIST,
		0,
		RTA_READONLY,
		display_chans,
		NULL,
		"the users channels."
	},
	{	
		"users",
		"awaymsg",
		RTA_STR,
		MAXHOST,
		0,
		RTA_READONLY,
		display_user_awaymsg,
		NULL,
		"the users away message."
	},
	{	
		"users",
		"swhois",
		RTA_STR,
		MAXHOST,
		0,
		RTA_READONLY,
		display_user_swhois,
		NULL,
		"the users swhois."
	},

};

TBLDEF neo_users = {
	"users",
	NULL, 	/* for now */
	sizeof(Client),
	0,
	TBL_HASH,
	neo_userscols,
	sizeof(neo_userscols) / sizeof(COLDEF),
	"",
	"The list of users connected to the IRC network"
};

