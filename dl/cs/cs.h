/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
** $Id: cs.h,v 1.13 2003/08/29 11:16:00 fishwaldo Exp $
*/


/* 
** If we're compiled for Ultimate 3.x.x IRCd, use these modes and flags
*/
#if defined(ULTIMATE3) || defined(QUANTUM)
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define GUESTADMIN_MODE 'G'
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 'n'
  #define NETADMIN_MODE 'N'
  #define COTECHADMIN_MODE 't'
  #define TECHADMIN_MODE 'T'		/* Set to a number as we dont use */
  #define SERVICESADMIN_MODE 'a'
  #define NETSERVICE_MODE 'S'
#elif ULTIMATE
/*
** If we are compiled to Use Ultimate 2.8.x use these modes and flags
*/
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 't'
  #define NETADMIN_MODE 'N'
  #define TECHADMIN_MODE 'T'
  #define SERVICESADMIN_MODE 'P'
  #define NETSERVICE_MODE 'S'
  #define BOT_MODE 'B'
#endif
#ifdef MYSTIC
/*
** If we are compiled to Use Ultimate 2.8.x use these modes and flags
*/
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 't'
  #define NETADMIN_MODE 'N'
  #define TECHADMIN_MODE 'T'
  #define SERVICESADMIN_MODE 'P'
  #define NETSERVICE_MODE 'S'
  #define BOT_MODE 'B'
#endif

#ifdef UNREAL
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define COSERVERADMIN_MODE 'C'
  #define SERVERADMIN_MODE 'A'
  #define NETADMIN_MODE 'N'
  #define TECHADMIN_MODE 'T'
  #define SERVICESADMIN_MODE 'a'
  #define NETSERVICE_MODE 'S'
  #define INVISIBLE_MODE 'I'
  #define BOT_MODE 'B'
#endif

#ifdef HYBRID7
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define SERVERADMIN_MODE 'a'
#endif

#ifdef NEOIRCD
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define SERVERADMIN_MODE 'a'
#endif

#ifdef BAHAMUT
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define SERVERADMIN_MODE 'a'
#endif
