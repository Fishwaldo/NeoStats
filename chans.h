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

#ifndef _CHANS_H_
#define _CHANS_H_

void ChanDump (const char *chan);
void part_chan (User * u, const char *chan, char *);
void join_chan (const char* nick, const char *chan);
void change_user_nick (Chans * c, const char *newnick, const char *oldnick);
int ChanMode (char *origin, char **av, int ac);
void ChanTopic (const char *owner, const char* chan, time_t time, const char *topic);
void ChangeChanUserMode (const char* chan, const char* nick, int add, long mode);
void kick_chan (const char *chan, const char *kicked, const char *kickby, char *);
void ChangeChanTS (Chans * c, time_t tstime);
int init_chan_hash (void);

#endif /* _CHANS_H_ */
