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
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
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

/** List of registered authentication modules 
 *  Auth subsystem use only. */
static Module *AuthModList[NUM_MODULES];

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
	int newauthlvl = 0;
	int authlvl = 0;
	int i;
	
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
	/* Run through list of authentication modules */
	for( i = 0; i < NUM_MODULES; i++ )
	{
		if( AuthModList[i] )
		{
			/* Get auth level */
			authlvl = AuthModList[i]->userauth( u );
			/* if authlvl is greater than newauthlvl, use it */
			if( authlvl > newauthlvl )
				newauthlvl = authlvl;
		}
	}
	/* Return calculated auth level */
	return newauthlvl;
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
	int i;
	mod_auth auth;

	/* Check module has the auth function */
	auth = ns_dlsym( mod_ptr->handle, "ModAuthUser" );
	if( auth ) 
	{
		/* Find free slot for module */
		for( i = 0; i < NUM_MODULES; i++ )
		{
			if( AuthModList[i] == NULL )
			{
				/* Set entries for authentication */
				mod_ptr->userauth = auth;					
				AuthModList[i] = mod_ptr;
				return NS_SUCCESS;
			}
		}
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
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DelAuthModule( Module *mod_ptr )
{
	int i;

	/* Run through authentication module list */
	for( i = 0; i < NUM_MODULES; i++ )
	{
		if( AuthModList[i] == mod_ptr )
		{
			/* Found requested module so clear entry */
			AuthModList[i] = NULL;
			return NS_SUCCESS;
		}
	}
	return NS_FAILURE;
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
	/* Clear the module list */
	os_memset( AuthModList, 0, sizeof( AuthModList ) );
	return NS_SUCCESS;
}
