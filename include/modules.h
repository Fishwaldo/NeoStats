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

#ifndef _MODULES_H_
#define _MODULES_H_

extern jmp_buf sigvbuf;

int InitModules (void);
void FiniModules (void);
Module *load_module (const char *path, Client * u);
int unload_module (const char *module_name, Client * u);
void unload_modules(void);
int ns_cmd_modlist (CmdParams* cmdparams);
void ModulesVersion (const char* nick, const char *remoteserver);
void SendAllModuleEvent (Event event, CmdParams* cmdparams);
void SendModuleEvent (Event event, CmdParams* cmdparams, Module* module_ptr);
int SynchModule (Module* module_ptr);
int SynchAllModules (void);

#endif /* _MODULES_H_ */
