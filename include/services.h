/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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
** $Id$
*/

#ifndef _SERVICES_H_
#define _SERVICES_H_

typedef struct neoroot {
	char nick[MAXNICK];
	char user[MAXUSER];
	char host[MAXHOST];
	int level;
} neoroot;

/* general configuration items */
typedef struct config {
	/* log level */
	LOG_LEVEL	loglevel;
	/* debug level */
	DEBUG_LEVEL debuglevel;
	/* dont load modules on startup */
	unsigned int modnoload:1;
	/* dont output anything on start */
	unsigned int quiet:1;
	/* dont detach into background */
	unsigned int foreground:1;
	unsigned int want_privmsg:1;
	unsigned int die:1;
	unsigned int error:1;
	unsigned int onlyopers:1;
	int setservertimes;
	int splittime;
	int msgsampletime;
	int msgthreshold;
	unsigned int versionscan;
	int r_time;
	char pass[MAXPASS];
	unsigned int debug;
	int debugtochan;
	char debugchan[MAXCHANLEN];
	int pingtime;
	int joinserviceschan;
	neoroot rootuser;
	char debugmodule[MAX_MOD_NAME];
	unsigned int singlebotmode;
	unsigned int cmdreport;
	unsigned int unauthreport;
	unsigned char cmdchar[2];
} config;

extern config nsconfig;

extern ModuleInfo ns_module_info;
extern Module ns_module;
extern BotInfo ns_botinfo;
EXPORTVAR extern Bot* ns_botptr;

void InitServices( void );
void FiniServices( void );
int init_services_bot( void );

#endif /* _SERVICES_H_ */
