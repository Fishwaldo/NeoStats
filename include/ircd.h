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
#ifndef IRCD_H
#define IRCD_H

#define MODE_TABLE_SIZE	256

#define NICKPARAM	0x00000001
#define MODEPARAM	0x00000002
#define MULTIPARAM	0x00000004

#define MSGTOK( a ) ( ( ircd_srv.protocol & PROTOCOL_TOKEN )? TOK_##a: MSG_##a )

typedef void( *ircd_cmd_handler )( char *origin, char **argv, int argc, int srv );

typedef struct ircd_cmd {
	const char *name;
	const char *token;
	ircd_cmd_handler handler;
	unsigned int usage;
}ircd_cmd;

typedef struct mode_init {
	unsigned char mode;
	unsigned int mask;
	unsigned int flags;
	unsigned char sjoin;
} mode_init;

typedef struct ircd_server {
	int burst;
	int uprot;
	int modex;
	int nicklen;
	int gc;
	char cloak[CLOAKKEYLEN];
	int maxglobalcnt;
	int tsendsync;
	unsigned int protocol;
	unsigned int features;
} ircd_server;

typedef struct ProtocolInfo {
	/* Minimum protocols that are required for the selected protocol
	 * e.g. NOQUIT */
	const unsigned int minprotocol;
	/* Optional protocols that are negotiated during connection that the
	 * protocol module supports but will work when not available e.g. SJOIN */
	const unsigned int optprotocol;
	/* Features provided by this protocol module e.g. SVSNICK support. */
	const unsigned int features;
	char *services_umode;
	char *services_cmode;
} ProtocolInfo;

#ifndef NEOSTATSCORE
MODULEVAR extern ircd_cmd cmd_list[];
MODULEVAR extern mode_init chan_umodes[];
MODULEVAR extern mode_init chan_modes[];
MODULEVAR extern mode_init user_umodes[];
MODULEVAR extern mode_init user_smodes[];
MODULEVAR extern ProtocolInfo protocol_info;
#ifndef IGNORECOREMESSAGEDEFS
MODULEVAR extern const char MSG_PRIVATE[];
MODULEVAR extern const char TOK_PRIVATE[];
MODULEVAR extern const char MSG_NOTICE[];
MODULEVAR extern const char TOK_NOTICE[];
MODULEVAR extern const char MSG_WALLOPS[];
MODULEVAR extern const char TOK_WALLOPS[];
MODULEVAR extern const char MSG_GLOBOPS[];
MODULEVAR extern const char TOK_GLOBOPS[];
MODULEVAR extern const char MSG_STATS[];
MODULEVAR extern const char TOK_STATS[];
MODULEVAR extern const char MSG_VERSION[];
MODULEVAR extern const char TOK_VERSION[];
MODULEVAR extern const char MSG_MOTD[];
MODULEVAR extern const char TOK_MOTD[];
MODULEVAR extern const char MSG_ADMIN[];
MODULEVAR extern const char TOK_ADMIN[];
MODULEVAR extern const char MSG_CREDITS[];
MODULEVAR extern const char TOK_CREDITS[];
MODULEVAR extern const char MSG_NICK[];
MODULEVAR extern const char TOK_NICK[];
MODULEVAR extern const char MSG_MODE[];
MODULEVAR extern const char TOK_MODE[];
MODULEVAR extern const char MSG_AWAY[];
MODULEVAR extern const char TOK_AWAY[];
MODULEVAR extern const char MSG_QUIT[];
MODULEVAR extern const char TOK_QUIT[];
MODULEVAR extern const char MSG_JOIN[];
MODULEVAR extern const char TOK_JOIN[];
MODULEVAR extern const char MSG_PART[];
MODULEVAR extern const char TOK_PART[];
MODULEVAR extern const char MSG_KICK[];
MODULEVAR extern const char TOK_KICK[];
MODULEVAR extern const char MSG_INVITE[];
MODULEVAR extern const char TOK_INVITE[];
MODULEVAR extern const char MSG_PING[];
MODULEVAR extern const char TOK_PING[];
MODULEVAR extern const char MSG_PONG[];
MODULEVAR extern const char TOK_PONG[];
MODULEVAR extern const char MSG_SERVER[];
MODULEVAR extern const char TOK_SERVER[];
MODULEVAR extern const char MSG_SQUIT[];
MODULEVAR extern const char TOK_SQUIT[];
MODULEVAR extern const char MSG_SVINFO[];
MODULEVAR extern const char TOK_SVINFO[];
MODULEVAR extern const char MSG_PROTOCTL[];
MODULEVAR extern const char TOK_PROTOCTL[];
MODULEVAR extern const char MSG_CAPAB[];
MODULEVAR extern const char MSG_CAPAB[];
MODULEVAR extern const char MSG_PASS[];
MODULEVAR extern const char TOK_PASS[];
MODULEVAR extern const char MSG_TOPIC[];
MODULEVAR extern const char TOK_TOPIC[];
MODULEVAR extern const char MSG_CHATOPS[];
MODULEVAR extern const char TOK_CHATOPS[];
MODULEVAR extern const char MSG_ERROR[];
MODULEVAR extern const char TOK_ERROR[];
MODULEVAR extern const char MSG_KILL[];
MODULEVAR extern const char TOK_KILL[];
MODULEVAR extern const char MSG_SETNAME[];
MODULEVAR extern const char TOK_SETNAME[];
MODULEVAR extern const char MSG_SETHOST[];
MODULEVAR extern const char TOK_SETHOST[];
MODULEVAR extern const char MSG_SETIDENT[];
MODULEVAR extern const char TOK_SETIDENT[];
MODULEVAR extern const char MSG_SVSNICK[];
MODULEVAR extern const char TOK_SVSNICK[];
MODULEVAR extern const char MSG_SVSJOIN[];
MODULEVAR extern const char TOK_SVSJOIN[];
MODULEVAR extern const char MSG_SVSPART[];
MODULEVAR extern const char TOK_SVSPART[];
MODULEVAR extern const char MSG_SVSMODE[];
MODULEVAR extern const char TOK_SVSMODE[];
MODULEVAR extern const char MSG_SVSKILL[];
MODULEVAR extern const char TOK_SVSKILL[];
#endif /* IGNORECOREMESSAGEDEFS */
#endif

EXPORTVAR extern ircd_server ircd_srv;

extern const int proto_maxhost;
extern const int proto_maxpass;
extern const int proto_maxnick;
extern const int proto_maxuser;
extern const int proto_maxrealname;
extern const int proto_chanlen;
extern const int proto_topiclen;

EXPORTFUNC int ircsplitbuf( char *buf, char ***argv, int colon_special );
EXPORTFUNC void process_ircd_cmd( int cmdptr, char *cmd, char *origin, char **av, int ac );

/* IRCD protocol module API */
EXPORTFUNC void _m_private( char *origin, char **argv, int argc, int cmdptr );
EXPORTFUNC void _m_notice( char *origin, char **argv, int argc, int cmdptr );
EXPORTFUNC void _m_pass( char *origin, char **argv, int argc, int cmdptr );
EXPORTFUNC void _m_protoctl( char *origin, char **argv, int argc, int cmdptr );
#define _m_capab _m_protoctl
EXPORTFUNC void _m_version( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_motd( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_admin( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_credits( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_stats( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_ping( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_pong( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_quit( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_topic( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_join( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_part( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_kick( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_away( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_kill( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_squit( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_mode( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_setname (char *origin, char **argv, int argc, int srv);
EXPORTFUNC void _m_sethost (char *origin, char **argv, int argc, int srv);
EXPORTFUNC void _m_setident (char *origin, char **argv, int argc, int srv);
EXPORTFUNC void _m_svsjoin( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_svspart( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_globops( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_wallops( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_chatops( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_svinfo( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_error( char *origin, char **argv, int argc, int srv );

EXPORTFUNC void do_synch_neostats( void );
EXPORTFUNC void do_motd( const char *nick, const char *remoteserver );
EXPORTFUNC void do_admin( const char *nick, const char *remoteserver );
EXPORTFUNC void do_credits( const char *nick, const char *remoteserver );
EXPORTFUNC void do_stats( const char *nick, const char *what );
EXPORTFUNC void do_ping( const char *origin, const char *destination );
EXPORTFUNC void do_pong( const char *origin, const char *destination );
EXPORTFUNC void do_version( const char *nick, const char *remoteserver );
EXPORTFUNC void do_protocol( char *origin, char **argv, int argc );
#define do_capab do_protocol
EXPORTFUNC void do_sjoin( char *tstime, char *channame, char *modes, char *sjoinnick, char **argv, int argc );
EXPORTFUNC void do_netinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname );
EXPORTFUNC void do_snetinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname );
EXPORTFUNC void do_join( const char *nick, const char *chanlist, const char *keys );
EXPORTFUNC void do_part( const char *nick, const char *chan, const char *reason );
EXPORTFUNC void do_nick( const char *nick, const char *hopcount, const char *TS, 
		const char *user, const char *host, const char *server, 
		const char *ip, const char *servicestamp, const char *modes, 
		const char *vhost, const char *realname, const char *numeric, 
		const char *smodes );
EXPORTFUNC void do_client( const char *nick, const char *hopcount, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *servicestamp, 
		const char *ip, const char *realname );
EXPORTFUNC void do_quit( const char *target, const char *quitmsg );
EXPORTFUNC void do_kill( const char *source, const char *target, const char *killmsg );
EXPORTFUNC void do_squit( const char *name, const char *reason );
EXPORTFUNC void do_kick( const char *kickby, const char *chan, const char *kicked, const char *kickreason );
EXPORTFUNC void do_svinfo( void );
EXPORTFUNC void do_vctrl( const char *uprot, const char *nicklen, const char *modex, const char *gc, const char *netname );
EXPORTFUNC void do_smode( const char *target, const char *modes );
EXPORTFUNC void do_mode_user( const char *target, const char *modes );
EXPORTFUNC void do_mode_channel( char *origin, char **argv, int argc );
EXPORTFUNC void do_svsmode_user( const char *target, const char *modes, const char *ts );
/* These are the same for now but we might need to be different in the 
 * future so use macros
 */
#define do_svsmode_channel do_mode_channel
EXPORTFUNC void do_away( const char *target, const char *reason );
EXPORTFUNC void do_vhost( const char *target, const char *vhost );
EXPORTFUNC void do_nickchange( const char *oldnick, const char *newnick, const char *ts );
EXPORTFUNC void do_topic( const char *chan, const char *owner, const char *ts, const char *topic );
EXPORTFUNC void do_server( const char *name, const char *uplink, const char *hops, const char *numeric, const char *infoline, int srv );
EXPORTFUNC void do_burst( char *origin, char **argv, int argc );
EXPORTFUNC void do_swhois( char *who, char *swhois );
EXPORTFUNC void do_tkl( const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason );
EXPORTFUNC void do_eos( const char *name );
EXPORTFUNC void do_setname( const char *nick, const char *realname );
EXPORTFUNC void do_sethost( const char *nick, const char *host );
EXPORTFUNC void do_setident( const char *nick, const char *ident );

EXPORTFUNC void do_globops( char *origin, char *message );
EXPORTFUNC void do_wallops( char *origin, char *message );
EXPORTFUNC void do_chatops( char *origin, char *message );

/* Defined in ircd specific files */
MODULEFUNC void send_privmsg( const char *source, const char *target, const char *buf );
MODULEFUNC void send_notice( const char *source, const char *target, const char *buf );
MODULEFUNC void send_globops( const char *source, const char *buf );
MODULEFUNC void send_wallops( const char *source, const char *buf );
MODULEFUNC void send_numeric( const char *source, const int numeric, const char *target, const char *buf );
MODULEFUNC void send_umode( const char *source, const char *target, const char *mode );
MODULEFUNC void send_join( const char *source, const char *chan, const char *key, const unsigned long ts );
MODULEFUNC void send_sjoin( const char *source, const char *who, const char *chan, const unsigned long ts );
MODULEFUNC void send_part( const char *source, const char *chan, const char *reason );
MODULEFUNC void send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts );
MODULEFUNC void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts );
MODULEFUNC void send_quit( const char *source, const char *quitmsg );
MODULEFUNC void send_kill( const char *source, const char *target, const char *reason );
MODULEFUNC void send_kick( const char *source, const char *chan, const char *target, const char *reason );
MODULEFUNC void send_invite( const char *source, const char *target, const char *chan );
MODULEFUNC void send_svskill( const char *source, const char *target, const char *reason );
MODULEFUNC void send_svsmode( const char *source, const char *target, const char *modes );
MODULEFUNC void send_svshost( const char *source, const char *target, const char *vhost );
MODULEFUNC void send_svsjoin( const char *source, const char *target, const char *chan );
MODULEFUNC void send_svspart( const char *source, const char *target, const char *chan );
MODULEFUNC void send_svsnick( const char *source, const char *target, const char *newnick, const unsigned long ts );
MODULEFUNC void send_swhois( const char *source, const char *target, const char *swhois );
MODULEFUNC void send_smo( const char *source, const char *umodetarget, const char *msg );
MODULEFUNC void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts );
MODULEFUNC void send_rakill( const char *source, const char *host, const char *ident );
MODULEFUNC void send_sqline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unsqline( const char *source, const char *mask );
MODULEFUNC void send_sgline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unsgline( const char *source, const char *mask );
MODULEFUNC void send_zline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unzline( const char *source, const char *mask );
MODULEFUNC void send_ping( const char *source, const char *reply, const char *target );
MODULEFUNC void send_pong( const char *reply );
MODULEFUNC void send_server( const char *source, const char *name, const int numeric, const char *infoline );
MODULEFUNC void send_squit( const char *server, const char *quitmsg );
MODULEFUNC void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname );
MODULEFUNC void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink );
MODULEFUNC void send_netinfo( const char *source, const int prot, const char *cloak, const char *netname, const unsigned long ts );
MODULEFUNC void send_snetinfo( const char *source, const int prot, const char *cloak, const char *netname, const unsigned long ts );
MODULEFUNC void send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow );
MODULEFUNC void send_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char *netname );
MODULEFUNC void send_burst( int b );
MODULEFUNC void send_svstime( const char *source, const unsigned long ts );
MODULEFUNC void send_setname( const char *nick, const char *realname );
MODULEFUNC void send_sethost( const char *nick, const char *host );
MODULEFUNC void send_setident( const char *nick, const char *ident );
MODULEFUNC void send_serverrequptime( const char *source, const char *target );
MODULEFUNC void send_serverreqversion( const char *source, const char *target );

MODULEFUNC void cloakhost(  char *host  );

int InitIrcd(  void  );
int irc_connect(  const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink  );
int irc_nick(  const char *nick, const char *user, const char *host, const char *realname, const char *modes  );
int irc_server(  const char *name, const int numeric, const char *infoline  );
int irc_squit(  const char *server, const char *quitmsg  );
/*int snetinfo_cmd( void );*/
/*int ssvinfo_cmd( void );*/
/*int sburst_cmd( int b );*/
/*int seob_cmd( const char *server );*/
int irc_smo(  const char *source, const char *umodetarget, const char *msg  );

EXPORTFUNC void send_cmd( char *fmt, ... )__attribute__( ( format( printf,1,2 ) ) ); /* 2=format 3=params */

MODULEFUNC int parse (void *notused, void *rline, size_t len);
int (*irc_parse) (void *notused, void *rline, size_t len);

#endif
