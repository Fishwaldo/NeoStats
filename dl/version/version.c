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
** $Id$
*/
#include <stdio.h>
#include "dl.h"
#include "stats.h"

const char v1version_date[] = __DATE__;
const char v1version_time[] = __TIME__;


Module_Info my_info[] = { {
	"version",
	"changed command hack test for ircd 0.1-beta2",
	"0.1"
} };

int new_m_version(char *origin, char **av, int ac) {
	snumeric_cmd(351, origin, "Module Version Loaded, v%s %s %s",my_info[0].module_version,v1version_date,v1version_time);
	return 0;
}


Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

EventFnList my_event_list[] = {
	{ NULL, 	NULL}
};

EventFnList *__module_get_events() {
	return my_event_list;
};

Module_Info *__module_get_info() {
	return my_info;
}

Functions *__module_get_functions() {
	return my_fn_list;
};

void _init() {
}
void _fini() {
}
