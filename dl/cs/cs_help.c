/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module: ConnectServ
** Description: Network Connection & Mode Monitoring Service
** Version: 1.0
** Authors: ^Enigma^ & Shmad
*/

const char *cs_help[] = {
"\2ConnectServ HELP\2",
"",
"COMMANDS:",
"     SIGNWATCH     KILLWATCH     MODEWATCH",
"",
"     STATUS     ABOUT",
"",
NULL
};

const char *cs_help_about[] = {
"\2About ConnectServ\2",
"",
"ConnectServ is designed to echo the signing on/off of users,",
"killing of users and the modes that the operators are using.",
"These three echo types can be customised to be echoed to the",
"services channel or NOT to be echoed at all. Technical Admins",
"and Network Admins can turn these three settings on and off",
"",
NULL
};

const char *cs_help_status[] = {
"\2ConnectServ Help : STATUS\2",
"\2Usage:\2 STATUS",
"",
"This will tell you the current status of the settings. eg: MODEWATCH is enabled ot disabled",
"",
NULL
};

const char *cs_help_signwatch[] = {
"\2ConnectServ Help : SIGNWATCH",
"\2Usage:\2 SIGNWATCH",
"",
"Will enable or disable SIGNWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};

const char *cs_help_killwatch[] = {
"\2ConnectServ Help : KILLWATCH",
"\2Usage:\2 KILLWATCH",
"",
"Will enable or disable KILLWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};

const char *cs_help_modewatch[] = {
"\2ConnectServ Help : MODEWATCH",
"\2Usage:\2 MODEWATCH",
"",
"Will enable or disable MODEWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};
