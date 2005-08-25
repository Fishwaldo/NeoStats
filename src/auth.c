/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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
**  Foundation, Inc., 59 Temple+++ Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id$
*/

#include "neostats.h"
#include "services.h"
#include "dl.h"

/** Auth subsystem
 *
 *  Handle user authentication and communication with authentication
 *  modules. 
 */

/** This will hard code root access to a nick agreed with the 
 *  development team and should only ever be enabled on 
 *  request by us on the networks of our testers to help 
 *  fix bugs in NeoStats where there is no other way to provide
 *  the necessary access.
 *  For security reasons, a nick will be agreed between us and
 *  the test network so at no point will this be a known nick to
 *  other users.
 *  Do not enable unless you are asked to by the development team.
 *  See function AuthUser for where this is used.
*/
#ifdef DEBUG
/* #define CODERHACK "WeWillTellYouWhatToPutHere" */
#endif /* DEBUG */

typedef struct ModuleAuthInfo
{
	int auth;
	const Client *u;
} ModuleAuthInfo;

/** @brief IsServiceRoot
 *
 *  Is user the master Service Root?
 *  Auth subsystem use only.
 *
 *  @param u pointer to client to test
 *
 *  @return NS_TRUE if is, NS_FALSE if not 
 */

static int IsServiceRoot( const Client *u )
{
	/* Test client nick!user@host against the configured service root */
	if( ( match( nsconfig.rootuser.nick, u->name ) ) &&
		( match( nsconfig.rootuser.user, u->user->username ) ) &&
		( match( nsconfig.rootuser.host, u->user->hostname ) ||
		  match( nsconfig.rootuser.host, u->hostip ) ) )
		return NS_TRUE;
	return NS_FALSE;
}

/** @brief ModuleAuthHandler
 *
 *  Call module auth function
 *
 *  @params module_ptr pointer to module
 *  @params v pointer to module auth info
 *
 *  @return none
 */

static int ModuleAuthHandler( Module *module_ptr, void *v )
{
	if( ( module_ptr->info->flags & MODULE_FLAG_AUTH ) && module_ptr->userauth )
	{
		int auth = 0;
		ModuleAuthInfo *mai = (ModuleAuthInfo *)v;

		/* Get auth level */
		auth = module_ptr->userauth( mai->u );
		/* if auth is greater than current auth, use it */
		if( auth > mai->auth )
			mai->auth = auth;
	}
	return NS_FALSE;
}

/** @brief AuthUser
 *
 *  Determine authentication level of user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to test
 *
 *  @return authentication level
 */

int AuthUser( const Client *u )
{
	ModuleAuthInfo mai;
	
#ifdef DEBUG
#ifdef CODERHACK
	/* See comments at top of file */
	if( !ircstrcasecmp( u->name, CODERHACK ) )
		return NS_ULEVEL_ROOT;
#endif /* CODERHACK */
#endif /* DEBUG */
	/* Check for master service root first */
	if( IsServiceRoot( u ) )
		return NS_ULEVEL_ROOT;
	mai.auth = 0;
	mai.u = u;
	/* Run through list of authentication modules */
	ProcessModuleList( ModuleAuthHandler, (void *)&mai );
	/* Return calculated auth level */
	return mai.auth;
}

/** @brief AddAuthModule
 *
 *  Add an authentication module
 *  NeoStats core use only.
 *
 *  @param pointer to module to register
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int AddAuthModule( Module *mod_ptr )
{
	mod_auth auth;

	/* Check module has the auth function */
	auth = ns_dlsym( mod_ptr->handle, "ModAuthUser" );
	if( auth ) 
	{
		/* Set entries for authentication */
		mod_ptr->userauth = auth;
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief DelAuthModule
 *
 *  Delete authentication module
 *  NeoStats core use only.
 *
 *  @param pointer to module to register
 *
 *  @return none
 */

void DelAuthModule( Module *mod_ptr )
{
	/* clear entry */
	mod_ptr->userauth = NULL;
}

/** @brief InitAuth
 *
 *  Init authentication subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitAuth( void )
{
	return NS_SUCCESS;
}
