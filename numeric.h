/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
** $Id$
*/

#ifndef _NUMERIC_H_
#define _NUMERIC_H_

/* Numeric used by NeoStats. Need to be in ircd code eventually
 *
 */

#define RPL_STATSLINKINFO    211
#define RPL_STATSCOMMANDS    212
#define RPL_STATSCLINE       213
#define RPL_STATSOLDNLINE    214
#define RPL_STATSNLINE		RPL_STATSOLDNLINE

#define RPL_ENDOFSTATS       219

#define	RPL_STATSLLINE       241
#define	RPL_STATSUPTIME      242
#define	RPL_STATSOLINE       243

#define	RPL_ADMINME          256
#define	RPL_ADMINLOC1        257
#define	RPL_ADMINLOC2        258

#define RPL_VERSION          351

#define	RPL_MOTD             372

#define	RPL_MOTDSTART        375
#define	RPL_ENDOFMOTD        376

#define	ERR_NOMOTD           422
#define	ERR_NOADMININFO      423

#endif /* _NUMERIC_H_ */
