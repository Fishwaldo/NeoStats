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

void ChanDump (CmdParams* cmdparams, const char *chan);
void part_chan (Client * u, const char *chan, const char* reason);
void PartAllChannels (Client * u, const char* reason);
void join_chan (const char* nick, const char *chan);
void ChanNickChange (Channel * c, const char *newnick, const char *oldnick);
void ChanTopic (const char* chan, const char *owner, const char* ts, const char *topic);
void kick_chan (const char *kickby, const char *chan, const char *kicked, const char *kickreason);
void SetChanTS (Channel * c, const time_t tstime);
int InitChannels (void);
void FiniChannels (void);

#endif /* _CHANS_H_ */
