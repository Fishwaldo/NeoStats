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
 *  - Since IRCu sends kick and part when a user is kicked, bots will be 
 *    unable to rejoin from the kick event
 */

#include "neostats.h"
#include "protocol.h"
#include "modes.h"
#include "bots.h"
#include "users.h"
#include "channels.h"
#include "exclude.h"
#include "services.h"
#include "modules.h"

/* @brief hash and list sizes */
#define CHANNEL_TABLE_SIZE	-1
#define CHANNEL_MEM_SIZE	-1
#define CHANNEL_MAXMODES	-1

/* @brief Module channel hash list */
static hash_t *channelhash;
/* @brief quit reason buffer */
static char quitreason[BUFSIZE];
/* temp buffer to save kick info for IRCu */
static char savekicker[MAXHOST];
static char savekickreason[BUFSIZE];
/** @brief Module data flags */
static unsigned int fchannelmoddata = 0;
static unsigned int moddatacnt[NUM_MODULES];

int comparechanmember( const void *key1, const void *key2 )
{
	ChannelMember *cm = ( ChannelMember * ) key1;
	return ircstrcasecmp( cm->u->name, key2 );
}


/** @brief ChanPartHandler
 *
 *  list handler for channel parts
 *  Channel subsystem use only.
 *
 *  @param list
 *  @param node
 *  @param v
 *
 *  @return none
 */

static void ChanPartHandler( list_t * list, lnode_t * node, void *v )
{
	PartChannel( ( Client * )v, lnode_get( node ), quitreason[0] != 0 ? quitreason : NULL );
}

/** @brief PartAllChannels
 *
 *  Part client from all channels
 *  NeoStats core use only.
 *
 *  @param u
 *  @param reason
 *
 *  @return none
 */

void PartAllChannels( Client *u, const char *reason )
{
	os_memset( quitreason, 0, BUFSIZE );
	if( reason ) {
		strlcpy( quitreason, reason, BUFSIZE );
		strip_mirc_codes( quitreason );
	}
	list_process( u->user->chans, u, ChanPartHandler );
}

/** @brief ChannelTopic
 *
 *  Process channel topic change and send EVENT_TOPIC to modules
 *  NeoStats core use only.
 *
 *  @param channel name
 *  @param owner who changed the topic
 *  @param time topic was changed
 *  @param topic
 *
 *  @return none
 */

void ChannelTopic( const char *chan, const char *owner, const char *ts, const char *topic )
{
	CmdParams *cmdparams;
	Channel *c;

	c = FindChannel( chan );
	if( !c ) {
		nlog( LOG_WARNING, "ChannelTopic: can't find channel %s", chan );
		return;
	}
	if( topic ) {
		strlcpy( c->topic, topic, BUFSIZE );
	} else {
		c->topic[0] = 0;
	}
	strlcpy( c->topicowner, owner, MAXHOST );
	c->topictime = ( ts ) ? atoi( ts ) : me.now;
	cmdparams = (CmdParams *) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = FindUser( owner );
	if( !cmdparams->source ) {
		cmdparams->source = FindServer( owner );
	}
	cmdparams->channel = c;
	SendAllModuleEvent( EVENT_TOPIC, cmdparams );
	ns_free( cmdparams );
}

/** @brief new_chan
 *
 *  Create new channel record and insert it into the hash.
 *
 *  @param chan name of channel to create
 *
 *  @return c pointer to created channel record
 */

static Channel *new_chan( const char *chan )
{
	CmdParams *cmdparams;
	Channel *c;

	if( hash_isfull( channelhash ) ) {
		nlog( LOG_CRITICAL, "new_chan: channel hash is full" );
		return NULL;
	}
	c = ns_calloc( sizeof( Channel ) );
	strlcpy( c->name, chan, MAXCHANLEN );
	if( ircstrcasecmp( me.serviceschan, chan ) == 0 )
	{
		c->flags |= CHANNEL_FLAG_ME;
	}
	hnode_create_insert( channelhash, c, c->name );
	c->members = list_create( CHANNEL_MEM_SIZE );
	c->modeparms = list_create( CHANNEL_MAXMODES );
	c->creationtime = me.now;
	/* XXX TODO: Set the channel language */
	c->lang = me.lang;
	/* check exclusions */
	ns_do_exclude_chan( c );
	me.channelcount++;
	cmdparams = (CmdParams *) ns_calloc( sizeof( CmdParams ) );
	cmdparams->channel = c;
	SendAllModuleEvent( EVENT_NEWCHAN, cmdparams );
	ns_free( cmdparams );
	return c;
}

/** @brief del_chan
 *
 *  frees memory associated with channel and removes it from the channel hash
 *
 *  @param c pointer to channel structure to delete
 *
 *  @return none
 */

void del_chan( Channel *c )
{
	CmdParams *cmdparams;
	hnode_t *cn;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "del_chan: deleting channel %s", c->name );
	cn = hash_lookup( channelhash, c->name );
	if( !cn ) {
		nlog( LOG_WARNING, "del_chan: channel %s not found.", c->name );
		return;
	}
	me.channelcount--;
	cmdparams = (CmdParams *) ns_calloc( sizeof( CmdParams ) );
	cmdparams->channel = c;
	SendAllModuleEvent( EVENT_DELCHAN, cmdparams );
	ns_free( cmdparams );
	list_destroy_auto( c->modeparms );
	list_destroy( c->members );
	hash_delete( channelhash, cn );
	hnode_destroy( cn );
	ns_free( c );
}

/** @brief del_user_channel
 *
 *  Deletes channel user record and remove it from the hash
 *  Channel subsystem use only.
 *
 *  @param c pointer to channel structure to delete
 *  @param u pointer to user client to delete
 *
 *  @return none
 */

static void del_user_channel( Channel *c, Client *u )
{
	lnode_t *un;

	un = list_find( u->user->chans, c->name, comparef );
	if( !un ) {
		nlog( LOG_WARNING, "del_user_channel: %s not found in channel %s", u->name, c->name );
	} else {
		lnode_destroy( list_delete( u->user->chans, un ) );
	}
}

/** @brief del_user_channel
 *
 *  Deletes channel member record
 *  Channel subsystem use only.
 *
 *  @param c pointer to channel structure to delete
 *  @param u pointer to user client to delete
 *
 *  @return none
 */

static void del_channel_member( Channel *c, Client *u )
{
	ChannelMember *cm;
	lnode_t *un;
	
	un = list_find( c->members, u->name, comparechanmember );
	if( !un ) {
		nlog( LOG_WARNING, "%s isn't a member of channel %s", u->name, c->name );
		return;
	}
	cm = lnode_get( un );
	lnode_destroy( list_delete( c->members, un ) );
	ns_free( cm );
	dlog( DEBUG3, "del_channel_member: cur users %s %d (list %d)", c->name, c->users,( int )list_count( c->members ) );
	c->users--;
}

/** @brief CheckEmptyChannel
 *
 *  Delete empty channels and raise appropriate events
 *  Channel subsystem use only.
 *
 *  @param c pointer to channel structure to delete
 *
 *  @return none
 */

static void CheckEmptyChannel( Channel *c )
{
	if( c->users <= 0 ) {
		del_chan( c );
	} else if( ( c->neousers > 0 ) && ( c->neousers == c->users ) ) {
		/* all real users have left the channel */
		handle_dead_channel( c );
	}
}

/** @brief KickChannel
 *
 *  Process a kick from a channel. 
 *  NeoStats core use only.
 *
 *  @param kickby, the user nick or servername doing the kick
 *  @param chan, channel name user being kicked from
 *  @param kicked, the user nick getting kicked
 *  @param kickreason the reason the user was kicked
 *
 *  @return none
 */

void KickChannel( const char *kickby, const char *chan, const char *kicked, const char *kickreason )		
{
	CmdParams *cmdparams;
	Channel *c;
	Client *u;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "KickChannel: %s kicked %s from %s for %s", kickby, kicked, chan, kickreason ? kickreason : "no reason" );
	u = FindUser( kicked );
	if( !u ) {
		nlog( LOG_WARNING, "KickChannel: user %s not found", kicked );
		return;
	}
	c = FindChannel( chan );
	if( !c ) {
		nlog( LOG_WARNING, "KickChannel: channel %s not found", chan );
		return;
	} 
	/* If PROTOCOL_KICKPART then we will also get part so DO NOT REMOVE USER or send event yet */
	if( ircd_srv.protocol & PROTOCOL_KICKPART ) {
		u->flags |= CLIENT_FLAG_ZOMBIE;
		strlcpy( savekicker, kickby, MAXHOST );
		if( kickreason )
			strlcpy( savekickreason, kickreason, BUFSIZE );
		else 
			savekickreason[0] = 0;
	} else {
		del_user_channel( c, u );
		del_channel_member( c, u );
		cmdparams = (CmdParams *) ns_calloc( sizeof( CmdParams ) );
		cmdparams->target = u;
		cmdparams->channel = c;
		cmdparams->source = FindUser( kickby );
		if( !cmdparams->source ) {
			cmdparams->source = FindServer( kickby );
		}
		cmdparams->param = (char *)kickreason;
		SendAllModuleEvent( EVENT_KICK, cmdparams );
		if( IsMe( u ) ) {
			/* its one of our bots */
			cmdparams->bot = u->user->bot;
			SendModuleEvent( EVENT_KICKBOT, cmdparams, u->user->bot->moduleptr );
		}
		ns_free( cmdparams );
		CheckEmptyChannel( c );
	}
}

/** @brief PartChannel
 *
 *  Parts a user from a channel and raises events if required
 *  Events raised are PARTCHAN and DELCHAN
 *  if its one of our bots, also update bot channel lists
 *  NeoStats core use only.
 *
 *  @param u the User structure corrosponding to the user that left the channel
 *  @param chan the channel to part them from
 *  @param reason the reason the user parted, if any
 *
 *  @return none
*/

void PartChannel( Client *u, const char *chan, const char *reason )
{
	CmdParams *cmdparams;
	Channel *c;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "PartChannel: parting %s from %s", u->name, chan );
	if( !u ) {
		nlog( LOG_WARNING, "PartChannel: trying to part NULL user from %s", chan );
		return;
	}
	c = FindChannel( chan );
	if( !c ) {
		nlog( LOG_WARNING, "PartChannel: channel %s not found", chan );
		return;
	}
	del_user_channel( c, u );
	del_channel_member( c, u );
	cmdparams = (CmdParams *) ns_calloc( sizeof( CmdParams ) );
	cmdparams->channel = c;
	if( u->flags & CLIENT_FLAG_ZOMBIE )
	{
		u->flags &= ~CLIENT_FLAG_ZOMBIE;
		cmdparams->target = u;
		cmdparams->source = FindUser( savekicker );
		if( !cmdparams->source ) {
			cmdparams->source = FindServer( savekicker );
		}
		cmdparams->param = savekickreason[0] ? savekickreason : NULL;
		SendAllModuleEvent( EVENT_KICK, cmdparams );
		if( IsMe( u ) ) {
			/* its one of our bots */
			cmdparams->bot = u->user->bot;
			SendModuleEvent( EVENT_KICKBOT, cmdparams, u->user->bot->moduleptr );
		}
		cmdparams->bot = NULL;
		cmdparams->target = NULL;
	}
	cmdparams->source = u;
	cmdparams->param = (char *) reason;
	SendAllModuleEvent( EVENT_PART, cmdparams );
	if( IsMe( u ) ) {
		/* its one of our bots */
		SendModuleEvent( EVENT_PARTBOT, cmdparams, u->user->bot->moduleptr );
		c->neousers --;
	}
	CheckEmptyChannel( c );
	ns_free( cmdparams );
}

/** @brief Process a user joining a channel
 *
 * joins a user to a channel and raises JOINCHAN event and if required NEWCHAN events
 * if the channel is new, a new channel record is requested and defaults are set
 * if its one of our bots, also update the botchanlist
 *
 * @param u The User structure of the user joining the channel
 * @param chan the channel name
 *
 * @returns Nothing
*/

void
JoinChannel( const char *nick, const char *chan )
{
	CmdParams *cmdparams;
	Client *u;
	Channel *c;
	ChannelMember *cm;
	
	SET_SEGV_LOCATION();
	u = FindUser( nick );
	if( !u ) {
		nlog( LOG_WARNING, "JoinChannel: tried to join unknown user %s to channel %s", nick, chan );
		return;
	}
	if( !ircstrcasecmp( "0", chan ) ) {
		/* join 0 is actually part all chans */
		dlog( DEBUG2, "JoinChannel: parting %s from all channels", u->name );
		PartAllChannels( u, NULL );
		return;
	}
	c = FindChannel( chan );
	if( !c ) {
		/* its a new Channel */
		dlog( DEBUG2, "JoinChannel: new channel %s", chan );
		c = new_chan( chan );
	}
	/* add this users details to the channel members hash */
	if( list_find( c->members, u->name, comparechanmember ) ) {
		nlog( LOG_WARNING, "JoinChannel: tried to add %s to channel %s but they are already a member", u->name, chan );
		return;
	}
	if( list_isfull( c->members ) ) {
		nlog( LOG_CRITICAL, "JoinChannel: channel %s member list is full", c->name );
		return;
	}
	dlog( DEBUG2, "JoinChannel: adding usernode %s to channel %s", u->name, chan );
	cm = ns_calloc( sizeof( ChannelMember ) );
	cm->u = FindUser( nick );
	cm->tsjoin = me.now;
	cm->flags = 0;
	lnode_create_append( c->members, cm );
	c->users++;
	if( list_isfull( u->user->chans ) ) {
		nlog( LOG_CRITICAL, "JoinChannel: user %s member list is full", u->name );
		return;
	}
	lnode_create_append( u->user->chans, c->name );
	cmdparams = (CmdParams *) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;
	cmdparams->channel = c;
	if( IsMe( u ) ) {
		/* its one of our bots */
		c->neousers ++;
	}
	SendAllModuleEvent( EVENT_JOIN, cmdparams );
	ns_free( cmdparams );
	dlog( DEBUG3, "JoinChannel: cur users %s %d (list %d)", c->name, c->users,( int )list_count( c->members ) );
}

/** @brief Dump Channel information
 *
 * dump either the entire channel list, or a single channel detail. Used for debugging
 * sends the output to the services channel
 *
 * @param chan the channel name to dump, or NULL for all channels
 *
 * @returns Nothing
*/

static void ListChannelMembers( CmdParams * cmdparams, Channel *c )
{
 	ChannelMember *cm;
	lnode_t *cmn;

	irc_prefmsg( ns_botptr, cmdparams->source, __( "Members:    %d (List %d)", cmdparams->source ), c->users,( int )list_count( c->members ) );
	cmn = list_first( c->members );
	while( cmn ) {
		cm = lnode_get( cmn );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "            %s Modes %s Joined: %ld", cmdparams->source ), cm->u->name, CmodeMaskToString( cm->flags ),( long )cm->tsjoin );
		cmn = list_next( c->members, cmn );
	}
}

static void ListChannel( CmdParams * cmdparams, Channel *c )
{
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Channel:    %s", cmdparams->source ), c->name );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Created:    %ld", cmdparams->source ),( long )c->creationtime );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "TopicOwner: %s TopicTime: %ld Topic: %s", cmdparams->source ), c->topicowner,( long )c->topictime, c->topic );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "PubChan?:   %d", cmdparams->source ), is_pub_chan( c ) );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Flags:      %x", cmdparams->source ), c->flags );
	ListChannelModes( cmdparams, c );
	ListChannelMembers( cmdparams, c );
	irc_prefmsg( ns_botptr, cmdparams->source, "========================================" );
}

void ListChannels( CmdParams * cmdparams, const char *chan )
{
	hnode_t *cn;
	hscan_t sc;
	Channel *c;

	if( !nsconfig.debug )
		return;
	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "================CHANLIST================",cmdparams->source ) );
	if( !chan ) {
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Channels %d", cmdparams->source ),( int )hash_count( channelhash ) );
		hash_scan_begin( &sc, channelhash );
		while( ( cn = hash_scan_next( &sc ) ) != NULL ) {
			c = hnode_get( cn );
			ListChannel( cmdparams, c );
		}
	} else {
		c = FindChannel( chan );
		if( c ) {
			ListChannel( cmdparams, c );
		} else {
			irc_prefmsg( ns_botptr, cmdparams->source, __( "ListChannels: can't find channel %s", cmdparams->source ), chan );
		}
	}
}

/** @brief FindChannel
 *
 *  Finds the channel structure for the channel named chan
 *
 *  @param chan the channel name to find
 *
 *  @returns channel structure for chan, or NULL if it can't be found.
*/

Channel *FindChannel( const char *chan )
{
	Channel *c;

	c = ( Channel * )hnode_find( channelhash, chan );
	if( !c ) {
		dlog( DEBUG3, "FindChannel: %s not found", chan );
	}
	return c;
}

/** @brief IsChannelMember 
 *
 *  Check whether nick is a member of the channel
 *
 *  @param c the channel to check
 *  @param u the user to check 
 *
 *  @returns NS_TRUE if user is a member of channel, NS_FALSE if not 
*/

int IsChannelMember( const Channel *c, const Client *u ) 
{
	if( !u || !c ) {
		return NS_FALSE;
	}
	if( list_find( c->members, u->name, comparechanmember ) ) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief test_cumode
 *
 *  Whether nick has a particular channel status e.g. ChanOp
 *
 *  @param chan the channel to check
 *  @param nick the nick to check 
 *
 *  @return NS_TRUE if has, else NS_FALSE
 */

int test_cumode( const char *chan, const char *nick, const int flag )
{
	Client *u;
	Channel *c;
 	ChannelMember *cm;

	u = FindUser( nick );
	c = FindChannel( chan );
	if( !u || !c ) {
		return NS_FALSE;
	}
	cm = lnode_find( c->members, nick, comparechanmember );
	if( cm ) {
		if( cm->flags & flag ) {
			return NS_TRUE;
		}
	}	
	return NS_FALSE;
}

/** @brief InitChannels
 *
 *  initialise channel subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitChannels( void )
{
	channelhash = hash_create( CHANNEL_TABLE_SIZE, 0, 0 );
	if( !channelhash )	{
		nlog( LOG_CRITICAL, "Unable to create channel hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniChannels
 *
 *  cleanup channel subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniChannels( void )
{
	hash_destroy( channelhash );
}

Channel *GetRandomChannel( void ) 
{
	hscan_t cs;
	hnode_t *cn;
	int randno, curno;
	
	curno = 0;
	randno = hrand( hash_count( channelhash ), 1 );	
	if( randno == -1 ) {
		return NULL;
	}
	hash_scan_begin( &cs, channelhash );
	while( ( cn = hash_scan_next( &cs ) ) != NULL ) {
		if( curno == randno ) {
			return(( Channel * )hnode_get( cn ) );
		}
		curno++;
	}
	nlog( LOG_WARNING, "GetRandomChannel() ran out of channels?" );
	return NULL;
}

/** @brief GetRandomChannelMember
 *
 *  find random channel member
 *
 *  @params uge use global exclusions
 *  @params c channel to select member from
 *
 *  @return Client pointer selected or NULL if none
 */

Client *GetRandomChannelMember(int uge, Channel *c) 
{
	ChannelMember *cm;
	lnode_t *ln;
	int randno;
	int curno = 0;
	
	randno = hrand( ( ( int )list_count( c->members ) ), 1 );	
	ln = list_first( c->members );
	while( ln ) 
	{
		cm = lnode_get(ln);
		if( !uge || !IsExcluded( cm->u ) )
		{
			if( curno == randno )
				return cm->u;
		}
		curno++;
		ln = list_next(c->members, ln);
	}
	return NULL;
}


char *GetRandomChannelKey( int length ) 
{
	int i;
	char *key;

	if( length < 1 || length > ( KEYLEN - 1 ) )
		length = KEYLEN - 1;
	key = ns_malloc( KEYLEN );
	for( i = 0; i < length; i++ )
	{
		key[i] = ( ( rand() % 26) + 'a' );
	}
	key[i] = 0;
	return key;
}

int GetChannelList( ChannelListHandler handler, void *v )
{
	hnode_t *node;
	hscan_t scan;
	Channel *c;

	SET_SEGV_LOCATION();
	hash_scan_begin( &scan, channelhash );
	while( ( node = hash_scan_next( &scan ) ) != NULL ) {
		c = hnode_get( node );
		if( handler( c, v ) == NS_TRUE )
			break;
	}
	return NS_SUCCESS;
}

int GetChannelMembers( Channel *c, ChannelMemberHandler handler, void *v )
{
 	ChannelMember *cm;
	lnode_t *cmn;

	cmn = list_first( c->members );
	while( cmn ) {
		cm = lnode_get( cmn );
		if( handler( c, cm, v ) == NS_TRUE )
			break;
		cmn = list_next( c->members, cmn );
	}
	return NS_SUCCESS;
}

hash_t *GetChannelHash( void )
{
	return channelhash;
}

void *AllocChannelModPtr( Channel* c, int size )
{
	void *ptr;
	ptr = ns_calloc( size );
	c->modptr[GET_CUR_MODNUM()] = ptr;
	fchannelmoddata |= ( 1 << GET_CUR_MODNUM() );
	moddatacnt[GET_CUR_MODNUM()]++;
	return ptr;
}

void FreeChannelModPtr( Channel *c )
{
	ns_free( c->modptr[GET_CUR_MODNUM()] );
	moddatacnt[GET_CUR_MODNUM()]--;
	if( moddatacnt[GET_CUR_MODNUM()] == 0 )
	{
		fchannelmoddata &= ~( 1 << GET_CUR_MODNUM() );
	}
}

void* GetChannelModPtr( Channel *c )
{
	return c->modptr[GET_CUR_MODNUM()];
}

void ClearChannelModValue( Channel *c )
{
	if( c )
	{
		c->modvalue[GET_CUR_MODNUM()] = NULL;
		moddatacnt[GET_CUR_MODNUM()]--;
	}
	if( moddatacnt[GET_CUR_MODNUM()] == 0 )
	{
		fchannelmoddata &= ~( 1 << GET_CUR_MODNUM() );
	}
}

void SetChannelModValue( Channel *c, void *data )
{
	if( c )
	{
		c->modvalue[GET_CUR_MODNUM()] = data;
		fchannelmoddata |= ( 1 << GET_CUR_MODNUM() );
		moddatacnt[GET_CUR_MODNUM()]++;
	}
}

void *GetChannelModValue( Channel *c )
{
	if( c )
	{
		return c->modvalue[GET_CUR_MODNUM()];
	}
	return NULL;	
}

void CleanupChannelModdata( int index )
{
	hnode_t *node;
	hscan_t scan;
	Channel *c;

	SET_SEGV_LOCATION();
	if (fchannelmoddata & (1 << index)) {
		if( moddatacnt[index] > 0 ) {
			nlog( LOG_WARNING, "Cleaning up channels after dirty module!" );
			hash_scan_begin( &scan, channelhash );
			while( ( node = hash_scan_next( &scan ) ) != NULL ) {
				c = hnode_get( node );
				if( c->modptr[index] ) {
					ns_free( c->modptr[index] );		
				}
				c->modvalue[index] = NULL;
			}
		}
		fchannelmoddata &= ~( 1 << index );
		moddatacnt[index] = 0;
	}
}
