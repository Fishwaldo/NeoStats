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

void *display_server_users (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return &data->server->users;
}                        

void *display_server_hops (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return &data->server->hops;                        
}                        

void *display_server_ping (void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return &data->server->ping;                        
}                        

void *display_server_uplink(void *tbl, char *col, char *sql, void *row) 
{
	Client *data = row;
	return data->uplinkname;
}                        

COLDEF neo_serverscols[] = {
	{
		"servers",
		"name",
		RTA_STR,
		MAXHOST,
		offsetof(Client, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the server linked to the IRC network"
	},
	{
		"servers",
		"hops",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_server_hops, 
		NULL,
		"The Number of hops away from the NeoStats Server"
	},
	{
		"servers",
		"users",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_server_users, 
		NULL,
		"The Number of users on server"
	},	
	{
		"servers",
		"connected",
		RTA_INT,
		sizeof(int),
		offsetof(Client, tsconnect),
		RTA_READONLY,
		NULL,
		NULL,
		"The time the server connected to the IRC network"
	},
	{
		"servers",
		"last_ping",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_server_ping,
		NULL,
		"The last ping time to this server from the NeoStats Server"
	},
	{
		"servers",
		"flags",
		RTA_INT,
		sizeof(int),
		offsetof(Client, flags),
		RTA_READONLY,
		NULL,
		NULL,
		"Flags that specify special functions for this Server"
	},
	{	
		"servers",
		"uplink",
		RTA_STR,
		MAXHOST,
		0,
		RTA_READONLY,
		display_server_uplink,
		NULL,
		"The uplink Server this server is connected to. if it = self, means the NeoStats Server"
	},
	{	
		"servers",
		"infoline",
		RTA_STR,
		MAXINFO,
		offsetof(Client, info),
		RTA_READONLY,
		NULL,
		NULL,
		"The description of this server"
	},
};

TBLDEF neo_servers = {
	"servers",
	NULL, 	/* for now */
	sizeof(Client),
	0,
	TBL_HASH,
	neo_serverscols,
	sizeof(neo_serverscols) / sizeof(COLDEF),
	"",
	"The list of Servers connected to the IRC network"
};

