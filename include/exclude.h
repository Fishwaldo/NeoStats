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

#ifndef _EXCLUDE_H_
#define _EXCLUDE_H_

typedef struct Exclude {
	NS_EXCLUDE type;
	char pattern[USERHOSTLEN];
	char addedby[MAXNICK];
	char reason[MAXREASON];
	time_t addedon;
} Exclude;

int InitExcludes( void );
void FiniExcludes( void );
int InitModExcludes( Module *mod_ptr );
void FiniModExcludes( Module *mod_ptr );
bot_cmd *GetModExcludeCommands( Module *mod_ptr );

void ns_do_exclude_chan( Channel *c );
void ns_do_exclude_server( Client *s );
void ns_do_exclude_user( Client *u );
int ns_cmd_exclude( CmdParams* cmdparams );

extern bot_cmd mod_exclude_commands[];

#endif /* _EXCLUDE_H_ */
