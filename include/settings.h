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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

int add_bot_setting_list( Bot *bot_ptr, bot_setting *bot_setting_list );
int del_bot_setting_list( Bot *bot_ptr, bot_setting *bot_setting_list );
int del_all_bot_settings( Bot* bot_ptr );
int bot_cmd_set( CmdParams *cmdparams );

#endif /* _SETTINGS_H_ */
