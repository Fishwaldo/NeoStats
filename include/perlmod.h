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
** $Id: ctcp.h 2216 2005-01-24 22:21:33Z Mark $
*/

#ifndef _PERLMOD_H_
#define _PERLMOD_H_
#ifdef USE_PERL
#include "config.h"

#ifdef PERLDEFINES
#include <EXTERN.h>
#define WIN32IOP_H
#include <perl.h>
#include <XSUB.h>


typedef struct PerlEvent {
	SV *callback;
	SV *userdata;
	int options;
} PerlEvent;

typedef struct PerlModInfo {
	char filename[MAXPATH];
	PerlInterpreter *my_perl;
} PerlModInfo;
#endif

#define MOD_PERLEXT	".pl"


int Init_Perl();
void PerlModFini(Module *);
void unload_perlmod(Module *);
void ns_cmd_modperlist(CmdParams *cmd);
Module *load_perlmodule(const char *, Client *);
#endif
#endif /* _PERLMOD_H_ */
