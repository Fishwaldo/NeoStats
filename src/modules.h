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

int InitModules (void);
int FiniModules (void);
void SendModuleEvent (char * event, char **av, int ac);
int load_module (char *path, User * u);
int unload_module (char *module_name, User * u);
int list_modules (User * u, char **av, int ac);
int get_dl_handle (char *mod_name);
int get_mod_num (char *mod_name);
Module *get_mod_ptr (char *mod_name);
void unload_modules(void);

void ModulesVersion (const char* nick, const char *remoteserver);

#endif /* _MODULES_H_ */
