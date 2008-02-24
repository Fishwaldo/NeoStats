/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2008 ^Enigma^
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
** $Id: neoserv_help.c,v 1.5 2003/05/26 09:18:30 fishwaldo Exp $
*/

const char *neoserv_help[] = {
"\2NeoServ - NeoStats Help System\2",
"",
"Avalible Topics:",
"\2TOPIC1\2 - I get some LDL error message when I try to compile NeoStats",
"\2TOPIC2\2 - I get a message about not being able to find my modules",
"\2TOPIC3\2 - How do I install a module that I downloaded from the NeoStats Site?",
"\2TOPIC4\2 - I get 'warning: initialization from incompatible pointer type' when I",
"         'make' NeoStats or 'make' a module",
"\2TOPIC5\2 - When I load the new version of StatServ it segfaults the stats server",
"\2TOPIC6\2 - I used NeoStats 1.x and now I have a module called 'sumyungguy' - what",
"          is 'sumyungguy'?",
"\2TOPIC7\2 - What IRCD or Services does the NeoStats Team reccomend?",
"\2TOPIC8\2 - What Operating System will NeoStats run on?",
"\2TOPIC9\2 - I upgraded my compiler and NeoStats has stopped working",
"\2TOPIC10\2 - Services & NeoStats won't 'play nice' with each other",
"\2TOPIC11\2 - Whats new in NeoStats, What's the main focus right now?",
"\2TOPIC12\2 - I've looked @ the website... I still have no idea what the modules do",
"\2TOPIC13\2 - NeoStats API - I want to have a module with multiple nicks. Is this possible?",
"\2TOPIC14\2 - I have an idea for a module... will you code it for me?",
"\2TOPIC15\2 - When I compile a module... NeoStats crashes!",
"\2TOPIC16\2 - NeoStats crashed... I've looked everywhere for 'netstats.debug'!",
"\2TOPIC17\2 - I get an 'Access Denied' message when I try to preform a command",
"\2TOPIC18\2 - I use HostServ... can I have multiple host masks for a user?",
"\2TOPIC19\2 - Why doesn't NeoStats 'RELOAD'?",
"\2TOPIC20\2 - Why does 'RESTART' take so long or not work at all?",
"\2TOPIC21\2 - My log file is flooded with 'Unable to find TLD entry' messages",
"\2TOPIC22\2 - Will there be a Windows version of NeoStats?",
"\2TOPIC23\2 - Can I use new modules on the old version of NeoStats?",
"\2TOPIC24\2 - I'm a Beta tester for 2.1",
"",
"\2***\2 Note: Modules & NeoStats should \2NOT\2 be installed as the '\2root\2' user. If",
"you have done this, \2reinstall\2 them as a \2normal user\2! Installing NeoStats as the",
"root user can cause security issues and problems with the working of NeoStats",
"",
"For Additional Help on each command type:",
"EXAMPLE: '/msg NeoServ HELP TOPIC1' or '/msg NeoServ TOPIC1'",
"",
NULL
};

const char *neoserv_help_topic1[] = {
"\2TOPIC1 - LDL Error",
"",
"If NeoStats won't compile and gives and 'LDL' error message then simply",
"remove the '-LDL' line from from the 'Makefile'",
"",
"(The line reads: 'LDFLAGS= -rdynamic -ldl -ldb')",
"",
"Recompile NeoStats and the problem should be fixed. People with compiler",
"questions should see 'TOPIC9' for more information",
"",
NULL
};

const char *neoserv_help_topic2[] = {
"\2TOPIC2 - Module Path Errors",
"",
"Check the path for your neostats.cfg file by going into the shell and into",
"the module directory. From there type the command 'pwd' and copy that path.",
"Enter the path you got from the 'pwd' command into the module path setting in",
"neostats.cfg.",
"",
"(The line reads: 'MODULE_PATH /home/path/to/module-directory')",
"",
"The most common error is people type 'Neostats' instead of 'NeoStats'.",
"",
NULL
};

const char *neoserv_help_topic3[] = {
"\2TOPIC3 - Module Installation",
"",
"Upload the module file into the \2module\2 directory",
"Type either:",
"a) 'tar xvfz Module-x.x.tar.gz'",
"or",
"b) 'gzip -d Module-x.x.tar.gz' Followed by 'tar xvf Module-x.x.tar'",
"Once This is done enter the directory the files were unpacked to",
"type 'make' (Ignore any 'errors' that appear)",
"In an IRC window type '/msg NeoStats load modulename' where 'modulename'",
"is the name of the directory that the module was unpacked to",
"",
"Note: Once the module has been 'made' you can make the module load when",
"NeoStats is loaded by editing the neostats.cfg file. Adding 'LOAD_MODULE modulename'",
"in the Module section will ensure this happens the next time NeoStats is loaded",
"",
NULL
};

const char *neoserv_help_topic4[] = {
"\2TOPIC4\2 - Pointer Type Error",
"",
"The solution to this problem is to simply ignore the message",
"the message does not affect the running of the module or NeoStats.",
"",
NULL
};

const char *neoserv_help_topic5[] = {
"\2TOPIC5\2 - StatServ Segfault",
"",
"This is a problem that the NeoStats Team is well aware of. We are",
"currently addressing this issue and hope to have a solution for you",
"soon.",
"",
NULL
};

const char *neoserv_help_topic6[] = {
"\2TOPIC6\2 - What is 'sumyungguy'?",
"",
"sumyungguy is the spam module. It no longer changes its name...",
"rather, it sits and waits for a user to spam it before it echos",
"the spammer's message, nick & hostname information.",
"",
NULL
};

const char *neoserv_help_topic7[] = {
"\2TOPIC7\2 - What IRCD and Services Does The Team Reccomend?",
"",
"Personally, we use the Unreal IRCD and Epona Services.",
"NeoStats will work on some other IRCDs and will get on well",
"with other services such as DaylightII. NeoStats-2.5 is",
"planned to support the Ultimate, Hybrid and Bahmut IRCDs.",
"",
NULL
};

const char *neoserv_help_topic8[] = {
"\2TOPIC8\2 - What OS Will NeoStats Run on?",
"",
"NeoStats runs best under any Linux/Unix Based Enivornment.",
"NeoStats-2.0.13 does not run under some BSDs, however a patch",
"for this was created and the NeoStats forums should be looked",
"at for more information on this. NeoStats-2.5 will support",
"BSDs.",
"",
"NeoStats will not run under the MAC or Windows environment",
"",
NULL
};

const char *neoserv_help_topic9[] = {
"\2TOPIC9\2 - I Have Upgraded My Compiler",
"",
"NeoStats will run perfectly when made under GCC versions",
"2.95.3 & 2.96. We know of an issue with the newer 3.0.2",
"compiler regarding the server names which causes a segfault",
"We are currently working on this issue and hope to have a",
"solution to this problem soon.",
"",
NULL
};

const char *neoserv_help_topic10[] = {
"\2TOPIC10\2 - Services & NeoStats Getting Along",
"",
"The first senario is if Services ops/deops NeoStats",
"and Neostats ops/deops itself in the services channel.",
"This problem stems from Services & NeoStats not having",
"registered nicknames. However, NeoStats does not need to",
"be op'ed in the Services channel to function properly.",
"",
"The second senario is a NeoStats/Services flood to a",
"much greater extent than a simple op/deop 'war'.",
"If your conflict is more serious than this occurance,",
"your problem is that you do not have U: lines in your",
"network IRCDS. Please add U: lines for the stats servers",
"in all of the network's IRCDS and this problem should be",
"solved.",
"",
NULL
};

const char *neoserv_help_topic11[] = {
"\2TOPIC11\2 - Whats New in NeoStats?",
"",
"The current development version of NeoStats is",
"NeoStats Version 2.5. The new version which is currently",
"in beta development fixes a number of issues that have",
"come to our attention such as statserv crashing NeoStats",
"when NeoStats is loaded. The new code has undergone some",
"major developments such as the complete re-coding of some",
"'questionable' routines in terms of stability. The team is",
"also proud to announce the releasing of a number of new",
"modules as well as updates to some of our older ones.",
"One of the noteable differences is a new core in the",
"NeoStats-2.5. Older modules will not work for the new",
"version of NeoStats.",
"",
"An improved StatServ is now avaliable which combines all",
"the requests that users have given us for feautres. We are",
"also proud to announce ConnectServ (which takes over some",
"of StatServ's features), HostServ as well as OperLog and",
"other modules such as LoveServ, NetInfo and MoraleServ.",
"The new options provide a few laughs such as LoveServ and",
"MoraleServ and greater accuracy and stability in terms of",
"Statserv.",
"",
"The Team's main target at the moment is completeing many",
"items on the 'TODO' list as well as 'overhauling' and",
"cleaning up large amounts of code. The second target",
"that we are working on at present is the stability",
"issues to make the NeoStats-2.5 release the most stable",
"release yet.",
"",
"Some things on our 'TODO' list. include trivial things such",
"as fixing RESTART and implementing RELOAD. Other items include",
"a new type of 'access' system similar to services 'operator' and",
"and 'roots' lists. We are busier than ever on version 2.5 and we",
"are working hard at it... as well as maintaing some sort of a life",
"away from our crackhead code :P j/k.",
"",
"We anticipate version 2.5 to be our most stable and best in",
"terms of features. We have no idea when it will be ready at",
"this stage, but if your on our mailing list we'll let you know",
":P",
"",
"The NeoStats Group Inc.",
"",
"Don't smoke crack. We really mean it this time :P",
"",
NULL
};

const char *neoserv_help_topic12[] = {
"\2TOPIC12\2 - What Does Each Module do Exactly?",
"",
"Ok... In a nutshell:",
"",
"LoveServ: Network Love Service... this module allows you to",
"send roses etc to your friends on IRC. fx:",
"'Shmad has sent you this beautiful rose! 3--<--<--<{4@'",
"",
"MoraleServ: Network Morale Service... this module provides",
"a few laughs. It's very similar to LoveServ and preforms",
"functions such as a 'lapdance' and 'odes' about a user to",
"be sent to another user.",
"",
"HostServ: Network Virtual Host Service.... allows a user's",
"host to be changed upon signon to the network. fx:",
"'not4u2know@ hide-24834.dsl.myisp.net' can become:",
"'not4u2know@ My.Own.Host.net' instantly when a user logs",
"onto the network.",
"",
"ConnectServ: Network Connection Service... this module",
"monitors signons/signoffs, mode settings such as +T, +B etc",
"and kills. These can be echoed to the channel or not echoed",
"at all... its entierly up to you! This module is very",
"customisable.",
"",
"StatServ: Network Statistics Service... this is a key",
"module and provides the part of NeoStats that monitors",
"the activity of the network and provides up-to-the-second",
"statistics. It is capable of many functions such as HTML",
"output.",
"",
"sumyungguy: Network Spam Service... this module is designed",
"to 'catch' spammers by taking all private messages and",
"broadcasting them so Operators can decide what to do from",
"there (/kill?)",
"",
"OperLog: Network Logging Service... this module logs",
"operator commands such as /kill etc so it can be determined",
"if any form of abuse etc is occuring",
"",
"NeoStats: This is the main file which *HAS* to be online.",
"this module is responsible for the loading/unloading of",
"modules as well as the correct shutdown etc of NeoStats",
"",
"NeoServ: NeoStats Help Service. This module is deisgned",
"for the NeoStats network to provide 24hr help to simple",
"and common questions.",
"",
NULL
};

const char *neoserv_help_topic13[] = {
"\2TOPIC13\2 - Multiple NeoStats Nicks & The API",
"",
"The short answer to this question is 'yes'. Some time",
"back development started on the NeoStats Channel Services",
"to replace the Epona services currently used and be fully",
"intergrated with NeoStats. During this design a module with",
"multiple nick's was deisgned and so was a patch to the API",
"which enabled this feature. At this stage there are no plans",
"to release the multiple nicks API, but rather intergate it",
"into the NeoStats-2.5 release.",
"",
NULL
};

const char *neoserv_help_topic14[] = {
"\2TOPIC14\2 - Module Ideas",
"",
"It really depends on what the module is for... but we will",
"take suggestions and comments and probably end up doing them :P",
"A good example of this is the HTML stats output in StatServ and",
"the design of HostServ.",
"",
NULL
};

const char *neoserv_help_topic15[] = {
"\2TOPIC15\2 - NeoStats Crashes During A Module Compile",
"",
"When you compile/recompile a module... it \2can't\2 be",
"loaded. It must first be unloaded (/msg NeoStats unload modulename)",
"Once the module is unloaded you can safley compile without",
"NeoStats crashing. If you are unsure if he module is loaded",
"you can find out by: /msg NeoStats modlist",
"",
NULL
};

const char *neoserv_help_topic16[] = {
"\2TOPIC16\2 - Where is 'netstats.debug'?",
"",
"This file does not exist. It is actually something which got changed",
"and the sentance structure was not modified before NeoStats was shipped.",
"Version 2.1 fixes this and the NeoStats core is dumped to a 'true'",
"Core file in the root of NeoStats.",
"",
"So Where did it actually get dumped to? Well, the rather 'primitive",
"dump' in previous versions was placed in the 'stats.log' file",
"which is located in the 'logs' directory. This file contains a great",
"deal of information and in version 2.1, a few things which are not",
"very relevant to you simply won't be printed in there which will save",
"on Hard Disk space.",
"",
"We reccomend you delete this file ('rm stats.log') once it's size is",
"over 2MB",
"",
NULL
};

const char *neoserv_help_topic17[] = {
"\2TOPIC17 - Access Denied",
"",
"To operate NeoStats you must have either Tech-Admin or Net-Admin",
"privileges. You may want to check that you have those flags",
"before you try the command.",
"",
"The old standard for NeoStats was 'Tech Admin' or higher...",
"this flag is fading away and in the newer versions of NeoStats,",
"the new standard is 'Net Admin' or higher.",
"",
"We have heard of a case where the flags were correct for the",
"irc-operator and the NeoStats were correct... but the network",
"had recently upgraded from Unreal-3.1.1 to Unreal3.2",
"",
"The development of NeoStats-2.5 will fix this problem as we are",
"now working on a new system independent of flags. The new system",
"is based around the idea of services 'operator' and 'root' lists.",
"We belive this will reduce the number of problems encountered",
"when changes are made to the various ircds.",
"",
NULL
};

const char *neoserv_help_topic18[] = {
"\2TOPIC18 - Multiple HostServ masks",
"",
"Yes, HostServ supports multiple hostnames for a singular",
"nick name. Simply 'ADD' another entry for the same nick",
"under the new host (if your unsure /msg HostServ HELP ADD).",
"HostServ also supports wildcard host masks. (See TOPIC12",
"for more help on this subject).",
"",
NULL
};

const char *neoserv_help_topic19[] = {
"\2TOPIC19 - 'RELOAD' Doesn't Work",
"",
"The reason for this is quite simple... there is no code",
"for it :P. We are implementing this into version 2.1",
"The reason for the lack of code in 2.0.13 is because it",
"was intended to be in there, however, we fell just short",
"of our target and in a hurry to release 2.0.13, it was",
"left out.",
"",
NULL
};

const char *neoserv_help_topic20[] = {
"\2TOPIC20 - RESTART Taking a Long Time or Not Working",
"",
"The NeoStats Group is well aware of this problem and",
"we are working on a solution for this in version 2.1",
"The problem lies in setting up the remote connection",
"and instead of connecting, the code is sent into a",
"virtual loop, enlarging the 'stats.log' file and",
"fustrating NeoStats users.",
"",
"We hope to have a solution for this soon.",
"",
NULL
};

const char *neoserv_help_topic21[] = {
"\2TOPIC21 - 'Unable to find TLD entry' Messages",
"",
"This Problem stems from the statserv.c file. The",
"log of it basically comes about due to the fact that",
"the last period is unable to be located ie '.net'.",
"The group has been pondering a total and complete",
"re-code of StatServ for some time to fix and address",
"issues like this. For now, simply ignore this message",
"and continue using NeoStats like you normally would.",
"The problem actually arises when the TLD is attempting",
"to be delted.",
"",
NULL
};

const char *neoserv_help_topic22[] = {
"\2TOPIC22 - Windows Version of NeoStats",
"",
"The NeoStats team has considered porting a windows platform",
"of NeoStats on many occasions. At this stage we feel that",
"we will create a windows version from the release that is",
"due out shortly (NeoStats-2.5). We are currently in the",
"process of fine-tuning and debugging the new and old code",
"of NeoStats. However, don't be surprised if no windows",
"version is available until a few releases later. Once we",
"have the remaining bugs ironed out, a windows version has a",
"60% chance of happening.",
"",
"When this version is released, Shmad, one of the coders",
"may port the software to win32 allowing it to be run under",
"a windows environment. We are unsure at this stage if we",
"would do this for every release thereafter or if it would",
"be simply the one version. If you have any specific",
"questions regarding this feel free to email",
"shmad@neostats.net as he will be the coder in charge of the",
"porting process.",
"",
"We will be posting more information about this particular",
"subject on our website shortly.",
"",
NULL
};

const char *neoserv_help_topic23[] = {
"\2TOPIC23 - New Modules on Older Versions of NeoStats",
"",
"Modules posted to the NeoStats website can be used on Version",
"2.0.13 and greater of NeoStats. The latest modules are intended",
"for use on the development version (2.1) but will work fine on",
"2.0.13 If you have an older version of NeoStats, it is reccomended",
"that you first upgrade to version 2.0.13 then install the new",
"modules which you have downloaded.",
"",
"However, recently the team decided to make a large number of",
"changes to NeoStats, causing the development version",
"NeoStats-2.1 to be incremented to NeoStats-2.5. In doing this,",
"code used for the modules was slightly modified and therefore",
"the older modules will not work on the new version of NeoStats.",
"A few of the modules on the web site at present are intended for",
"use with the older development version NeoStats-2.1, but may",
"work with NeoStats-2.0.13. If errors are incured with the new",
"versions from the website it is probably due to this fact.",
"",
"If you get an '-LDL' error, see 'TOPIC1' for help on this matter.",
"",
NULL
};

const char *neoserv_help_topic24[] = {
"\2TOPIC24 - NeoStats Beta Testers Note",
"",
"Welcome to the Beta preview of NeoStats-2.5, the next",
"generation of statistical software from the NeoStats Group Inc.",
"",
"Here's what you should expect when downloading this beta:",
"",
"NeoStats-2.5 is a beta which may contain bugs, we recommend",
"that you install NeoStats-2.5 in it's own folder rather than",
"overwrite the NeoStats 2.x installation that you currently",
"have. If you have a previous version of NeoStats-2.5 you",
"should delete it and do a fresh install of the next Beta release.",
"The software has several, major known issues, which prevent",
"certain features from being fully usable by NeoStats users.",
"Functionality will increase as we start moving towards the",
"final product Who should download this beta:",
" - Networks who don't mind occasional segmentation faults",
" - People interested in seeing the direction of NeoStats",
" - People who are reasonably technically minded",
"",
"We recommend you install this for evaluation purposes, in",
"order to have an understanding of what the future holds.",
"This beta will not be perfect, but it will give you an idea of",
"where we're going with the final release.",
"",
"Bugs should be reported to the NeoStats Bug Tracking Forum or",
"by email in a report fashion:",
"http://www.neostats.net/cgi-bin/Forum.cgi or support@neostats.net",
"",
"With that said, we hope you enjoy using NeoStats-2.5!",
"",
NULL
};
