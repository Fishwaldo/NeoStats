/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _SERVER_H_
#define _SERVER_H_

Server *AddServer (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline);
void DelServer(const char *name, const char* reason);
void ServerDump (const char *name);
int InitServers (void);
void PingServers (void);
void FiniServers (void);
Server *findserverbase64 (const char *num);
void RequestServerUptimes (void);

#endif /* _SERVER_H_ */
