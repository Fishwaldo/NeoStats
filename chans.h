/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _CHANS_H_
#define _CHANS_H_

void ChanDump (char *chan);
void part_chan (User * u, char *chan);
void join_chan (User * u, char *chan);
void change_user_nick (Chans * c, char *newnick, char *oldnick);
int ChanMode (char *origin, char **av, int ac);
void ChangeTopic (char *, Chans *, time_t t, char *);
void ChangeChanUserMode (Chans * c, User * u, int add, long mode);
void kick_chan (char *chan, char *kicked, char *kickby);
void ChangeChanTS (Chans * c, time_t tstime);
int init_chan_hash (void);

#endif /* _CHANS_H_ */
