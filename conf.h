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
** $Id: conf.h,v 1.1 2003/04/09 14:29:49 fishwaldo Exp $
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


int GetConf(void **data, int type, const char *item);
int SetConf(void *data, int type, char *item);



#endif
