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
#ifndef MODES_H
#define MODES_H

unsigned int UmodeStringToMask (const char *UmodeString);
char *UmodeMaskToString (const unsigned int mask);
char UmodeMaskToChar (const unsigned int mask);

unsigned int SmodeStringToMask (const char *UmodeString);
char *SmodeMaskToString (const unsigned int mask);
char SmodeMaskToChar (const unsigned int mask);

unsigned int CmodeStringToMask (const char *UmodeString);
char *CmodeMaskToString (const unsigned int mask);
char *CmodeMaskToPrefixString (const unsigned int mask);
int CmodeCharToMask (const char mode);
char CmodeMaskToChar (const unsigned int mask);
EXPORTFUNC int CmodeCharToFlags (const char mode);
unsigned int CmodePrefixToMask (const char prefix);
char CmodePrefixToChar (const char prefix);
char CmodeMaskToPrefix (const unsigned int mask);
char CmodeCharToPrefix (const char mode);

int InitIrcdModes (void);

EXPORTFUNC int ChanMode (char *origin, char **av, int ac);
int ChanModeHandler (Channel* c, char *modes, int j, char **av, int ac);
void ChanUserMode (const char *chan, const char *nick, int add, const unsigned int mode);

void dumpchanmodes (CmdParams* cmdparams, Channel* c);


#endif
