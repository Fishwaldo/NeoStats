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

#ifndef _VERSION_H_
#define _VERSION_H_

typedef struct ctcpversionstat {
	char name[BUFSIZE];
	statistic users;
}ctcpversionstat;

extern list_t *versionstatlist;

int topcurrentversions(const void *key1, const void *key2);
int ss_cmd_userversion(CmdParams *cmdparams);
int ss_event_ctcpversion(CmdParams *cmdparams);
void InitVersionStats (void);
void FiniVersionStats (void);

#endif /* _VERSION_H_ */
