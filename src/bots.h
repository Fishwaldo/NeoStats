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

#ifndef _BOTS_H_
#define _BOTS_H_

int InitBots(void);
int FiniBots(void);
Bot* add_mod_user (Module* modptr, char *nick);
Bot* add_neostats_mod_user (char *nick);
int list_bots (User * u, char **av, int ac);
int del_mod_user (char *nick);
void add_chan_bot (char *bot, char *chan);
void del_chan_bot (char *bot, char *chan);
int list_bot_chans (User * u, char **av, int ac);
void bot_message (char *origin, char **av, int ac);
void bot_notice (char *origin, char **av, int ac);

#endif /* _BOTS_H_ */
