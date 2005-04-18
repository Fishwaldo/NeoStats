/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "ircd.h"

/* Messages/Tokens */
const char MSG_PRIVATE[] = "PRIVMSG";/* PRIV */
const char TOK_PRIVATE[] = "!";/* 33 */
const char MSG_WHOIS[] = "WHOIS";/* WHOI */
const char TOK_WHOIS[] = "#";/* 35 */
const char MSG_WHOWAS[] = "WHOWAS";/* WHOW */
const char TOK_WHOWAS[] = "$";/* 36 */
const char MSG_USER[] = "USER";/* USER */
const char TOK_USER[] = "%";/* 37 */
const char MSG_NICK[] = "NICK";/* NICK */
const char TOK_NICK[] = "&";/* 38 */
const char MSG_SERVER[] = "SERVER";/* SERV */
const char TOK_SERVER[] = "'";/* 39 */
const char MSG_LIST[] = "LIST";/* LIST */
const char TOK_LIST[] = "(";/* 40 */
const char MSG_TOPIC[] = "TOPIC";/* TOPI */
const char TOK_TOPIC[] = ")";/* 41 */
const char MSG_INVITE[] = "INVITE";/* INVI */
const char TOK_INVITE[] = "*";/* 42 */
const char MSG_VERSION[] = "VERSION";/* VERS */
const char TOK_VERSION[] = "+";/* 43 */
const char MSG_QUIT[] = "QUIT";/* QUIT */
const char TOK_QUIT[] = ",";/* 44 */
const char MSG_SQUIT[] = "SQUIT";/* SQUI */
const char TOK_SQUIT[] = "-";/* 45 */
const char MSG_KILL[] = "KILL";/* KILL */
const char TOK_KILL[] = ".";/* 46 */
const char MSG_INFO[] = "INFO";/* INFO */
const char TOK_INFO[] = "/";/* 47 */
const char MSG_LINKS[] = "LINKS";/* LINK */
const char TOK_LINKS[] = "0";/* 48 */
const char MSG_SUMMON[] = "SUMMON";/* SUMM */
const char TOK_SUMMON[] = "1";/* 49 */
const char MSG_STATS[] = "STATS";/* STAT */
const char TOK_STATS[] = "2";/* 50 */
const char MSG_USERS[] = "USERS";/* USER -> USRS */
const char TOK_USERS[] = "3";/* 51 */
const char MSG_HELP[] = "HELP";/* HELP */
const char MSG_HELPOP[] = "HELPOP";/* HELP */
const char TOK_HELP[] = "4";/* 52 */
const char MSG_ERROR[] = "ERROR";/* ERRO */
const char TOK_ERROR[] = "5";/* 53 */
const char MSG_AWAY[] = "AWAY";/* AWAY */
const char TOK_AWAY[] = "6";/* 54 */
const char MSG_CONNECT[] = "CONNECT";/* CONN */
const char TOK_CONNECT[] = "7";/* 55 */
const char MSG_PING[] = "PING";/* PING */
const char TOK_PING[] = "8";/* 56 */
const char MSG_PONG[] = "PONG";/* PONG */
const char TOK_PONG[] = "9";/* 57 */
const char MSG_OPER[] = "OPER";/* OPER */
const char TOK_OPER[] = ";";/* 59 */
const char MSG_PASS[] = "PASS";/* PASS */
const char TOK_PASS[] = "<";/* 60 */
const char MSG_WALLOPS[] = "WALLOPS";/* WALL */
const char TOK_WALLOPS[] = "=";/* 61 */
const char MSG_TIME[] = "TIME";/* TIME */
const char TOK_TIME[] = ">";/* 62 */
const char MSG_NAMES[] = "NAMES";/* NAME */
const char TOK_NAMES[] = "?";/* 63 */
const char MSG_ADMIN[] = "ADMIN";/* ADMI */
const char TOK_ADMIN[] = "@";/* 64 */
const char MSG_NOTICE[] = "NOTICE";/* NOTI */
const char TOK_NOTICE[] = "B";/* 66 */
const char MSG_JOIN[] = "JOIN";/* JOIN */
const char TOK_JOIN[] = "C";/* 67 */
const char MSG_PART[] = "PART";/* PART */
const char TOK_PART[] = "D";/* 68 */
const char MSG_LUSERS[] = "LUSERS";/* LUSE */
const char TOK_LUSERS[] = "E";/* 69 */
const char MSG_MOTD[] = "MOTD";/* MOTD */
const char TOK_MOTD[] = "F";/* 70 */
const char MSG_MODE[] = "MODE";/* MODE */
const char TOK_MODE[] = "G";/* 71 */
const char MSG_KICK[] = "KICK";/* KICK */
const char TOK_KICK[] = "H";/* 72 */
const char MSG_SERVICE[] = "SERVICE";/* SERV -> SRVI */
const char TOK_SERVICE[] = "I";/* 73 */
const char MSG_USERHOST[] = "USERHOST";/* USER -> USRH */
const char TOK_USERHOST[] = "J";/* 74 */
const char MSG_ISON[] = "ISON";/* ISON */
const char TOK_ISON[] = "K";/* 75 */
const char MSG_REHASH[] = "REHASH";/* REHA */
const char TOK_REHASH[] = "O";/* 79 */
const char MSG_RESTART[] = "RESTART";/* REST */
const char TOK_RESTART[] = "P";/* 80 */
const char MSG_CLOSE[] = "CLOSE";/* CLOS */
const char TOK_CLOSE[] = "Q";/* 81 */
const char MSG_DIE[] = "DIE";/* DIE */
const char TOK_DIE[] = "R";/* 82 */
const char MSG_HASH[] = "HASH";/* HASH */
const char TOK_HASH[] = "S";/* 83 */
const char MSG_DNS[] = "DNS";/* DNS -> DNSS */
const char TOK_DNS[] = "T";/* 84 */
const char MSG_SILENCE[] = "SILENCE";/* SILE */
const char TOK_SILENCE[] = "U";/* 85 */
const char MSG_AKILL[] = "AKILL";/* AKILL */
const char TOK_AKILL[] = "V";/* 86 */
const char MSG_KLINE[] = "KLINE";/* KLINE */
const char TOK_KLINE[] = "W";/* 87 */
const char MSG_UNKLINE[] = "UNKLINE";/* UNKLINE */
const char TOK_UNKLINE[] = "X";/* 88 */
const char MSG_RAKILL[] = "RAKILL";/* RAKILL */
const char TOK_RAKILL[] = "Y";/* 89 */
const char MSG_GNOTICE[] = "GNOTICE";/* GNOTICE */
const char TOK_GNOTICE[] = "Z";/* 90 */
const char MSG_GOPER[] = "GOPER";/* GOPER */
const char TOK_GOPER[] = "[";/* 91 */
const char MSG_GLOBOPS[] = "GLOBOPS";/* GLOBOPS */
const char TOK_GLOBOPS[] = "]";/* 93 */
const char MSG_LOCOPS[] = "LOCOPS";/* LOCOPS */
const char TOK_LOCOPS[] = "^";/* 94 */
const char MSG_PROTOCTL[] = "PROTOCTL";/* PROTOCTL */
const char TOK_PROTOCTL[] = "_";/* 95 */
const char MSG_WATCH[] = "WATCH";/* WATCH */
const char TOK_WATCH[] = "`";/* 96 */
const char MSG_TRACE[] = "TRACE";/* TRAC */
const char TOK_TRACE[] = "b";/* 97 */
const char MSG_SQLINE[] = "SQLINE";/* SQLINE */
const char TOK_SQLINE[] = "c";/* 98 */
const char MSG_UNSQLINE[] = "UNSQLINE";/* UNSQLINE */
const char TOK_UNSQLINE[] = "d";/* 99 */
const char MSG_SVSNICK[] = "SVSNICK";/* SVSNICK */
const char TOK_SVSNICK[] = "e";/* 100 */
const char MSG_SVSNOOP[] = "SVSNOOP";/* SVSNOOP */
const char TOK_SVSNOOP[] = "f";/* 101 */
const char MSG_IDENTIFY[] = "IDENTIFY";/* IDENTIFY */
const char TOK_IDENTIFY[] = "g";/* 102 */
const char MSG_SVSKILL[] = "SVSKILL";/* SVSKILL */
const char TOK_SVSKILL[] = "h";/* 103 */
const char MSG_NICKSERV[] = "NICKSERV";/* NICKSERV */
const char MSG_NS[] = "NS";
const char TOK_NICKSERV[] = "i";/* 104 */
const char MSG_CHANSERV[] = "CHANSERV";/* CHANSERV */
const char MSG_CS[] = "CS";
const char TOK_CHANSERV[] = "j";/* 105 */
const char MSG_OPERSERV[] = "OPERSERV";/* OPERSERV */
const char MSG_OS[] = "OS";
const char TOK_OPERSERV[] = "k";/* 106 */
const char MSG_MEMOSERV[] = "MEMOSERV";/* MEMOSERV */
const char MSG_MS[] = "MS";
const char TOK_MEMOSERV[] = "l";/* 107 */
const char MSG_SERVICES[] = "SERVICES";/* SERVICES */
const char TOK_SERVICES[] = "m";/* 108 */
const char MSG_SVSMODE[] = "SVSMODE";/* SVSMODE */
const char TOK_SVSMODE[] = "n";/* 109 */
const char MSG_SAMODE[] = "SAMODE";/* SAMODE */
const char TOK_SAMODE[] = "o";/* 110 */
const char MSG_CHATOPS[] = "CHATOPS";/* CHATOPS */
const char TOK_CHATOPS[] = "p";/* 111 */
const char MSG_ZLINE[] = "ZLINE";/* ZLINE */
const char TOK_ZLINE[] = "q";/* 112 */
const char MSG_UNZLINE[] = "UNZLINE";/* UNZLINE */
const char TOK_UNZLINE[] = "r";/* 113 */
const char MSG_HELPSERV[] = "HELPSERV";/* HELPSERV */
const char MSG_HS[] = "HS";
const char TOK_HELPSERV[] = "s";/* 114 */
const char MSG_RULES[] = "RULES";/* RULES */
const char TOK_RULES[] = "t";/* 115 */
const char MSG_MAP[] = "MAP";/* MAP */
const char TOK_MAP[] = "u";/* 117 */
const char MSG_SVS2MODE[] = "SVS2MODE";/* SVS2MODE */
const char TOK_SVS2MODE[] = "v";/* 118 */
const char MSG_DALINFO[] = "DALINFO";/* dalinfo */
const char TOK_DALINFO[] = "w";/* 119 */
const char MSG_ADMINCHAT[] = "ADCHAT";/* Admin chat */
const char TOK_ADMINCHAT[] = "x";/* 120 */
const char MSG_MKPASSWD[] = "MKPASSWD";/* MKPASSWD */
const char TOK_MKPASSWD[] = "y";/* 121 */
const char MSG_ADDLINE[] = "ADDLINE";/* ADDLINE */
const char TOK_ADDLINE[] = "z";/* 122 */
const char MSG_GLINE[] = "GLINE";/* The awesome g-line */
const char TOK_GLINE[] = "}";/* 125 */
const char MSG_SJOIN[] = "SJOIN";
const char TOK_SJOIN[] = "~";
const char MSG_SETHOST[] = "SETHOST";/* sethost */
const char TOK_SETHOST[] = "AA";/* 127 4ever !;) */
const char MSG_NACHAT[] = "NACHAT";/* netadmin chat */
const char TOK_NACHAT[] = "AC";/* *beep* */
const char MSG_SETIDENT[] = "SETIDENT";
const char TOK_SETIDENT[] = "AD";
const char MSG_SETNAME[] = "SETNAME";/* set GECOS */
const char TOK_SETNAME[] = "AE";/* its almost unreeaaall... */
const char MSG_LAG[] = "LAG";/* Lag detect */
const char TOK_LAG[] = "AF";/* a or ? */
const char MSG_STATSERV[] = "STATSERV";/* alias */
const char TOK_STATSERV[] = "AH";
const char MSG_KNOCK[] = "KNOCK";
const char TOK_KNOCK[] = "AI";
const char MSG_CREDITS[] = "CREDITS";
const char TOK_CREDITS[] = "AJ";
const char MSG_LICENSE[] = "LICENSE";
const char TOK_LICENSE[] = "AK";
const char MSG_CHGHOST[] = "CHGHOST";
const char TOK_CHGHOST[] = "AL";
const char MSG_RPING[] = "RPING";
const char TOK_RPING[] = "AM";
const char MSG_RPONG[] = "RPONG";
const char TOK_RPONG[] = "AN";
const char MSG_NETINFO[] = "NETINFO";
const char TOK_NETINFO[] = "AO";
const char MSG_SENDUMODE[] = "SENDUMODE";
const char TOK_SENDUMODE[] = "AP";
const char MSG_ADDMOTD[] = "ADDMOTD";
const char TOK_ADDMOTD[] = "AQ";
const char MSG_ADDOMOTD[] = "ADDOMOTD";
const char TOK_ADDOMOTD[] = "AR";
const char MSG_SVSMOTD[] = "SVSMOTD";
const char TOK_SVSMOTD[] = "AS";
const char MSG_SMO[] = "SMO";
const char TOK_SMO[] = "AU";
const char MSG_OPERMOTD[] = "OPERMOTD";
const char TOK_OPERMOTD[] = "AV";
const char MSG_TSCTL[] = "TSCTL";
const char TOK_TSCTL[] = "AW";
const char MSG_SVSJOIN[] = "SVSJOIN";
const char TOK_SVSJOIN[] = "BR";
const char MSG_SAJOIN[] = "SAJOIN";
const char TOK_SAJOIN[] = "AX";
const char MSG_SVSPART[] = "SVSPART";
const char TOK_SVSPART[] = "BT";
const char MSG_SAPART[] = "SAPART";
const char TOK_SAPART[] = "AY";
const char MSG_CHGIDENT[] = "CHGIDENT";
const char TOK_CHGIDENT[] = "AZ";
const char MSG_SWHOIS[] = "SWHOIS";
const char TOK_SWHOIS[] = "BA";
const char MSG_SVSO[] = "SVSO";
const char TOK_SVSO[] = "BB";
const char MSG_SVSFLINE[] = "SVSFLINE";
const char TOK_SVSFLINE[] = "BC";
const char MSG_TKL[] = "TKL";
const char TOK_TKL[] = "BD";
const char MSG_VHOST[] = "VHOST";
const char TOK_VHOST[] = "BE";
const char MSG_BOTMOTD[] = "BOTMOTD";
const char TOK_BOTMOTD[] = "BF";
const char MSG_REMGLINE[] = "REMGLINE";/* remove g-line */
const char TOK_REMGLINE[] = "BG";
const char MSG_HTM[] = "HTM";
const char TOK_HTM[] = "BH";
const char MSG_UMODE2[] = "UMODE2";
const char TOK_UMODE2[] = "|";
const char MSG_DCCDENY[] = "DCCDENY";
const char TOK_DCCDENY[] = "BI";
const char MSG_UNDCCDENY[] = "UNDCCDENY";
const char TOK_UNDCCDENY[] = "BJ";
const char MSG_CHGNAME[] = "CHGNAME";
const char MSG_SVSNAME[] = "SVSNAME";
const char TOK_CHGNAME[] = "BK";
const char MSG_SHUN[] = "SHUN";
const char TOK_SHUN[] = "BL";
const char MSG_NEWJOIN[] = "NEWJOIN";/* For CR Java Chat */
const char MSG_POST[] = "POST";
const char TOK_POST[] = "BN";
const char MSG_INFOSERV[] = "INFOSERV";
const char MSG_IS[] = "IS";
const char TOK_INFOSERV[] = "BO";

const char MSG_BOTSERV[] = "BOTSERV";
const char TOK_BOTSERV[] = "BS";

const char MSG_CYCLE[] = "CYCLE";
const char TOK_CYCLE[] = "BP";

const char MSG_MODULE[] = "MODULE";
const char TOK_MODULE[] = "BQ";
/* BR and BT are in use */

const char MSG_SENDSNO[] = "SENDSNO";
const char TOK_SENDSNO[] = "Ss";

const char MSG_EOS[] = "EOS";
const char TOK_EOS[] = "ES";

/* Umodes */				
#define UMODE_FAILOP		0x00200000
#define UMODE_SERVNOTICE	0x00400000
#define UMODE_NOCTCP		0x00800000
#define UMODE_WEBTV			0x01000000
#define UMODE_WHOIS			0x02000000
#define UMODE_SECURE		0x04000000
#define UMODE_VICTIM		0x08000000
#define UMODE_HIDEOPER		0x10000000
#define UMODE_SETHOST		0x20000000
#define UMODE_STRIPBADWORDS	0x40000000
#define UMODE_HIDEWHOIS		0x80000000

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_umode2( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_eos( char *origin, char **argv, int argc, int srv );
static void m_netinfo( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );
static void m_svsnick( char *origin, char **argv, int argc, int srv );
static void m_smo( char *origin, char **argv, int argc, int srv );
static void m_swhois( char *origin, char **argv, int argc, int srv );
static void m_tkl( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN | PROTOCOL_NICKIP | PROTOCOL_NICKv2 | PROTOCOL_SJ3,
	/* Features supported by this IRCd */
	FEATURE_UMODECLOAK,
	/* Max host length */
	128,
	/* Max password length */
	32,
	/* Max nick length */
	30,
	/* Max user length */
	10,
	/* Max real name length */
	50,
	/* Max channel name length */
	32,
	/* Max topic length */
	307,
	/* Default operator modes for NeoStats service bots */
	"+OwoSq",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

ircd_cmd cmd_list[] = 
{
	/*Message	Token	Function	usage */
	{MSG_SERVER, TOK_SERVER, m_server, 0},
	{MSG_UMODE2, TOK_UMODE2, m_umode2, 0},
	{MSG_SVSMODE, TOK_SVSMODE, m_svsmode, 0},
	{MSG_SVS2MODE, TOK_SVS2MODE, m_svsmode, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_NETINFO, TOK_NETINFO, m_netinfo, 0},
	{MSG_SJOIN, TOK_SJOIN, m_sjoin, 0},
	{MSG_SVSNICK, TOK_SVSNICK, m_svsnick, 0},
	{MSG_SWHOIS, TOK_SWHOIS, m_swhois, 0},
	{MSG_SMO, TOK_SMO, m_smo, 0},
	{MSG_EOS, TOK_EOS, m_eos, 0},
	{MSG_TKL, TOK_TKL, m_tkl, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANPROT, 0, '*'},
	{'q', CUMODE_CHANOWNER, 0, '~'},
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTR, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'c', CMODE_NOCOLOR, 0},
	{'O', CMODE_OPERONLY, 0},
	{'A', CMODE_ADMONLY, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'Q', CMODE_NOKICKS, 0},
	{'S', CMODE_STRIP, 0},
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'K', CMODE_NOKNOCK, 0},
	{'V', CMODE_NOINVITE, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
	{'M', CMODE_MODREG, 0},
	{'G', CMODE_STRIPBADWORDS, 0},
	{'C', CMODE_NOCTCP, 0},
	{'u', CMODE_AUDITORIUM, 0},
	{'z', CMODE_ONLYSECURE, 0},
	{'N', CMODE_NONICKCHANGE, 0},
	{0, 0},
};

mode_init user_umodes[] = {
	{'S', UMODE_SERVICES},
	{'N', UMODE_NETADMIN},
	{'a', UMODE_SADMIN},
	{'A', UMODE_ADMIN},
	{'C', UMODE_COADMIN},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'w', UMODE_WALLOP},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'q', UMODE_KIX},
	{'B', UMODE_BOT},
 	{'d', UMODE_DEAF},
	{'R', UMODE_RGSTRONLY},
 	{'T', UMODE_NOCTCP},
	{'V', UMODE_WEBTV},
	{'p', UMODE_HIDEWHOIS},
	{'H', UMODE_HIDEOPER},
	{'G', UMODE_STRIPBADWORDS},
	{'t', UMODE_SETHOST},
	{'x', UMODE_HIDE},
	/*{'b', UMODE_CHATOP},*/
	{'W', UMODE_WHOIS},
	{'z', UMODE_SECURE},
	{'v', UMODE_VICTIM},	
	{0, 0},
};

static const char Base64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';

int b64_decode( char const *src, unsigned char *target, int targsize )
{
	int tarindex, state, ch;
	char *pos;

	state = 0;
	tarindex = 0;

	while( ( ch = *src++ ) != '\0' ) {
		if( isspace( ch ) )	/* Skip whitespace anywhere. */
			continue;

		if( ch == Pad64 )
			break;

		pos = strchr( Base64, ch );
		if( pos == 0 ) 		/* A non-base64 character. */
			return( -1 );

		switch( state ) {
		case 0:
			if( target ) {
				if( tarindex >= targsize )
					return( -1 );
				target[tarindex] =( unsigned char )( pos - Base64 ) << 2;
			}
			state = 1;
			break;
		case 1:
			if( target ) {
				if( tarindex + 1 >= targsize )
					return( -1 );
				target[tarindex]   |= ( pos - Base64 ) >> 4;
				target[tarindex+1]  =( unsigned char )( ( pos - Base64 ) & 0x0f )
							<< 4 ;
			}
			tarindex++;
			state = 2;
			break;
		case 2:
			if( target ) {
				if( tarindex + 1 >= targsize )
					return( -1 );
				target[tarindex]   |= ( pos - Base64 ) >> 2;
				target[tarindex+1]  =( unsigned char )( ( pos - Base64 ) & 0x03 )
							<< 6;
			}
			tarindex++;
			state = 3;
			break;
		case 3:
			if( target ) {
				if( tarindex >= targsize )
					return( -1 );
				target[tarindex] |=( pos - Base64 );
			}
			tarindex++;
			state = 0;
			break;
		default:
			abort();
		}
	}

	/*
	 * We are done decoding Base-64 chars.  Let's see if we ended
	 * on a byte boundary, and/or with erroneous trailing characters.
	 */

	if( ch == Pad64 ) {		/* We got a pad char. */
		ch = *src++;		/* Skip it, get next. */
		switch( state ) {
		case 0:		/* Invalid = in first position */
		case 1:		/* Invalid = in second position */
			return( -1 );

		case 2:		/* Valid, means one byte of info */
			/* Skip any number of spaces. */
			for( ( void )NULL; ch != '\0'; ch = *src++ )
				if( !isspace( ch ) )
					break;
			/* Make sure there is another trailing = sign. */
			if( ch != Pad64 )
				return( -1 );
			ch = *src++;		/* Skip the = */
			/* Fall through to "single trailing =" case. */
			/* FALLTHROUGH */

		case 3:		/* Valid, means two bytes of info */
			/*
			 * We know this char is an =.  Is there anything but
			 * whitespace after it?
			 */
			for( ( void )NULL; ch != '\0'; ch = *src++ )
				if( !isspace( ch ) )
					return( -1 );

			/*
			 * Now make sure for cases 2 and 3 that the "extra"
			 * bits that slopped past the last full byte were
			 * zeros.  If we don't check them, they become a
			 * subliminal channel.
			 */
			if( target && target[tarindex] != 0 )
				return( -1 );
		}
	} else {
		/*
		 * We ended by seeing the end of the string.  Make sure we
		 * have no partial bytes lying around.
		 */
		if( state != 0 )
			return( -1 );
	}

	return( tarindex );
}

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
/* PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NS SJB64 TKLEXT NICKIP CHANMODES=be,kfL,l,psmntirRcOAQKVGCuzNSMT */
	send_cmd( "%s TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NICKIP VHP", MSGTOK( PROTOCTL ) );
	send_cmd( "%s %s", MSGTOK( PASS ), pass );
	send_cmd( "%s %s %d :U0-*-%d %s", MSGTOK( SERVER ), name, 1, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSGTOK( SJOIN ), ts, chan, target );
}

/* m_nick
 *  argv[0] = nickname
 * if from new client
 *  argv[1] = nick password
 * if from server:
 *  argv[1] = hopcount
 *  argv[2] = timestamp
 *  argv[3] = username
 *  argv[4] = hostname
 *  argv[5] = servername
 * if NICK version 1:
 *  argv[6] = servicestamp
 *  argv[7] = info
 * if NICK version 2:
 *  argv[6] = servicestamp
 *  argv[7] = umodes
 *  argv[8] = virthost, * if none
 *  argv[9] = info
 * if NICKIP:
 *  argv[9] = ip
 *  argv[10] = info
 */
/*
RX: & Mark 1 1089324634 mark 127.0.0.1 irc.foonet.com 0 +iowghaAxN F72CBABD.ABE021B4.D9E4BB78.IP fwAAAQ== :Mark
RX: & Mark 1 1089324634 mark 127.0.0.1 irc.foonet.com 0 +iowghaAxN F72CBABD.ABE021B4.D9E4BB78.IP :Mark
*/
void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s 0 %s * :%s", MSGTOK( NICK ), nick, ts, ident, host, server, newmode, realname );
}

void send_netinfo( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSGTOK( NETINFO ), ts, prot, cloak, netname );
}

void send_smo( const char *source, const char *umodetarget, const char *msg )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( SMO ), umodetarget, msg );
}

void send_swhois( const char *source, const char *target, const char *swhois )
{
	send_cmd( "%s %s :%s", MSGTOK( SWHOIS ), target, swhois );
}

void send_svshost( const char *source, const char *target, const char *vhost )
{
	send_cmd( ":%s %s %s %s", source, MSGTOK( CHGHOST ), target, vhost );
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s + G %s %s %s %lu %lu :%s", source, MSGTOK( TKL ), ident, host, setby,( ts + length ), ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s - G %s %s %s", source, MSGTOK( TKL ), ident, host, source );
}

void send_svstime( const char *source, const unsigned long ts )
{
	send_cmd( ":%s %s SVSTIME %lu", source, MSGTOK( TSCTL ), ts );
}

/* m_server
 *	argv[0] = servername
 *  argv[1] = hopcount
 *  argv[2] = numeric
 *  argv[3] = serverinfo
 * on old protocols, serverinfo is argv[2], and numeric is left out
 */
/*SERVER servername hopcount :U<protocol>-flags-numeric serverdesc*/
static void m_server( char *origin, char **argv, int argc, int srv )
{
	char* s = argv[argc-1];
	if( *origin== 0 ) {
		/* server desc from uplink includes extra info so we need to 
		   strip protocol, flags and numeric. We can use the first
		   space to do this*/
		while( *s != ' ' )
			s++;
		/* Strip the now leading space */
		s++;
	}
	if( argc > 3 ) {
		do_server( argv[0], origin, argv[1], argv[2], s, srv );
	} else {
		do_server( argv[0], origin, argv[1], NULL, s, srv );
	}
	
}

/* m_svsmode
 *  argv[0] - username to change mode for
 *  argv[1] - modes to change
 *  argv[2] - Service Stamp( if mode == d )
 */
static void m_svsmode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' ) {
		do_svsmode_channel( origin, argv, argc );
	} else {
		do_svsmode_user( argv[0], argv[1], argv[2] );
	}
}

/* m_umode2
 * argv[0] - modes to change
 */
static void m_umode2( char *origin, char **argv, int argc, int srv )
{
	do_mode_user( origin, argv[0] );
}

/* m_nick
 *  argv[0] = nickname
 * if from new client
 *  argv[1] = nick password
 * if from server:
 *  argv[1] = hopcount
 *  argv[2] = timestamp
 *  argv[3] = username
 *  argv[4] = hostname
 *  argv[5] = servername
 * if NICK version 1:
 *  argv[6] = servicestamp
 *  argv[7] = info
 * if NICK version 2:
 *  argv[6] = servicestamp
 *  argv[7] = umodes
 *  argv[8] = virthost, * if none
 *  argv[9] = info
 * if NICKIP:
 *  argv[9] = ip
 *  argv[10] = info
 */

int decode_ip( char *buf )
{
	int len = strlen( buf );
	char targ[25];
	struct in_addr ia;

	b64_decode( buf, targ, 25 );
	ia = *( struct in_addr * )targ;
	if( len == 8 )  /* IPv4 */
		return ia.s_addr;
	return 0;
}

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		if( ircd_srv.protocol & PROTOCOL_NICKv2 )
		{
			if( ircd_srv.protocol & PROTOCOL_NICKIP )
			{
				char ip[25];

				ircsnprintf( ip, 25, "%d", ntohl( decode_ip( argv[9] ) ) );
				do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
					ip, argv[6], argv[7], argv[8], argv[10], NULL, NULL );
			}
			else
			{
				do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
					NULL, argv[6], argv[7], argv[8], argv[9], NULL, NULL );
			}
		}
		else
		{
			do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
				NULL, argv[6], NULL, NULL, argv[9], NULL, NULL );
		}
	} else {
		do_nickchange( origin, argv[0], NULL );
	}
}

/* m_netinfo
 *  argv[0] = max global count
 *  argv[1] = time of end sync
 *  argv[2] = unreal protocol using( numeric )
 *  argv[3] = cloak-crc( > u2302 )
 *  argv[4] = free( ** )
 *  argv[5] = free( ** )
 *  argv[6] = free( ** )
 *  argv[7] = ircnet
 */
static void m_netinfo( char *origin, char **argv, int argc, int srv )
{
	do_netinfo( argv[0], argv[1], argv[2], argv[3], argv[7] );
}

/*  EOS
 *  :servername EOS
 */
static void m_eos( char *origin, char **argv, int argc, int srv )
{
	do_eos( origin );
}
    
/* m_sjoin  
 *  argv[0] = channel timestamp
 *    char *argv[], pvar[MAXMODEPARAMS][MODEBUFLEN + 3];
 *  argv[1] = channel name
 *  "ts chname :"
 * if( argc == 3 ) 
 *  argv[2] = nick names + modes - all in one parameter
 *  "ts chname modebuf :"
 *  "ts chname :"@/"""name"	OPT_SJ3
 * if( argc == 4 )
 *  argv[2] = channel modes
 *  argv[3] = nick names + modes - all in one parameter
 *  "ts chname modebuf parabuf :"
 * if( argc > 4 )
 *  argv[2] = channel modes
 *  argv[3 to argc - 2] = mode parameters
 *  argv[argc - 1] = nick names + modes
 *  "ts parabuf :parv[parc - 1]"	OPT_SJOIN | OPT_SJ3 
 */
/*    MSG_SJOIN creationtime chname    modebuf parabuf :member list */
/* R: ~         1073861298   #services +       <none>  :Mark */
static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1],( ( argc >= 4 ) ? argv[2] : "" ), origin, argv, argc );
}

/* m_svsnick
 *  argv[0] = old nickname
 *  argv[1] = new nickname
 *  argv[2] = timestamp
 */
static void m_svsnick( char *origin, char **argv, int argc, int srv )
{
	do_nickchange( argv[0], argv[1], argv[2] );
}

/* m_swhois
 *  argv[0] = nickname
 *  argv[1] = new swhois
 */
static void m_swhois( char *origin, char **argv, int argc, int srv )
{
	do_swhois( argv[0], argv[1] );
}

static void m_smo( char *origin, char **argv, int argc, int srv )
{
	/* TODO */
}

/*
 *  argv[0]  +|- 
 *  argv[1]  G   
 *  argv[2]  user 
 *  argv[3]  host 
 *  argv[4]  setby 
 *  argv[5]  expire_at 
 *  argv[6]  set_at 
 *  argv[7]  reason 

R: :server BD + G * mask setter 1074811259 1074206459 :reason
R: :server BD + Z * mask setter 0 1070062390 :reason
R: :server c dos_bot* :Reserved nickname: Dosbot
*/

static void m_tkl( char *origin, char **argv, int argc, int srv )
{
	do_tkl( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7] );
}

