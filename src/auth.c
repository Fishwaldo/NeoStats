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
#include "modules.h"
#include "dl.h"
#include "services.h"

static Module* AuthModList[NUM_MODULES];

int IsServiceRoot(Client * u)
{
	if ((match(config.rootuser.nick, u->name))
	&& (match(config.rootuser.user, u->user->username))
	&& (match(config.rootuser.host, u->user->hostname))) {
		return (1);
	}
	return (0);
}

int UserAuth(Client * u)
{
	int newauthlvl = 0;
	int authlvl = 0;
	int i;
	
	if(IsServiceRoot(u)) {
		return NS_ULEVEL_ROOT;
	} 
	for(i = 0; i < NUM_MODULES; i++)
	{
		authlvl = AuthModList[i]->userauth (u);
		/* if authlvl is greater than newauthlvl, then auth is authoritive */
		if (authlvl > newauthlvl) {
			newauthlvl = authlvl;
		}
	}
	return newauthlvl;
}

int init_auth_module(Module *mod_ptr)
{
	int i;
	mod_auth auth;

	auth = ns_dlsym (mod_ptr->dl_handle, "ModAuthUser");
	if (auth) 
	{
		for(i = 0; i < NUM_MODULES; i++)
		{
			if (AuthModList[i] == NULL)
			{
				mod_ptr->userauth = auth;					
				AuthModList[i] = mod_ptr;
				return NS_SUCCESS;
			}
		}
	}
	return NS_FAILURE;
}

int delete_auth_module(Module *mod_ptr)
{
	int i;

	for(i = 0; i < NUM_MODULES; i++)
	{
		if (AuthModList[i] == mod_ptr)
		{
			AuthModList[i] = NULL;
			return NS_SUCCESS;
		}
	}
	return NS_FAILURE;
}

int InitAuth(void)
{
	memset(AuthModList, 0, sizeof(AuthModList));
	return NS_SUCCESS;
}
