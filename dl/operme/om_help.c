/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module:  OperMe
** Version: 1.6
** Authors: Shmad & ^Enigma^
*/

#include "stats.h"

const char *om_help[] = {
"\2OperMe HELP\2",
"",
"COMMANDS:",
"     ABOUT     LOGIN     ADD     DEL     LIST",
"",
"Only Server Admins (and higher) can use the ADD & DEL functions.",
"The LIST function is available to any IRCOp (+o).",
"** All passwords in LIST are encrypted so no other opers can use them",
"   Network Admins and higher see the un-encrypted passwords.",
"",
NULL
};

const char *om_help_about[] = {
"\2About OperMe\2",
"",
"OperMe is designed to allow IRC operators to connect to other",
"servers and 'OPER' up on that server.  This is useful for",
"the routing committee and ircops when their home servers",
"are down due to outage of service or Denial of Service attacks.",
"",
"If you find your OperMe! o-line is not working, it could have",
"been removed due to abuse, or the fact you are connecting from",
"a different Provider or have a numerical address. Contact an admin",
"",
NULL
};

const char *om_help_login[] = {
"\2OperMe Help : LOGIN\2",
"\2Usage:\2 LOGIN <PASSWORD>",
"",
"Login to the OperMe! service.",
"",
NULL
};

const char *om_help_add[] = {
"\2OperMe Help : ADD\2",
"\2Usage:\2 ADD <NICK> <HOST NAME> <PASSWORD> <FLAGS>",
"",
"Add an IRC operator the permission to OPER up using the OperMe! service",
"",
"FIELDS:",
"NICK: Self Explainitory",
"HOSTNAME: the host the operator connects from wildcards are allowed. DO NOT INCLUDE AN @",
"PASSWORD: the operators password",
"FLAGS: the operators O-Line flags.  Wildcard * is NOT allowed.",
"",
NULL
};

const char *om_help_del[] = {
"\2OperMe Help : DEL\2",
"\2Usage:\2 DEL <NICK> <HOST NAME>",
"",
"The information needed for this is best copied out directly from the 'LIST' command",
"",
NULL
};

const char *om_help_list[] = {
"\2OperMe Help : LIST",
"\2Usage:\2 LIST",
"",
"Lists the allowed IRC operators that can use this service from the database.",
"An ecrypted password is displayed to increase security.  All Network Admins",
"will see the UN-ENCRYPTED password for troubleshooting and support inquires.",
"",
NULL
};
