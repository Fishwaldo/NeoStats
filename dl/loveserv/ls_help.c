/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module:  LoveServ
** Description:  Network Love Service
** Version: 1.5
** Author: Shmad
*/

const char *ls_help[] = {
"\2LoveServ Help\2",
"",
"Commands:",
"     ROSE       KISS     TONSIL     HUG       ADMIRER",
"     CHOCOLATE  CANDY    LOVENOTE   APOLOGY",
"     THANKYOU   VERSION",
"",
"For Additional Help on each command type:",
"EXAMPLE: /msg LoveServ HELP ROSE",
"",
NULL
};

const char *ls_help_admin[] = {
"\2Additional Commands for Net and Tech Admins",
"",
"     VIEWLOGS",
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

const char *ls_help_kiss[] = {
"\2LoveServ Help: KISS",
"Send a kiss to that special someone on IRC.",
"\2SYNTAX:\2 /msg LoveServ KISS NICK",
"",
NULL
};

const char *ls_help_tonsil[] = {
"\2LoveServ Help: TONSIL",
"Send a deep tonsil penitrating kiss to someone on IRC.",
"\2SYNTAX:\2 /msg LoveServ TONSIL NICK",
"",
NULL
};	

const char *ls_help_hug[] = {
"\2LoveServ HELP: HUG",
"Send a hug to someone on IRC.",
"\2SYNTAX:\2 /msg LoveServ HUG NICK",
"",
NULL
};

const char *ls_help_admirer[] = {
"\2LoveServ HELP: ADMIRER",
"Tell someone on IRC they have a SECRET Admirer!",
"\2SYNTAX:\2 /msg LoveServ ADMIRER NICK",
"",
NULL
};

const char *ls_help_chocolate[] = {
"\2LoveServ HELP: CHOCOLATE",
"Send a big yummy box of candy to someone on IRC.",
"\2SYNTAX:\2 /msg LoveServ CHOCOLATE NICK",
"",
NULL
};

const char *ls_help_candy[] = {
"\2LoveServ HELP: CANDY",
"Send someone a box of yummy heart shaped candies",
"\2SYNTAX:\2 /msg LoveServ CANDY NICK",
"",
NULL
};

const char *ls_help_lovenote[] = {
"\2LoveServ HELP: LOVENOTE",
"Send that special someone a love note.",
"\2SYNTAX:\2 /msg LoveServ LOVENOTE NICK I love you dearly.",
"",
NULL
};

const char *ls_help_apology[] = {
"\2LoveServ HELP: APOLOGY",
"Send an Apology to someone",
"\2SYNTAX:\2 /msg LoveServ APOLOGY NICK deleting all those songs",
"",
NULL
};

const char *ls_help_thankyou[] = {
"\2LoveServ HELP: THANKYOU",
"Send a THANKYOU message to someone",
"\2SYNTAX:\2 /msg LoveServ THANKYOU NICK uploading those songs",
"",
NULL
};

const char *ls_help_version[] = {
"\2LoveServ HELP: VERSION",
"Show LoveServ's current version",
"",
NULL
};

const char *ls_help_viewlogs[] = {
"\2Net/Tech Admin Command: VIEWLOGS",
"View the LoveServ LOGFILE.  Please be forewarned",
"That this \2CAN\2 and \2MAY\2 flood you off the",
"Network.  Use at your own risk!",
"\2SYNTAX:\2 /msg LoveServ VIEWLOGS",
"",
NULL
};
