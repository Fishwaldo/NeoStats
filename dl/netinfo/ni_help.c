/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      ns_help.c, 
** Version: 1.1.1.1
** Date:    03/02/2000
*/

const char *ni_help[] = {
"*** Netinfo Help ***",
"Commands:",
"\2Set\2       Set Specific Options up",
"\2Show\2      Show Specific Settings",
"\2Version\2   Shows you the current NetInfo Version.",
"End of Help.",
NULL
};

const char *ni_help_set[] = {
"*** Netinfo \2SET\2 Help ***",
"Commands:",
"\2AUTOJOIN <ON/OFF>\2:",
"       Toggles the Setting of your AutoJoin.",
"       The Administrator may setup Default Channels when you join",
"       Turning this off means that you will not be autojoined when you connect",
"       to the Network",
NULL
};
const char *ni_help_seto[] = {
"*** Administrator Commands:",
"\2MAP <ADD/DEL> <TLD> <CHANNEL> <MESSAGE>\2:",
"       Allows you to add or Delete AutoJoin TLD to Channel Mapping",
"       <TLD>           :either a IP, or Hostmask. e.g: *.au or 203.202.181.*",
"       <CHANNEL>       :The Channel to Join the User to",
"       <MESSAGE>       :The Message to Send to the User when they are AutoJoined",
NULL,
};

const char *ni_help_show[] = {
"*** Netinfo \2SHOW\2 Help ***",
"\2Show\2",
"       Shows your Current Settings,",
NULL
};	
const char *ni_help_showo[] = {
"*** Administrator Options:",
"\2Show map\2",
"       Shows you the Current TLD to Channel Mapping",
"\2Show ignore\2",
"       Shows you the Current Ignore List",
NULL
};


