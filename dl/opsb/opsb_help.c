/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module:  opsb
** Author: Fish
** $Id: opsb_help.c,v 1.1 2002/08/22 13:53:08 fishwaldo Exp $
*/

#include "stats.h"

const char *opsb_help[] = {
"\2Open Proxy Scanning Bot HELP\2",
"",
" This bot scans the network for insecure clients. For more info",
" \2/msg opsb info\2",
"",
"COMMANDS:",
"     LOOKUP    INFO",
"",
NULL
};

const char *opsb_help_oper[] = {
"OPERTATOR COMMANDS:",
"     CHECK    STATUS    SET",
"",
NULL
};

const char *opsb_help_lookup[] = {
"Usage: \2LOOKUP <ip or Hostname> <flag>\2",
"",
"This command allows you to lookup DNS records on the Internet",
"Different types of Records can be looked up by specifing different flags",
"",
"The Flags are:",
"    txt - Lookup Text Records",
"    rp  - Lookup the Responsible Person for this record",
"    ns  - Lookup the NameServers for this record",
"    soa - Lookup the SOA for this Record",
"",
"If you do not specify a flag, it defaults to looking up either the IP address for Hostnames, or", 
"The Hostname for IP addresses",
"",
NULL
};

const char *opsb_help_info[] = {
"\2Open Proxy Scanning Bot Information\2",
"",
"This bot is intended to scan clients connecting to this network for insecure proxies",
"Insecure proxies are often used to attack networks or channel with \2clone\2 bots",
"This check scans the following ports:", 
"    3128, 8080, 80 23 and 1080",
"if you have Firewall, or IDS software, please ignore any errors that this scan may generate",
"",
"If you have any futher questions, please contact network adminstration staff",
NULL
};

const char *opsb_help_check[] = {
"Usage: \2CHECK <nickname/IP/hostname>\2",
"",
"This option will scan either a user connected to your Network",
"Or a IP address or Hostname for Insecure proxies, and report the status to you",
"If a Insecure proxy is found, the host will be banned from the network",
"",
NULL
};

const char *opsb_help_status[] = {
"Usage: \2STATUS\2",
"",
"View Detailed information about the state of the Open Proxy Scanning Bot",
"",
NULL
};

const char *opsb_help_set[] = {
"Usage: \2SET <OPTIONS> <SETTING>\2",
"",
"This command will set various options relating to OPSB.",
"The Settings take effect imediatly",
"The Options are:",
"    \2TARGETIP\2      - Change the IP address we try to make the proxies connect to",
"                        This should be set to a IP address of on of your IRC Servers.",
"    \2TARGETPORT\2    - Change the Port number we try to make proxies connect to",
"                        This should be a port that runs on your IRCD",
"\2Advanced Settings\2 - These settings should not be changed unless you know the effects in full",
"    \2OPMDOMAIN\2     - Change the Domain we use to Lookup for Blacklists.",
"    \2MAXBYTES\2      - This is the maximum number of bytes we recieve from a proxy before disconnecting it",
"    \2TIMEOUT\2       - This is the ammount of time we wait for a proxy to respond to our servers before",
"                        Disconnecting, and assuming its not a open Proxy",
"    \2OPENSTRING\2    - This is the string we expect to see if there is a successfull Open Proxy",
"    \2SPLITTIME\2     - This is used to determine if users connecting to the network are part of a Netjoin",
"                        (when two servers link together)",
"    \2SCANMSG\2       - This is the message sent to a user when we scan their hosts",
"    \2BANTIME\2       - This is how long the user will be banned from the network for",
"",
NULL
};
