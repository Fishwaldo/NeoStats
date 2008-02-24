/* NeoStats - IRC Statistical Services 
** Copyright (c) 2006 Mark Hetherington
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
#include "commands.h"

/** Bot pointer */
Bot *ns_bot;

const char *ns_about[] = 
{
	"test",
	"http://www.neostats.net/",
	NULL
};

/** Copyright info */
const char *ns_copyright[] = 
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"nstest",
	"nstest",
	ns_copyright,
	ns_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** BotInfo */
static BotInfo ns_botinfo = 
{
	"nstest", 
	"nstest1", 
	"ns", 
	BOT_COMMON_HOST, 
	"nstest", 	
	BOT_FLAG_SERVICEBOT|BOT_FLAG_RESTRICT_OPERS, 
	ns_commands, 
	ns_settings,
};

/** @brief ModInit
 *
 *  Init handler
 *  Loads connectserv configuration
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	/* Load stored configuration */
	ModuleConfig( ns_settings );
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();
	/* Create module bot */
	ns_bot = AddBot( &ns_botinfo );
	/* If failed to create bot, module will terminate */
	if( !ns_bot ) 
	{
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	SET_SEGV_LOCATION();
	return NS_SUCCESS;
}
