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

#include "neostats.h"

const char ms_help_hail_oneline[]="Hail someone on IRC";
const char ms_help_ode_oneline[]="Send an ODE to someone on IRC";
const char ms_help_poem_oneline[]="Send a poem to someone on IRC";
const char ms_help_lapdance_oneline[]="Give someone on IRC a lapdance";
const char ms_help_redneck_oneline[]="Send a Redneck message.";
const char ms_help_cheerup_oneline[]="Send an cheer up message to someone on IRC";
const char ms_help_behappy_oneline[]="Send an be happy song to someone on IRC";
const char ms_help_wonderful_oneline[]="Send an song to someone on IRC";

const char *ms_about[] = {
	"\2MoraleServ\2 is a fun module to send cheery",
	"messages to friends on IRC.",
	NULL
};

const char *ms_help_hail[] = {
	"Syntax: \2HAIL <WHO TO HAIL> <NICK TO SEND HAIL TO>\2",
	"",
	"Send a \"hail\" song greeting to a loved one on IRC.",
	NULL
};

const char *ms_help_ode[] = {
	"Syntax: \2ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>\2",
	"",
	"Send an \"ode\" about a user to a loved one on IRC.",
	NULL
};

const char *ms_help_lapdance[] = {
	"Syntax: \2LAPDANCE <NICK>\2",
	"",
	"Send a \"lapdance\" to a loved one on IRC, sure to cheer any person up",
	NULL
};

const char *ms_help_poem[] = {
	"Syntax: \2POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>\2",
	"",
	"Send a \"poem\" about a user to a loved one on IRC.",
	NULL
};

const char *ms_help_redneck[] = {
	"Syntax: \2REDNECK <NICK>\2",
	"",
	"Send a \"redneck\" dubbing to a loved one on IRC.",
	NULL
};


const char *ms_help_cheerup[] = {
	"Syntax: \2CHEERUP <NICK>\2",
	"",
	"Send a \"cheerup\" message to a loved one on IRC to cheer them up.",
	NULL
};

const char *ms_help_behappy[] = {
	"Syntax: \2BEHAPPY <NICK>\2",
	"",
	"Send a \"behappy\" song to a loved one on IRC to cheer them up.",
	"This function sends a number of messages that may ping the target",
	"nick out if they do not have a fast connection.",
	"DO NOT USE unless you are sure.",
	NULL
};

const char *ms_help_wonderful[] = {
	"Syntax: \2WONDERFUL <NICK>\2",
	"",
	"Send a \"I hope you don't mind, that I put down in words...\" song",
	"to a loved one on IRC to cheer them up. This function will send the",
	"lyrics with the target nick in them to the target nick",
	NULL
};
