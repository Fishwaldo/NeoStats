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

typedef int (*getauthfunc) (User *, int curlvl);
typedef int (*listauthfunc) (User * u);

typedef struct AuthModule {
	Module* module_ptr;
	getauthfunc getauth;
	listauthfunc listauth;
}AuthModule;

extern void *load_auth_mods[NUM_MODULES];

static AuthModule AuthModList[NUM_MODULES];

static int AuthModuleCount = 0;

/* Do dl lookups in advance to speed up UserLevel processing 
 *
 */

int UserAuth(User * u)
{
	int tmplvl = 0;
	int authlvl = 0;
	int i;
	
	if(IsServiceRoot(u))
		return(NS_ULEVEL_ROOT);
	for(i = 0; i < AuthModuleCount; i ++)
	{
		if (AuthModList[i].getauth) {
			authlvl = AuthModList[i].getauth (u, tmplvl);
			/* if authlvl is greater than tmplvl, then auth is authoritive */
			if (authlvl > tmplvl) {
				tmplvl = authlvl;
			}
		}
	}
	return tmplvl;
}

static void load_auth_module(const char* name)
{
	AuthModule* newauth;
	
	newauth = &AuthModList[AuthModuleCount];
	newauth->module_ptr = load_module (name, NULL);
	if(newauth->module_ptr) {
		newauth->getauth = 
			ns_dlsym (newauth->module_ptr->dl_handle, "ModAuthUser");
		newauth->listauth = 
			ns_dlsym (newauth->module_ptr->dl_handle, "ModAuthList");
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
