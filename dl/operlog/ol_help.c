/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module: OperLog
** Description: Network Command Logging Service
** Version: 1.0
** Authors: ^Enigma^ & Shmad
*/

const char *ol_help[] = {
"\2OperLog HELP\2",
"",
"COMMANDS:",
"     CHATWATCH     KILLWATCH     MODEWATCH",
"",
"     STATUS     VIEWLOG     RESET     ABOUT",
"",
NULL
};

const char *ol_help_about[] = {
"\2About OperLog\2",
"",
"OperLog is designed to log commands issued by IRC-Operators.",
"The service can show un-baised use/abuse of certain commands",
"such as /kill, modes such as +I and chatops conversation.",
"The three log types can be customised to be logged or NOT to",
"be logged at all. Network Admins can turn these three settings",
"on and off at any time.",
"",
NULL
};

const char *ol_help_status[] = {
"\2OperLog Help : STATUS\2",
"\2Usage:\2 STATUS",
"",
"This will tell you the current status of the settings. eg: MODEWATCH is enabled ot disabled",
"",
NULL
};

const char *ol_help_viewlog[] = {
"\2OperLog Help : VIEWLOG\2",
"\2Usage:\2 VIEWLOG",
"",
"View the OperLog log file. Please be forewarned That this \2CAN\2 and \2MAY\2",
"flood you off the Network. Use this command at your own risk!",
"",
NULL
};

const char *ol_help_reset[] = {
"\2OperLog Help : RESET\2",
"\2Usage:\2 RESET",
"",
"This command resets the Operator.log file for a new logging period",
"",
NULL
};

const char *ol_help_chatwatch[] = {
"\2OperLog Help : CHATWATCH",
"\2Usage:\2 CHATWATCH",
"",
"Will enable or disable CHATWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command. CHATWATCH will log operator chat forms such as globops and",
"chatops.",
"",
NULL
};

const char *ol_help_killwatch[] = {
"\2OperLog Help : KILLWATCH",
"\2Usage:\2 KILLWATCH",
"",
"Will enable or disable KILLWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command. KILLWATCH will log global and local kills by operators",
"(including who issued the kill and what the kill message was.",
"",
NULL
};

const char *ol_help_modewatch[] = {
"\2OperLog Help : MODEWATCH",
"\2Usage:\2 MODEWATCH",
"",
"Will enable or disable MODEWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command. MODEWATCH logs certain modes being set by operators such",
"as the use of the +I (invisible) mode.",
"",
NULL
};
