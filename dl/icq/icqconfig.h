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
** $Id: icqconfig.h,v 1.2 2002/09/04 08:40:28 fishwaldo Exp $
*/



/* LOGIN_DELAY: seconds till login process times out
   KEEP_ALIVE_DELAY: how often icqtech should send the keepalive message
      to the ICQ server
   SRV_TO_USE: ICQ server to connect to
   SRV_PORT: Port of ICQ server

   MAX_MSG_LENGTH: Maximum length of messages (should not change)
   MSG_TIMEOUT: Time until user is erased from queue */
   
#define LOGIN_DELAY      30  /* seconds */
#define KEEP_ALIVE_DELAY 120 /* seconds */
#define MAX_MSG_LENGTH 450
#define MSG_TIMEOUT 120 /* in seconds */


/* icq_help.c */
const char *icq_help[];