/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

const char *ls_help[] = {
	"The following commands can be used with LoveServ",
	"",
	"    ABOUT       About LoveServ",
	"    ROSE        Give someone a rose",
	"    KISS        Give someone a kiss",
	"    TONSIL      Give someone a tonsil kiss",
	"    HUG         Give someone a hug",
	"    ADMIRER     Tell someone they have a secret admirer",
	"    CHOCOLATE   Give someone chocolate",
	"    CANDY       Give someone candy",
	"    LOVENOTE    Give someone a lovenote",
	"    APOLOGY     Give someone an apology",
	"    THANKYOU    Give someone a thank you",
	"    VERSION     Display version info",
	NULL
};

const char *ls_help_on_help[] = {
	"",
	"To use a command, type",
	"    \2/msg LoveServ command\2",
	"For for more information on a command, type", 
	"    \2/msg LoveServ HELP command\2.",
	NULL
};

const char *ls_help_about[] = {
	"\2LoveServ\2 is a fun module to send presents and ",
	"messages to loved ones on IRC.",
	NULL
};

const char *ls_help_rose[] = {
	"Syntax: \2ROSE <NICK>\2",
	"",
	"Send a rose to a loved one on IRC.",
	NULL
};

const char *ls_help_kiss[] = {
	"Syntax: \2KISS <NICK>\2",
	"",
	"Send a kiss to that special someone on IRC.",
	NULL
};

const char *ls_help_tonsil[] = {
	"Syntax: \2TONSIL <NICK>\2",
	"",
	"Send a deep tonsil penitrating kiss to someone on IRC.",
	NULL
};

const char *ls_help_hug[] = {
	"Syntax: \2HUG <NICK>\2",
	"",
	"Send a hug to someone on IRC.",
	NULL
};

const char *ls_help_admirer[] = {
	"Syntax: \2ADMIRER <NICK>\2",
	"",
	"Tell someone on IRC they have a SECRET Admirer!",
	NULL
};

const char *ls_help_chocolate[] = {
	"Syntax: \2CHOCOLATE <NICK>\2",
	"",
	"Send a big yummy box of candy to someone on IRC.",
	NULL
};

const char *ls_help_candy[] = {
	"Syntax: \2CANDY <NICK>\2",
	"",
	"Send someone a box of yummy heart shaped candies",
	NULL
};

const char *ls_help_lovenote[] = {
	"Syntax: \2LOVENOTE <NICK> I love you dearly.\2",
	"",
	"Send that special someone a love note.",
	NULL
};

const char *ls_help_apology[] = {
	"Syntax: \2APOLOGY <NICK> deleting all those songs\2",
	"",
	"Send an Apology to someone",
	NULL
};

const char *ls_help_thankyou[] = {
	"Syntax: \2THANKYOU <NICK> uploading those songs\2",
	"",
	"Send a THANKYOU message to someone",
	NULL
};

const char *ls_help_version[] = {
	"Syntax: \2VERSION\2",
	"",
	"Show LoveServ's current version",
	NULL
};
