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

/*  TODO:
 *  - Other ban types beside Unreal TKL
 *  - Channel bans?
 */

#include "neostats.h"
#include "nsevents.h"
#include "services.h"

/** Bans subsystem
 *
 *  Handle bans on the network
 */

/** List of bans
 *  Bans subsystem use only. */
static hash_t *banhash;

/** @brief ProcessBanList
 *
 *  Calls handler for all bans
 *
 *  @params handler to call
 *  @params v additional data to send to handler
 *
 *  @return result of handler NS_TRUE or NS_FALSE
 */

int ProcessBanList( BanListHandler handler, void *v )
{
	Ban *ban;
	hscan_t ss;
	hnode_t *bansnode;

	SET_SEGV_LOCATION();
	hash_scan_begin( &ss, banhash );
	while( ( bansnode = hash_scan_next( &ss ) ) != NULL )
	{
		ban = hnode_get( bansnode );
		if( handler( ban, v ) == NS_TRUE )
			return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief new_ban
 *
 *  Create a new ban
 *  Bans subsystem use only.
 *
 *  @param mask of client to create ban for
 *
 *  @return pointer to newly created entry or NULL for failure
 */

static Ban *new_ban( const char *mask )
{
	Ban *ban;

	if( hash_isfull( banhash ) ) {
		nlog( LOG_CRITICAL, "new_ban: bans hash is full" );
		return NULL;
	}
	/* Allocate memory for ban and add to hash table */
	dlog( DEBUG2, "new_ban: %s", mask );
	ban = ns_calloc( sizeof( Ban ) );
	strlcpy( ban->mask, mask, MAXHOST );
	hnode_create_insert( banhash, ban, ban->mask );
	return ban;
}

/** @brief AddBan
 *
 *  Add a new ban to the system
 *  NeoStats core use only.
 *
 *  @param type
 *  @param user
 *  @param host
 *  @param mask,
 *  @param reason
 *  @param setby
 *  @param tsset
 *  @param tsexpires
 *
 *  @return nothing
 */

void AddBan( const char *type, const char *user, const char *host, const char *mask,
			 const char *reason, const char *setby, const char *tsset, const char *tsexpires )
{
	CmdParams * cmdparams;
	Ban* ban;

	SET_SEGV_LOCATION();
	ban = new_ban( mask );
	if( !ban ) {
		return;
	}
	strlcpy( ban->type, type, 8 );
	strlcpy( ban->user, user, MAXUSER );
	strlcpy( ban->host, host, MAXHOST );
	strlcpy( ban->mask, mask, MAXHOST );
	strlcpy( ban->reason, reason,BUFSIZE );
	strlcpy( ban->setby ,setby, MAXHOST );
	ban->tsset = atol( tsset );
	ban->tsexpires = atol( tsexpires );
	/* run the module event */
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->param = ( char * )ban;
	SendAllModuleEvent( EVENT_ADDBAN, cmdparams );
	ns_free( cmdparams );
}

/** @brief DelBan
 *
 *  Delete a ban from the system
 *  NeoStats core use only.
 *
 *  @param type
 *  @param user
 *  @param host
 *  @param mask,
 *  @param reason
 *  @param setby
 *  @param tsset
 *  @param tsexpires
 *
 *  @return nothing
 */

void DelBan( const char *type, const char *user, const char *host, const char *mask,
			 const char *reason, const char *setby, const char *tsset, const char *tsexpires )
{
	CmdParams * cmdparams;
	Ban *ban;
	hnode_t *bansnode;

	SET_SEGV_LOCATION();
	bansnode = hash_lookup( banhash, mask );
	if( !bansnode ) {
		nlog( LOG_WARNING, "DelBan: unknown ban %s", mask );
		return;
	}
	ban = hnode_get( bansnode );
	/* run the module event */
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->param = ( char * )ban;
	SendAllModuleEvent( EVENT_DELBAN, cmdparams );
	ns_free( cmdparams );
	hash_delete_destroy_node( banhash, bansnode );
	ns_free( ban );
}

/** @brief ReportBan
 *
 *  BANS LIST helper
 *  report ban info
 *
 *  @params module_ptr pointer to module to report
 *  @params v client to send to
 *
 *  @return none
 */

static int ReportBan( Ban *ban, void *v )
{
	irc_chanalert( ns_botptr, _( "Ban: %s " ), ban->mask );
	return NS_FALSE;
}

/** @brief ListBans
 *
 *  List all bans currently set
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return nothing
 */

void ListBans( void )
{
	irc_chanalert( ns_botptr, _( "Ban Listing:" ) );
	ProcessBanList( ReportBan, NULL );
	irc_chanalert( ns_botptr, _( "End of list." ) );
}

/** @brief FiniBans
 *
 *  Cleanup bans subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return nothing
 */

void FiniBans( void )
{
	Ban *ban;
	hnode_t *bansnode;
	hscan_t hs;

	hash_scan_begin( &hs, banhash );
	while( ( bansnode = hash_scan_next( &hs ) ) != NULL  ) {
		ban = hnode_get( bansnode );
		hash_scan_delete_destroy_node( banhash, bansnode );
		ns_free( ban );
	}
	hash_destroy( banhash );
}

/** @brief InitBans
 *
 *  Init bans subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitBans( void )
{
	banhash = hash_create( -1, 0, 0 );
	if( !banhash ) {
		nlog( LOG_CRITICAL, "Unable to create bans hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}
