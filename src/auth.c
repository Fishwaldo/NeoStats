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

typedef int (*userauthfunc) (User *u);
typedef int (*listauthfunc) (User * u);

typedef struct AuthModule {
	Module* module_ptr;
	userauthfunc userauth;
	listauthfunc listauth;
}AuthModule;

extern void *load_auth_mods[NUM_MODULES];

static AuthModule AuthModList[NUM_MODULES];
static int AuthModuleCount = 0;

int UserAuth(User * u)
{
	int newauthlvl = 0;
	int authlvl = 0;
	int i;
	
	for(i = 0; i < AuthModuleCount; i ++)
	{
		if (AuthModList[i].userauth) {
			authlvl = AuthModList[i].userauth (u);
			/* if authlvl is greater than newauthlvl, then auth is authoritive */
			if (authlvl > newauthlvl) {
				newauthlvl = authlvl;
			}
		}
	}
	return newauthlvl;
}

static void load_auth_module(const char* name)
{
	AuthModule* auth_module;
	
	auth_module = &AuthModList[AuthModuleCount];
	auth_module->module_ptr = load_module (name, NULL);
	if(auth_module->module_ptr) {
		auth_module->userauth = 
			ns_dlsym (auth_module->module_ptr->dl_handle, "ModAuthUser");
		auth_module->listauth = 
			ns_dlsym (auth_module->module_ptr->dl_handle, "ModAuthList");
		AuthModuleCount ++;
	}
}

int InitAuth(void)
{
	int i;

	memset(AuthModList, 0, sizeof(AuthModList));
	for (i = 0; (i < NUM_MODULES) && (load_auth_mods[i] != 0); i++) {
		load_auth_module((char*)load_auth_mods[i]);
	}
	return NS_SUCCESS;
}

int ListAuth(User *u)
{
	int i;
	
	for(i = 0; i < AuthModuleCount; i ++)
	{
		if (AuthModList[i].listauth) {
			AuthModList[i].listauth (u);
		}
	}
	return NS_SUCCESS;
}
