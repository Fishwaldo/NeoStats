/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module:  HostServ
** Version: 1.6
** Authors: Shmad & ^Enigma^
*/

#include "stats.h"

const char *hs_help[] = {
"\2HostServ HELP\2",
"",
"COMMANDS:",
"     ABOUT     ADD     DEL     LIST",
"",
"Only Network Admins can use the ADD, DEL & LIST functions",
"",
NULL
};

const char *hs_help_about[] = {
"\2About HostServ\2",
"",
"HostServ is designed to let users use their own unique host",
"While on the Network. IRC Operators add them to the database",
"And upon connection the user gets their unique host.",
"",
"If you find your host is not working, it could have been removed",
"Due to abuse, or the fact you are connecting from a different",
"Internet Provider or have a numerical address. Contact an admin",
"",
NULL
};

const char *hs_help_add[] = {
"\2HostServ Help : ADD\2",
"\2Usage:\2 ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>",
"",
"Register a host name to be set. eg: my-host.com 4DO NOT INCLUDE AN @",
"The <HOST NAME> must be where the user is connecting from 4WITHOUT THE @",
"HostServ supports wildcards such as *.myhost.com in the <HOST NAME> setting", 
"",
NULL
};

const char *hs_help_del[] = {
"\2HostServ Help : DEL\2",
"\2Usage:\2 DEL <NICK> <HOST NAME> <VIRTUAL HOST NAME>",
"",
"The information needed for this is best copied out directly from the 'LIST' command",
"",
NULL
};

const char *hs_help_list[] = {
"\2HostServ Help : LIST",
"\2Usage:\2 LIST",
"",
"Lists the people and hosts in the Database in db format",
"",
NULL
};
