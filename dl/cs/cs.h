/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: cs.h,v 1.5 2002/09/04 08:40:27 fishwaldo Exp $
*/


/* 
** If we're compiled for Ultimate 3.x.x IRCd, use these modes and flags
*/
#ifdef ULTIMATE3
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define GUESTADMIN_MODE 'j'
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 't'
  #define NETADMIN_MODE 'T'
  #define TECHADMIN_MODE '7'		/* Set to a number as we dont use */
  #define SERVICESADMIN_MODE 'P'
  #define NETSERVICE_MODE 'S'
  #define INVISIBLE_MODE '8'		/* Set to a number as we dont use */
  #define BOT_MODE '9'			/* Set to a number as we dont use */
#elif ULTIMATE
/*
** If we are compiled to Use Ultimate 2.8.x use these modes and flags
*/
  #define LOCOP_MODE 'O'
  #define OPER_MODE, 'o'
  #define GUESTADMIN_MODE '7'		/* Set to a number as we dont use */
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 't'
  #define NETADMIN_MODE 'N'
  #define TECHADMIN_MODE 'T'
  #define SERVICESADMIN_MODE 'P'
  #define NETSERVICE_MODE 'S'
  #define INVISIBLE_MODE '8'            /* Set to a number as we dont use */
  #define BOT_MODE 'B'
#endif

#ifdef UNREAL
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define GUESTADMIN_MODE '7'		/* Set to a number we dont use */
  #define COSERVERADMIN_MODE 'C'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE '8'		/* Set to a number we dont use */
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
  #define GUESTADMIN_MODE '7'		/* Set to a number we dont use */
  #define COSERVERADMIN_MODE '1'
  #define SERVERADMIN_MODE 'a'
  #define CONETADMIN_MODE '8'		/* Set to a number we dont use */
  #define NETADMIN_MODE '6'
  #define TECHADMIN_MODE '9'
  #define SERVICESADMIN_MODE '5'
  #define NETSERVICE_MODE '4'
  #define INVISIBLE_MODE '3'
  #define BOT_MODE '2'
#endif
