/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"

const char *cs_about[] = {
	"\2ConnectServ\2 is designed to track users signing on and",
	"off, killing of users, modes that the operators are using",
	"and nickname changes. These events can be optionally",
	"echoed to the services channel.",
	NULL
};

const char *cs_help_set_signwatch[] = {
	"SIGNWATCH <ON/OFF> whether to echo signon/signoff",
	"events to the services channel.",
	NULL
};

const char *cs_help_set_exclusions[] = {
	"USEEXCLUSIONS <ON/OFF> whether to use the NeoStats",
	"exclusions before displaying events",
	NULL
};

const char *cs_help_set_logging[] = {
	"LOGGING <ON/OFF> whether to log the events",
	"reported by connectserv",
	NULL
};

const char *cs_help_set_killwatch[] = {
	"KILLWATCH <ON/OFF> whether to echo kill",
	"events to the services channel.",
	NULL
};
const char *cs_help_set_modewatch[] = {
	"MODEWATCH <ON/OFF> whether to echo operator modes",
	"events to the services channel.",
	NULL
};
const char *cs_help_set_nickwatch[] = {
	"NICKWATCH <ON/OFF> whether to echo nick changes",
	"events to the services channel.",
	NULL
};
const char *cs_help_set_servwatch[] = {
	"SERVWATCH <ON/OFF> whether to echo server",
	"joins and quits to the services channel.",
	NULL
};
