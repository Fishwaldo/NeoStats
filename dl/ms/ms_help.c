/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module:  MoraleServ
** Description:  Network Morale Service
** Version: 2.1
** Author: ^Enigma^
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
"Commands for \2Technical Admins - You\2:",
"     JOIN     PART     LOVESERVLOGS",
"",
"     VIEWLOG     LOGBACKUP     RESET",
"",
"     SWHOIS     SVSNICK     SVSJOIN",
"",
"     SVSPART     KICK",
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

const char *ms_help_join[] = {
"\2MoraleServ Help : JOIN",
"Usage: \2JOIN <CHANNEL>\2",
"",
"Have me Join a Channel",
"",
NULL
};

const char *ms_help_viewlog[] = {
"\2MoraleServ Help : VIEWLOG",
"Usage: \2VIEWLOG\2",
"",
"Sends today's logfile via PRIVMSG/NOTICE",
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

const char *ms_help_swhois[] = {
"\2MoraleServ Help : SWHOIS",
"Usage: \2SWHOIS <NICK> <TEXT>\2",
"",
"Will change the swhois information about the person if they are online",
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

const char *ms_help_svsnick[] = {
"\2MoraleServ Help : SVSNICK",
"Usage: \2SVSNICK <OLD NICK> <NEW NICK>\2",
"",
"Will change the nick of the person if they are online",
"",
NULL
};

const char *ms_help_part[] = {
"\2MoraleServ Help : PART",
"Usage: \2PART <Channel>\2",
"",
"Have me Part a Channel",
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

const char *ms_help_loveservlogs[] = {
"\2MoraleServ Help : LOVESERVLOGS",
"Usage: \2LOVESERVLOGS\2",
"",
"Sends today's Loveserv logfile via PRIVMSG/NOTICE",
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

const char *ms_help_svsjoin[] = {
"\2MoraleServ Help : SVSNICK",
"Usage: \2SVSJOIN <NICK> <CHANNEL>\2",
"",
"Will join the target nick to the specified channel",
"",
NULL
};

const char *ms_help_svspart[] = {
"\2MoraleServ Help : SVSNICK",
"Usage: \2SVSPART <NICK> <CHANNEL>\2",
"",
"Will part the target nick to the specified channel",
"",
NULL
};

const char *ms_help_kick[] = {
"\2MoraleServ Help : KICK",
"Usage: \2KICK <CHANNEL> <NICK>\2",
"",
"Have the target nick kicked from a channel by me",
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

const char *ms_help_reset[] = {
"\2MoraleServ Help : RESET",
"Usage: \2RESET\2",
"",
"Resets MoraleServ - 4DO NOT USE unless you are sure.",
"",
NULL
};

const char *ms_help_logbackup[] = {
"\2MoraleServ Help : LOGBACKUP",
"Usage: \2LOGBACKUP\2",
"",
"Creates a backup of MoraleServ's logs",
"",
NULL
};

const char *ms_help_printfile[] = {
"\2MoraleServ Help : PRINTFILE",
"Usage: \2PRINTFILE <PATH TO FILE>\2",
"",
"Will send the contents of the specified file via PRIVMSG/NOTICE",
"",
NULL
};

const char *ms_help_unlinkfile[] = {
"\2MoraleServ Help : UNLINKFILE",
"Usage: \2UNLINKFILE <PATH TO FILE>\2",
"",
"Will delete the specified file from the server",
"",
NULL
};
