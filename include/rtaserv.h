/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _RTASERV_H_
#define _RTASERV_H_

#include "rta.h"

extern TBLDEF neo_bans;
extern TBLDEF neo_chans;
extern TBLDEF neo_users;
extern TBLDEF neo_modules;
extern TBLDEF neo_servers;

void rta_hook_1 (fd_set *read_fd_set, fd_set *write_fd_set);
void rta_hook_2 (fd_set *read_fd_set, fd_set *write_fd_set);

void rtaserv_init (void);
void rtaserv_fini (void);
void rtaserv_init2 (void);

#endif /* _RTASERV_H_ */
