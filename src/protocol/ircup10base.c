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

static void m_private( char *origin, char **argv, int argc, int srv );
static void m_notice( char *origin, char **argv, int argc, int srv );
static void m_version( char *origin, char **argv, int argc, int srv );
static void m_motd( char *origin, char **argv, int argc, int srv );
static void m_admin( char *origin, char **argv, int argc, int srv );
static void m_server( char *origin, char **argv, int argc, int srv );
static void m_squit( char *origin, char **argv, int argc, int srv );
static void m_quit( char *origin, char **argv, int argc, int srv );
static void m_mode( char *origin, char **argv, int argc, int srv );
static void m_kill( char *origin, char **argv, int argc, int srv );
static void m_pong( char *origin, char **argv, int argc, int srv );
static void m_away( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_topic( char *origin, char **argv, int argc, int srv );
static void m_kick( char *origin, char **argv, int argc, int srv );
static void m_join( char *origin, char **argv, int argc, int srv );
static void m_create( char *origin, char **argv, int argc, int srv );
static void m_part( char *origin, char **argv, int argc, int srv );
static void m_stats( char *origin, char **argv, int argc, int srv );
static void m_ping( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_end_of_burst( char *origin, char **argv, int argc, int srv );
static void m_wallusers( char *origin, char **argv, int argc, int srv );
static void m_wallops( char *origin, char **argv, int argc, int srv );
static void m_vhost( char *origin, char **argv, int argc, int srv );

/* Temporary buffers for numeric conversion */
char neostatsbase64[3] = "\0";
/* Flags for numeric usage; limits to 64 clients */
char neonicknumerics[64];

/*
 * Numeric nicks are new as of version ircu2.10.00beta1.
 *
 * The idea is as follows:
 * In most messages( for protocol 10+ ) the original nick will be
 * replaced by a 3 character string: YXX
 * Where 'Y' represents the server, and 'XX' the nick on that server.
 *
 * 'YXX' should not interfer with the input parser, and therefore is
 * not allowed to contain spaces or a ':'.
 * Also, 'Y' can't start with a '+' because of m_server( ).
 *
 * We keep the characters printable for debugging reasons too.
 *
 * The 'XX' value can be larger then the maximum number of clients
 * per server, we use a mask( struct Server::nn_mask ) to get the real
 * client numeric. The overhead is used to have some redundancy so
 * just-disconnected-client aren't confused with just-connected ones.
 */

/* These must be the same on ALL servers ! Do not change ! */

#define NUMNICKLOG 6
#define NUMNICKBASE 64          /*( 2 << NUMNICKLOG ) */
#define NUMNICKMASK 63          /*( NUMNICKBASE-1 ) */

/*
 * convert2y[] converts a numeric to the corresponding character.
 * The following characters are currently known to be forbidden:
 *
 * '\0' : Because we use '\0' as end of line.
 *
 * ' '  : Because parse_*( ) uses this as parameter seperator.
 * ':'  : Because parse_server( ) uses this to detect if a prefix is a
 *        numeric or a name.
 * '+'  : Because m_nick( ) uses this to determine if parv[6] is a
 *        umode or not.
 * '&', '#', '+', '$', '@' and '%' :
 *        Because m_message( ) matches these characters to detect special cases.
 */
static const char convert2y[] = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]";

static const unsigned int convert2n[] = 
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0, 
   0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,62, 0,63, 0, 0,
   0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned int base64toint( const char* s )
{
	int max = 0;
	unsigned int i = convert2n[( unsigned char ) *s++];
	max++;
	while( *s ) {
		i <<= NUMNICKLOG;
		i += convert2n[( unsigned char ) *s++];
		max++;
		if( max>=5 ) 
			break;
	}
	return i;
}

unsigned int base64toIP( const char* s )
{
	unsigned int i = convert2n[( unsigned char ) *s++];
	while( *s ) {
		i <<= NUMNICKLOG;
		i += convert2n[( unsigned char ) *s++];
	}
	return i;
}

const char* inttobase64( char* buf, unsigned int v, unsigned int count )
{
	buf[count] = '\0';  
	while( count > 0 ) {
		buf[--count] = convert2y[( v & NUMNICKMASK )];
		v >>= NUMNICKLOG;
	}
	return buf;
}

/* server
inttobase64( cli_yxx( c ), numeric, 2 ) */

/* nick
inttobase64( cli_yxx( cptr ), last_nn, 3 ) */
/*
DEBUG2 CORE - SENT: PASS password
DEBUG2 CORE - SENT: SERVER stats.mark.net 1 1076012166 1076012166 P10 CD] :[stats.mark.net] NeoStats 2.5 IRC Statistical Server!
DEBUG3 CORE - Sendings pings...
DEBUG1 CORE - R: PASS :password
DEBUG1 CORE - R: SERVER mark.local.org 1 1076002125 1076012166 J10 ABAP] + :me
DEBUG1 CORE - New Server: mark.local.org
DEBUG1 CORE - R: AB N Mark 1 1076011621 a 192.168.238.13 DAqO4N ABAAB :M
DEBUG1 CORE - R: AB EB
DEBUG1 CORE - R: AB G !1076012256.68240 stats.mark.net 1076012256.68240
DEBUG3 CORE - Sendings pings...
DEBUG2 CORE - SENT: :stats.mark.net PING stats.mark.net :mark.local.org
DEBUG1 CORE - R: AB Z AB :stats.mark.net
DEBUG1 CORE - R: AB SQ mark.local.org 0 :Ping timeout
DEBUG3 CORE - Sendings pings...
DEBUG2 CORE - SENT: :stats.mark.net PING stats.mark.net :mark.local.org
*/

void send_server( const char *source, const char *name, const int numeric, const char *infoline )
{
	send_cmd( "%s %s * +%s 604800 %lu :%s", neostatsbase64, TOK_JUPE, name, me.now, infoline );
}

/*
1 <name of new server>
2 <hops>
3 <boot TS>
4 <link TS>
5 <protocol>
6 <numeric of new server><max client numeric>
7 <flags>
-1 <description of new server>
*/
void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	/* Reset our numeric buffer */
	os_memset( neonicknumerics, 0 , sizeof( neonicknumerics ) );
	inttobase64( neostatsbase64, numeric, 2 );
	send_cmd( "%s :%s", MSG_PASS, pass );
	send_cmd( "%s %s 1 %lu %lu J10 %s]]] +s :%s", MSG_SERVER, name, tsboot, tslink, neostatsbase64, infoline );
	set_server_base64( name, neostatsbase64 );
}

void send_squit( const char *server, const char *quitmsg )
{
	send_cmd( "%s %s %s 0 :%s", neostatsbase64, TOK_SQUIT, server, quitmsg );
}

void send_quit( const char *source, const char *quitmsg )
{
	char* num;

	/* Clear numeric */
	num = nick_to_base64( source );
	send_cmd( "%s %s :%s", num, TOK_QUIT, quitmsg );
	neonicknumerics[convert2n[( int )num[4]]] = 0;
}

void send_part( const char *source, const char *chan, const char *reason )
{
	send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_PART, chan, reason );
}

void send_join( const char *source, const char *chan, const char *key, const unsigned long ts )
{
	send_cmd( "%s %s %s %lu", nick_to_base64( source ), TOK_JOIN, chan, ts );
}

/* R: ABAAH M #c3 +tn */
void send_cmode( const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( "%s %s %s %s %s %lu", neostatsbase64, TOK_MODE, chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	int i;
	char nicknumbuf[6];

	for( i = 0; i < 64; i++ )
	{
		/* Reserve numeric */
		if( neonicknumerics[i]==0 )
		{
			neonicknumerics[i] = 1;
			break;
		}
	}
	ircsnprintf( nicknumbuf, 6, "%sAA%c", neostatsbase64,( i+'A' ) );
	send_cmd( "%s %s %s 1 %lu %s %s %s AAAAAA %s :%s", neostatsbase64, TOK_NICK, nick, ts, ident, host, newmode, nicknumbuf, realname );
	set_nick_base64( nick, nicknumbuf );
}

void send_ping( const char *source, const char *reply, const char *target )
{
	send_cmd( "%s %s %s :%s", neostatsbase64, TOK_PING, reply, target );
}

void send_umode( const char *source, const char *target, const char *mode )
{
	send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_MODE, target, mode );
}

void send_numeric( const char *source, const int numeric, const char *target, const char *buf )
{
	send_cmd( "%s %d %s :%s", neostatsbase64, numeric, nick_to_base64( target ), buf );
}

void send_pong( const char *reply )
{
	send_cmd( "%s %s %s :%s", neostatsbase64, TOK_PONG, neostatsbase64, reply );
}

void send_kill( const char *source, const char *target, const char *reason )
{
	send_cmd( "%s %s %s :%s", neostatsbase64, TOK_KILL, nick_to_base64( target ), reason );
}

void send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts )
{
	send_cmd( "%s %s %s %lu", nick_to_base64( oldnick ), TOK_NICK, newnick, ts );
}

void send_invite( const char *source, const char *target, const char *chan ) 
{
	send_cmd( "%s %s %s %s", nick_to_base64( source ), TOK_INVITE, target, chan );
}

void send_kick( const char *source, const char *chan, const char *target, const char *reason )
{
	send_cmd( "%s %s %s %s :%s", nick_to_base64( source ), TOK_KICK, chan, nick_to_base64( target ),( reason ? reason : "No Reason Given" ) );
}

void send_wallops( const char *source, const char *buf )
{
	char* b64source;
	if( !( b64source = server_to_base64( source ) ) ) {
		b64source = nick_to_base64( source );
	}
	send_cmd( "%s %s :%s", b64source, TOK_WALLUSERS, buf );
}

void send_end_of_burst( void )
{
	send_cmd( "%s %s", neostatsbase64, TOK_END_OF_BURST );
}

void send_end_of_burst_ack( void )
{
	if( !is_synched ) {
		do_synch_neostats( );
		send_end_of_burst( );
	}
	send_cmd( "%s %s", neostatsbase64, TOK_END_OF_BURST_ACK );
}

void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( "%s %s * +%s@%s %lu :%s", neostatsbase64, TOK_GLINE, ident, host, length, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( "%s %s * -%s@%s", neostatsbase64, TOK_GLINE, ident, host );
}

void send_privmsg( const char *source, const char *target, const char *buf )
{
	if( target[0] == '#' ) {
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_PRIVATE, target, buf );
	} else {
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_PRIVATE, nick_to_base64( target ), buf );
	}
}

void send_notice( const char *source, const char *target, const char *buf )
{
	if( target[0] == '#' ) {
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_NOTICE, target, buf );
	} else {
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_NOTICE, nick_to_base64( target ), buf );
	}
}

void send_globops( const char *source, const char *buf )
{
	char* b64source;
	if( !( b64source = server_to_base64( source ) ) ) {
		b64source = nick_to_base64( source );
	}
	send_cmd( "%s %s :%s", b64source, TOK_WALLOPS, buf );
}

void send_svstime( const char *source, const unsigned long ts )
{
	send_cmd( "%s %s %lu", server_to_base64( source ), TOK_SETTIME, ts );
}

void send_serverrequptime( const char *source, const char *target )
{
	send_cmd( "%s %s u :%s", nick_to_base64( source ), TOK_STATS, server_to_base64( target ) );
}

void send_serverreqversion( const char *source, const char *target )
{
	send_cmd( "%s %s %s", server_to_base64( source ), TOK_VERSION, server_to_base64( target ) );
}

static void m_stats( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_stats( b64origin, argv[0] );
}

/* ABAAB V :Bj */
static void m_version( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_version( b64origin, base64_to_server( argv[0] ) );
}

static void m_motd( char *origin, char **argv, int argc, int srv )
{
	do_motd( base64_to_nick( origin ), base64_to_server( argv[0] ) );
}

static void m_admin( char *origin, char **argv, int argc, int srv )
{
	do_admin( base64_to_nick( origin ), base64_to_server( argv[0] ) );
}

/* m_server
 *
 * argv[0] = servername
 * argv[1] = hopcount
 * argv[2] = start timestamp
 * argv[3] = link timestamp
 * argv[4] = major protocol version: P10/P11
 * argv[5] = YMM, YMMM or YYMMM; where 'YY' is the server numeric and
 *      "MMM" is the numeric nick mask of this server.
 * argv[6] = 0( not used yet, mandatory unsigned int after u2.10.06 )
 * argv[argc-1] = serverinfo
 * NumServ( sptr ) SERVER name hop 0 TSL PROT YxxCap 0 :info
 */
/*
SERVER srvname hop TSBoot TSLink Prot numcap modes :desc
AB S srvname hop TSBoot TSLink Prot numcap modes :desc
*/
static void m_server( char *origin, char **argv, int argc, int srv )
{
	if( srv == 2 ) {
		do_server( argv[0], NULL, argv[1], NULL, argv[argc-1], 0 );
	} else {
		do_server( argv[0], base64_to_server( origin ), argv[1], NULL, argv[argc-1], srv );
	}
	argv[5][2] = '\0';
	set_server_base64( argv[0], argv[5] );
}

/* R: AB SQ mark.local.org 0 :Ping timeout */
/* R: ABAAV SQ york.gose.org 1076280461 :relink */
static void m_squit( char *origin, char **argv, int argc, int srv )
{
	char* b64arg0;
	if( ( b64arg0 = base64_to_server( argv[0] ) ) ) {
		do_squit( b64arg0, argv[2] );
	} else {
		do_squit( argv[0], argv[2] );
	}
}

static void m_quit( char *origin, char **argv, int argc, int srv )
{
	do_quit( base64_to_nick( origin ), argv[0] );
}

/* R: ABAAE M Mark :+i */
/* R: ABAAH M #c3 +tn */
/* R: ABAAG M #chan1 +v ABAAH */
static void m_mode( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	if( argv[0][0] == '#' ) {
		char **av;
		int ac = 0;
		int j = 2;
		int add = 0;
		char *modes;

		AddStringToList( &av, argv[0], &ac );
		modes = argv[1];
		AddStringToList( &av, argv[1], &ac );

		while( *modes ) {
			unsigned int mask;
			unsigned int flags;      

			mask = CmodeCharToMask( *modes );
			flags = CmodeCharToFlags( *modes );

			switch( *modes ) {
				case '+':
					add = 1;
					break;
				case '-':
					add = 0;
					break;
				default:
					if( flags&NICKPARAM ) {
						AddStringToList( &av, base64_to_nick( argv[j] ), &ac );
						j++;
					} else if( add ) {
						if( mask == CMODE_LIMIT ) {
							AddStringToList( &av, argv[j], &ac );
							j++;
						} else if( mask == CMODE_KEY ) {
							AddStringToList( &av, argv[j], &ac );
							j++;
						} else if( flags ) {
							AddStringToList( &av, argv[j], &ac );
							j++;
						}
					} else {
						if( mask == CMODE_KEY ) {
							AddStringToList( &av, argv[j], &ac );
							j++;
						}
					}
			}
			modes++;
		}

		do_mode_channel( b64origin, av, ac );
		ns_free( av );
	} else {
		do_mode_user( argv[0], argv[1] );
	}
}
static void m_kill( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_kill( b64origin, base64_to_nick( argv[0] ), argv[1] );
}

static void m_away( char *origin, char **argv, int argc, int srv )
{
	char *buf;

	if( argc > 0 ) {
		buf = joinbuf( argv, argc, 0 );
		do_away( base64_to_nick( origin ), buf );
		ns_free( buf );
	} else {
		do_away( base64_to_nick( origin ), NULL );
	}
}

/*
1 <nickname>
2 <hops>
3 <TS>
4 <userid>
5 <host>
6 [<+modes>]
7+ [<mode parameters>]
-3 <base64 IP>
-2 <numeric>
-1 <fullname>
*/
/* R: AB N Mark 1 1076011621 a xxx.xxx.xxx.xxx DAqO4N ABAAB :M */
/* R: AB N TheEggMan 1 1076104492 ~eggy 64.XX.XXX.XXX +oiwg BAFtnj ABAAA :eggy */
/* R: ABAAH N m2 1076077934 */
/*
<reed> in a generated burst message, the users must be sorted by the modes: first users w/o modes, then users with voice, then with op, then with op+voice: num,num:v,num:o,num:ov
*/

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( argc > 2 ) {
		char IPAddress[32];
		unsigned long IP;
		const char *modes;
		const char *modeptr;
		const char *account = NULL;
		const char *sethost = NULL;
		int param;

		modes =( argv[5][0] == '+' ) ? argv[5]: NULL;
		if( modes ) {
			param = 6;
			for( modeptr = modes; *modeptr; ++modeptr ) {
				switch( *modeptr ) {
				case 'r':
					account = argv[param++];
					break;
				case 'h':
					sethost = argv[param++];
					break;
				default:
					break;
				} /* switch( *modeptr ) */
			} /* for( ) */
		} /* if( modes ) */

		IP = htonl( base64toIP( argv[argc-3] ) );
		ircsnprintf( IPAddress, 32, "%lu", IP );

		/*       nick,    hopcount, TS,     user,    host, */       
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], 
			/* server, ip, servicestamp, modes, */
			base64_to_server( origin ), IPAddress, NULL, modes,
			/* vhost, realname, numeric, smodes */ 
			NULL, argv[argc-1], argv[argc-2], NULL );
	} else {
		do_nickchange( base64_to_nick( origin ), argv[0], argv[1] );
	}
}

static void m_topic( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_topic( argv[0], b64origin, NULL, argv[argc-1] );
}

static void m_kick( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_kick( b64origin, argv[0], base64_to_nick( argv[1] ), argv[2] );
}

/* R: ABAAE C #chan 1076069009 */
static void m_create( char *origin, char **argv, int argc, int srv )
{
	do_join( base64_to_nick( origin ), argv[0], argv[1] );
}

static void m_join( char *origin, char **argv, int argc, int srv )
{
	do_join( base64_to_nick( origin ), argv[0], argv[1] );
}

static void m_part( char *origin, char **argv, int argc, int srv )
{
	do_part( base64_to_nick( origin ), argv[0], argv[1] );
}

static void m_ping( char *origin, char **argv, int argc, int srv )
{
	do_ping( base64_to_server( origin ), argv[1] );
}

/* R: AB G !1076065765.431368 stats.mark.net 1076065765.431368 */
static void m_pong( char *origin, char **argv, int argc, int srv )
{
	do_pong( base64_to_server( origin ), argv[1] );
}

/*
1 <channel>
2 <timestamp>
3+ [<modes> [<mode extra parameters>]] [<users>] [<bans>]
*/
/* R: AB B #chan 1076064445 ABAAA:o */
/* R: AB B #c3 1076083205 +tn ABAAH:o */
/*
 * parv[0] = channel name
 * parv[1] = channel timestamp
 * The meaning of the following parv[]'s depend on their first character:
 * If parv[n] starts with a '+':
 * Net burst, additive modes
 *   parv[n] = <mode>
 *   parv[n+1] = <param>( optional )
 *   parv[n+2] = <param>( optional )
 * If parv[n] starts with a '%', then n will be parc-1:
 *   parv[n] = %<ban> <ban> <ban> ...
 * If parv[n] starts with another character:
 *   parv[n] = <nick>[:<mode>],<nick>[:<mode>],...
 *   where <mode> is the channel mode( ov ) of nick and all following nicks.
 *
 * Example:
 * "S BURST #channel 87654321 +ntkl key 123 AAA,AAB:o,BAA,BAB:ov :%ban1 ban2"
*/

static char ircd_buf[BUFSIZE];

static void m_burst( char *origin, char **argv, int argc, int srv )
{
	int param; 

	/* IRCu passes this information in a stupid order so we must first
	 * find and process clients and ignore modes to "create" the channel 
	 * then process modes and ignore clients - look into better system for NS2.6
	 */
	param = 2;
	while( param < argc ) {
	    switch( argv[param][0] ) {
			case '+': /* mode string */
			{
				char *modes;

				modes = argv[param];
				param++;
				modes++;
				while( *modes ) {
					if( CmodeCharToFlags( *modes ) & MODEPARAM ) {
						param ++;
					}
					modes++;
				}
				break;
			}
		    case '%': /* bans */
				/* ignored for now */
				param++;
				break;
		    case '~': /* ban exceptions */
				/* ignored for now */
				param++;
				break;
		    default: /* clients */
			{
				char *s, *t;
				char modechar = 0;
			
				t =( char* )argv[param];
				while( *( s = t ) ) {
					t = s + strcspn( s, "," );
					if( *t )
						*t++ = 0;
					do_join( base64_to_nick( s ), argv[0], NULL );
					if( s[5] == ':' ) {
						modechar = s[6];
					}
					if( modechar ) {
						char **av;
						int ac;
						ircsnprintf( ircd_buf, BUFSIZE, "%s +%c %s", argv[0], modechar, base64_to_nick( s ) );
						ac = split_buf( ircd_buf, &av, 0 );
						do_mode_channel( me.name, av, ac );
						ns_free( av );
					}
				}
				param++;
				break;
			}
		}
	}

	param = 2;
	while( param < argc ) {
	    switch( argv[param][0] ) {
			case '+': /* mode string */
			{
				char *modes;

				modes = argv[param];
				param++;
				modes++;
				while( *modes ) {
					char **av;
					int ac;

					if( CmodeCharToFlags( *modes ) & MODEPARAM ) {
						ircsnprintf( ircd_buf, BUFSIZE, "%s +%c %s", argv[0], *modes, argv[param] );
						param ++;
					} else {
						ircsnprintf( ircd_buf, BUFSIZE, "%s +%c", argv[0], *modes );
					}
					ac = split_buf( ircd_buf, &av, 0 );
					do_mode_channel( me.name, av, ac );
					ns_free( av );
					modes++;
				}
				break;
			}
		    case '%': /* bans */
				/* ignored for now */
				param++;
				break;
		    case '~': /* ban exceptions */
				/* ignored for now */
				param++;
				break;
		    default: /* clients */
			{
				param++;
				break;
			}
		}
	}
}

static void m_end_of_burst( char *origin, char **argv, int argc, int srv )
{
	if( ircstrcasecmp( base64_to_server( origin ), me.uplink ) == 0 ) {
		send_end_of_burst_ack( );
	}
}

static void m_wallusers( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_wallops( b64origin, argv[0] );	
}

static void m_wallops( char *origin, char **argv, int argc, int srv )
{
	char* b64origin;
	if( !( b64origin = base64_to_nick( origin ) ) ) {
		b64origin = base64_to_server( origin );
	}
	do_globops( b64origin, argv[0] );
}

static void m_vhost( char *origin, char **argv, int argc, int srv )
{
	do_vhost( base64_to_nick( argv[0] ), argv[1] );
}

/* :<source> <command> <param1> <paramN> :<last parameter> */
/* <source> <command> <param1> <paramN> :<last parameter> */
int parse( void *notused, void *rline, size_t len )
{
	char origin[64], cmd[64], *coreLine;
	char *line =( char * )rline;
	int cmdptr = 0;
	int ac = 0;
	char **av = NULL;

	SET_SEGV_LOCATION( );
	if( !( *line ) )
		return NS_FAILURE;
	dlog( DEBUG1, "------------------------BEGIN PARSE-------------------------" );
	dlog( DEBUGRX, "%s", line );
	coreLine = strpbrk( line, " " );
	if( coreLine ) {
		*coreLine = 0;
		while( isspace( *++coreLine ) );
	} else
		coreLine = line + strlen( line );
	if( ( !ircstrcasecmp( line, "SERVER" ) ) ||( !ircstrcasecmp( line, "PASS" ) ) ) {
		strlcpy( cmd, line, sizeof( cmd ) );
		dlog( DEBUG1, "cmd   : %s", cmd );
		dlog( DEBUG1, "args  : %s", coreLine );
		ac = ircsplitbuf( coreLine, &av, 1 );
		cmdptr = 2;
		dlog( DEBUG1, "0 %d", ac );
		/* really needs to be in AddServer since this is a NeoStats wide bug
		 if config uplink name does not match our uplinks server name we can
		 never find the uplink!
		*/
		if( strcmp( cmd, "SERVER" ) == 0 ) {
			strlcpy( me.uplink, av[0], MAXHOST );
		}
	} else {
		strlcpy( origin, line, sizeof( origin ) );	
		cmdptr = 0;
		line = strpbrk( coreLine, " " );
		if( line ) {
			*line = 0;
			while( isspace( *++line ) );
		} /*else
			coreLine = line + strlen( line );*/
		strlcpy( cmd, coreLine, sizeof( cmd ) );
		dlog( DEBUG1, "origin: %s", origin );
		dlog( DEBUG1, "cmd   : %s", cmd );
		dlog( DEBUG1, "args  : %s", line );
		if( line ) {
			ac = ircsplitbuf( line, &av, 1 );
		}
		dlog( DEBUG1, "0 %d", ac );
	}
	process_ircd_cmd( cmdptr, cmd, origin, av, ac );
	ns_free( av );
	dlog( DEBUG1, "-------------------------END PARSE--------------------------" );
	return NS_SUCCESS;
}

static void m_private( char *origin, char **argv, int argc, int srv )
{
	char **av;
	int ac = 0;
	int i;
	char* av0;
	
	if( argv[0][0] == '#' ) {
		av0 = argv[0];
	} else {
		av0 = base64_to_nick( argv[0] );
		/* In case a real nick came through*/
		if( av0 == NULL ) {
			av0 = argv[0];
		}
	}	
	AddStringToList( &av, av0, &ac );
	for( i = 1; i < argc; i++ ) {
		AddStringToList( &av, argv[i], &ac );
	}
	_m_private( base64_to_nick( origin ), av, ac, srv );
	ns_free( av );
}

static void m_notice( char *origin, char **argv, int argc, int srv )
{
	char **av;
	int ac = 0;
	int i;
	char* av0;
	
	if( argv[0][0] == '#' ) {
		av0 = argv[0];
	} else {
		av0 = base64_to_nick( argv[0] );
		/* In case a real nick came through*/
		if( av0 == NULL ) {
			av0 = argv[0];
		}
	}
	AddStringToList( &av, av0, &ac );
	for( i = 1; i < argc; i++ ) {
		AddStringToList( &av, argv[i], &ac );
	}
	_m_notice( base64_to_nick( origin ), av, ac, srv );
	ns_free( av );
}
