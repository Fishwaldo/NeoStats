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
** $Id: serviceroots.c 1721 2004-04-09 22:17:19Z Mark $
*/

/** templateauth.c 
 *  You can copy this file as a template for writing your own auth modules
 */

/* neostats.h is the only required include for your module to access the 
 * NeoStats module API. You should not need to include any other NeoStats
 * files in order to develop your module.
 */
#include "neostats.h"

const char *template_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

ModuleInfo module_info = {
	"TemplateAuth",
	"Template Authentication Module",
	template_copyright,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
};

int ModInit(Module* modptr)
{
	return 1;
}

void ModFini()
{
}

int ModAuthUser(Client * u)
{
	return 0;
}

int ModAuthList(Client * u)
{
	return 1;
}
