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
** $Id: ms_help.c,v 1.4 2003/01/04 04:56:03 fishwaldo Exp $
*/

const char *ms_help[] = {
"\2MoraleServ Help\2",
"",
"Commands:",
"     HAIL     ODE     POEM     LAPDANCE     REDNECK",
"",
"     CHEERUP      BEHAPPY      WONDERFUL      MSG",
"",
"     VERSION",
"",
"For Additional Help on each command type:",
"EXAMPLE: /msg MoraleServ HELP HAIL",
"",
NULL
};

const char *ms_help_tech[] = {
"\2MoraleServ Help\2",
"",
"Commands:",
"     HAIL     ODE     POEM     LAPDANCE     REDNECK",
"",
"     CHEERUP      BEHAPPY      WONDERFUL      MSG",
"",
"     VERSION",
"",
"For Additional Help on each command type:",
"EXAMPLE: /msg MoraleServ HELP HAIL",
"",
NULL
};

const char *ms_help_hail[] = {
"\2MoraleServ Help : HAIL",
"\2Usage:\2 HAIL <WHO TO HAIL> <NICK TO SEND HAIL TO>",
"",
"Send a \"HAIL\" song greeting to a loved one on IRC.",
"",
NULL
};

const char *ms_help_ode[] = {
"\2MoraleServ Help : ODE",
"\2Usage:\2 ODE <WHO THE ODE ODE IS ABOUT> <NICK TO SEND ODE TO>",
"",
"Send an ode about a user to a loved one on IRC.",
"",
NULL
};

const char *ms_help_lapdance[] = {
"\2MoraleServ Help : LAPDANCE",
"\2Usage:\2 LAPDANCE <NICK>",
"",
"Send a lapdance to a loved one on IRC... sure to cheer any person up",
"",
NULL
};

const char *ms_help_version[] = {
"\2MoraleServ Help : VERSION",
"Usage: \2VERISON\2",
"",
"Prints today's current version information via PRIVMSG/NOTICE",
"",
NULL
};

const char *ms_help_poem[] = {
"\2MoraleServ Help : POEM",
"Usage: \2POEM <WHO THE POEM IS ABOUT> <NICK TO SEND TO>",
"",
"Send a poem about a user to a loved one on IRC.",
"",
NULL
};

const char *ms_help_redneck[] = {
"\2MoraleServ Help : REDNECK",
"Usage: \2REDNECK <NICK>",
"",
"Send a redneck dubbing to a loved one on IRC.",
"",
NULL
};

const char *ms_help_msg[] = {
"\2MoraleServ Help : MSG",
"Usage: \2MSG <PERSON OR CHAN TO MESSAGE> <MESSAGE>\2",
"",
"Have me say something to a Channel/User",
"",
NULL
};

const char *ms_help_cheerup[] = {
"\2MoraleServ Help : CHEERUP",
"Usage: \2CHEERUP <NICK>\2",
"",
"Send a \"cheerup\" message to a loved one on IRC to cheer them up.",
"",
NULL
};

const char *ms_help_behappy[] = {
"\2MoraleServ Help : BEHAPPY",
"Usage: \2BEHAPPY <NICK>\2",
"",
"Send a \"behappy\" song to a loved one on IRC to cheer them up.",
"This function sends a number of messages that may ping the target nick",
"out if they do not have a fast connection. 4DO NOT USE unless you are sure.",
"",
NULL
};

const char *ms_help_wonderful[] = {
"\2MoraleServ Help : WONDERFUL",
"Usage: \2WONDERFUL <NICK>\2",
"",
"Send a \"I hope you don't mind, that I put down in words...\" song to a loved one on IRC to cheer them up.",
"This function will send the lyrics with the target nick in them to the target nick",
"",
NULL
};

