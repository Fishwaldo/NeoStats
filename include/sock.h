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

#ifndef _SOCK_H_
#define _SOCK_H_

#include "event.h"

int InitSocks( void );
void FiniSocks( void );
int ns_cmd_socklist( CmdParams* cmdparams );
int del_sockets( Module *mod_ptr );
void Connect( void );
int check_sql_sock( void );

Sock *add_buffered_socket(const char *sock_name, OS_SOCKET socknum, evbuffercb readcb, evbuffercb writecb, everrorcb errcb, void *arg);

extern char recbuf[BUFSIZE];

#endif /* _SOCK_H_ */
