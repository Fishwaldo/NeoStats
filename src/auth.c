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
#include "services.h"
#include "dl.h"

/** Auth SubSystem
 *
 *  Manage user authentication and communication with authentication
 *  modules
 */


/** List of registered authentication modules */
static Module* AuthModList[NUM_MODULES];

/** @brief IsServiceRoot
 *
 *  Is user the master Service Root?
 *
 *  @param u pointer to client to test
 *
 *  @return NS_TRUE if is, NS_FALSE if not 
 */

static int IsServiceRoot (Client * u)
{
	if ((match (config.rootuser.nick, u->name))
		&& (match (config.rootuser.user, u->user->username))
		&& (match (config.rootuser.host, u->user->hostname))) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief UserAuth
 *
 *  Determine authentication level of user
 *
 *  @param u pointer to client to test
 *
 *  @return authentication level
 */

int UserAuth (Client * u)
{
	int newauthlvl = 0;
	int authlvl = 0;
	int i;
	
	if (IsServiceRoot (u)) {
		return NS_ULEVEL_ROOT;
	} 
	for (i = 0; i < NUM_MODULES; i++)
	{
		if (AuthModList[i]) {
			authlvl = AuthModList[i]->userauth (u);
			/* if authlvl is greater than newauthlvl, then 
			* authentication is authoritive */
			if (authlvl > newauthlvl) {
				newauthlvl = authlvl;
			}
		}
	}
	return newauthlvl;
}

/** @brief init_auth_module
 *
 *  Register authentication module
 *
 *  @param pointer to module to register
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int init_auth_module (Module *mod_ptr)
{
	int i;
	mod_auth auth;

	auth = ns_dlsym (mod_ptr->dl_handle, "ModAuthUser");
	if (auth) 
	{
		for( i = 0; i < NUM_MODULES; i++)
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

/** @brief delete_auth_module
 *
 *  Unregister authentication module
 *
 *  @param pointer to module to register
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int delete_auth_module (Module *mod_ptr)
{
	int i;

	for (i = 0; i < NUM_MODULES; i++)
	{
		if (AuthModList[i] == mod_ptr)
		{
			AuthModList[i] = NULL;
			return NS_SUCCESS;
		}
	}
	return NS_FAILURE;
}

/** @brief InitAuth
 *
 *  Init authentication sub system
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitAuth(void)
{
	memset (AuthModList, 0, sizeof(AuthModList));
	return NS_SUCCESS;
}
