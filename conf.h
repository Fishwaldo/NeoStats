/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
** $Id: conf.h,v 1.3 2003/04/18 06:41:34 fishwaldo Exp $
*/


#ifndef _conf_h_
#define _conf_h_

/*
 * conf.h
 * dynamic configuration runtime libary
 */

/* define the config types */

#define CFGSTR   1
#define CFGINT   2
#define CFGFLOAT 3
#define CFGBOOL  4


/* general configuration items */
struct config {
	/* debug level */
	unsigned int debug;
	/* enable recv.log */
	unsigned int recvlog : 1;
	/* dont load modules on startup */
	unsigned int modnoload : 1;
	/* dont output anything on start */
	unsigned int quiet : 1;
	/* dont detach into background */
	unsigned int foreground : 1;
} config;

#define GetDir(x, y) kp_get_dir(x,y,NULL);

int GetConf(void **data, int type, const char *item);
int SetConf(void *data, int type, char *item);

/* need this to stop modules complaining */
int   kp_get_dir     (const char *keypath, char ***keysp, unsigned int *nump);


#endif
