/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module:  LoveServ
** Version: 1.0
*/

const char *ls_help[] = {
"\2LoveServ Help\2",
"",
"Commands:",
"     ROSE       KISS     TONSIL     HUG     ADMIRER",
"     CHOCOLATE  CANDY    LOVENOTE",
"",
"For Additional Help on each command type:",
"EXAMPLE: /msg LoveServ HELP ROSE",
"",
NULL
};

const char *ls_help_rose[] = {
"\2LoveServ Help : ROSE",
"",
"Send a rose to a loved one on IRC.",
"\2SYNTAX:\2 /msg LoveServ ROSE NICK",
"",
NULL
};

const char *ls_help_seto[] = {
"*** Administrator Commands:",
"\2MAP <ADD/DEL> <TLD> <CHANNEL> <MESSAGE>\2:",
"       Allows you to add or Delete AutoJoin TLD to Channel Mapping",
"       <TLD>           :either a IP, or Hostmask. e.g: *.au or 203.202.181.*",
"       <CHANNEL>       :The Channel to Join the User to",
"       <MESSAGE>       :The Message to Send to the User when they are AutoJoined",
NULL,
};

const char *ls_help_show[] = {
"*** Netinfo \2SHOW\2 Help ***",
"\2Show\2",
"       Shows your Current Settings,",
NULL
};	
const char *ls_help_showo[] = {
"*** Administrator Options:",
"\2Show map\2",
"       Shows you the Current TLD to Channel Mapping",
"\2Show ignore\2",
"       Shows you the Current Ignore List",
NULL
};


