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

#include "neostats.h"
#include "ircd.h"
#include "modes.h"

unsigned char UmodeChRegNick = 'r';
static char UmodeStringBuf[64];
static char SmodeStringBuf[64];
unsigned int ircd_supported_umodes = 0;
unsigned int ircd_supported_smodes = 0;
cumode_init* chan_umodes;
cmode_init* chan_modes;
umode_init* user_umodes;
umode_init* user_smodes;
ChanModes ircd_cmodes[MODE_TABLE_SIZE];
UserModes ircd_umodes[MODE_TABLE_SIZE];
UserModes ircd_smodes[MODE_TABLE_SIZE];

typedef struct ModeDesc {
	unsigned int mask;
	const char * desc;
} ModeDesc;

static ModeDesc UmodeDesc[] = {
#ifdef UMODE_DEBUG
	{UMODE_DEBUG,		"Debug"},
#endif
	{UMODE_TECHADMIN,	"Technical Administrator"},
#ifdef UMODE_SERVICESOPER
	{UMODE_SERVICESOPER,"Services operator"},
#endif
#ifdef UMODE_IRCADMIN
	{UMODE_IRCADMIN,	"IRC admin"},
#endif
#ifdef UMODE_SUPER
	{UMODE_SUPER,		"Super"},
#endif
#ifdef UMODE_SRA
	{UMODE_SRA,			"Services root"},
#endif
	{UMODE_SERVICES,	"Network Service"},
	{UMODE_NETADMIN,	"Network Administrator"},
	{UMODE_SADMIN,		"Services Administrator"},
	{UMODE_ADMIN,		"Server Administrator"},
	{UMODE_COADMIN,		"Co-Server Administrator"},
	{UMODE_OPER,		"Global Operator"},
	{UMODE_LOCOP,		"Local Operator"},
	{UMODE_REGNICK,		"Registered nick"},
	{UMODE_BOT,			"Bot"},
	{0, 0},
};

static ModeDesc SmodeDesc[] = {
	{SMODE_NETADMIN,	"Network Administrator"},
	{SMODE_CONETADMIN,	"Co-Network Administrator"},
	{SMODE_TECHADMIN,	"Technical Administrator"},
	{SMODE_COTECHADMIN,	"Co-Technical Administrator"},
	{SMODE_ADMIN,		"Server Administrator"},
	{SMODE_GUESTADMIN,	"Guest Administrator"},
	{SMODE_COADMIN,		"Co-Server Administrator"},
	{0, 0},
};

/** @brief InitIrcdModes
 *
 *  Build internal mode tables by translating the protocol information
 *  into a faster indexed lookup table
 *
 * @return 
 */
int
InitIrcdModes (void)
{
	cmode_init* cmodes;
	cumode_init* cumodes;
	umode_init* umodes;
	umode_init* smodes;

	/* build cmode lookup table */
	memset(&ircd_cmodes, 0, sizeof(ircd_cmodes));
	cmodes = chan_modes;
	while(cmodes->modechar) 
	{
		dlog(DEBUG4, "Adding channel mode %c", cmodes->modechar);
		ircd_cmodes[(int)cmodes->modechar].mode = cmodes->mode;
		ircd_cmodes[(int)cmodes->modechar].flags = cmodes->flags;
		cmodes ++;
	}
	cumodes = chan_umodes;
	while(cumodes->modechar) 
	{
		dlog(DEBUG4, "Adding channel user mode %c", cumodes->modechar);
		ircd_cmodes[(int)cumodes->modechar].mode = cumodes->mode;
		ircd_cmodes[(int)cumodes->modechar].sjoin = cumodes->sjoin;
		ircd_cmodes[(int)cmodes->modechar].flags = NICKPARAM;
		cumodes ++;
	}
	/* build umode lookup table */
	memset(&ircd_umodes, 0, sizeof(ircd_umodes));
	umodes = user_umodes;
	while(umodes->modechar) 
	{	
		dlog(DEBUG4, "Adding user mode %c", umodes->modechar);
		ircd_umodes[(int)umodes->modechar].umode = umodes->umode;
		ircd_umodes[(int)umodes->modechar].flags = umodes->flags;
		/* Build supported modes mask */
		ircd_supported_umodes |= umodes->umode;
		if(umodes->umode&UMODE_REGNICK) {
			UmodeChRegNick = umodes->modechar;
		}
		umodes ++;
	}
	/* build smode lookup table */
	memset(&ircd_smodes, 0, sizeof(ircd_smodes));
	smodes = user_smodes;
	if(smodes) {
		while(umodes->modechar) 
		{
			dlog(DEBUG4, "Adding user smode %c", smodes->modechar);
			ircd_smodes[(int)smodes->modechar].umode = smodes->umode;
			ircd_smodes[(int)smodes->modechar].flags = smodes->flags;
			/* Build supported smodes mask */
			ircd_supported_umodes |= umodes->umode;
			smodes ++;
		}
	}
	/* preset our umode mask so we do not have to calculate in real time */
	me.servicesumodemask = UmodeStringToMask(me.servicesumode, 0);
	return NS_SUCCESS;
};

/** @brief UmodeMaskToString
 *
 *  Translate a mode mask to the string equivalent
 *
 * @return 
 */
char* 
UmodeMaskToString(const long Umode) 
{
	int i, j;

	UmodeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < MODE_TABLE_SIZE; i++) {
		if (Umode & ircd_umodes[i].umode) {
			UmodeStringBuf[j] = i;
			j++;
		}
	}
	UmodeStringBuf[j] = '\0';
	return UmodeStringBuf;
}

/** @brief UmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
long
UmodeStringToMask(const char* UmodeString, long Umode)
{
	int add = 0;
	char* tmpmode;

	/* Walk through mode string and convert to umode */
	tmpmode = (char*)UmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (add) {
				Umode |= ircd_umodes[(int)*tmpmode].umode;
				break;
			} else {
				Umode &= ~ircd_umodes[(int)*tmpmode].umode;
				break;
			}
		}
		tmpmode++;
	}
	return Umode;
}

/** @brief SmodeMaskToString
 *
 *  Translate a smode mask to the string equivalent
 *
 * @return 
 */
char* 
SmodeMaskToString(const long Smode) 
{
	int i, j;

	SmodeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < MODE_TABLE_SIZE; i++) {
		if (Smode & ircd_smodes[i].umode) {
			SmodeStringBuf[j] = i;
			j++;
		}
	}
	SmodeStringBuf[j] = '\0';
	return SmodeStringBuf;
}

/** @brief SmodeStringToMask
 *
 *  Translate a smode string to the mask equivalent
 *
 * @return 
 */
long
SmodeStringToMask(const char* SmodeString, long Smode)
{
	int add = 0;
	char* tmpmode;

	/* Walk through mode string and convert to smode */
	tmpmode = (char*)SmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (add) {
				Smode |= ircd_smodes[(int)*tmpmode].umode;
				break;
			} else {
				Smode &= ~ircd_smodes[(int)*tmpmode].umode;
				break;
			}
		}
		tmpmode++;
	}
	return Smode;
}

/** @brief CUmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
long
CUmodeStringToMask (const char* UmodeString, long Umode)
{
	int add = 0;
	char* tmpmode;

	/* Walk through mode string and convert to umode */
	tmpmode = (char*)UmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (add) {
				Umode |= ircd_cmodes[(int)*tmpmode].mode;
				break;
			} else {
				Umode &= ~ircd_cmodes[(int)*tmpmode].mode;
				break;
			}
		}
		tmpmode++;
	}
	return Umode;
}

int IsBotMode (const char mode)
{
	if(ircd_umodes[(int)mode].umode & UMODE_BOT) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

int GetUmodeMask (const char mode)
{
	return ircd_umodes[(int)mode].umode;
}

int GetSmodeMask (const char mode)
{
	return ircd_smodes[(int)mode].umode;
}

const char * GetUmodeDesc (const unsigned int mask)
{
	ModeDesc* entry;

	entry = UmodeDesc;
	while(entry->mask) {
		if (entry->mask == mask) {
			return entry->desc;
		}
		entry ++;
	}
	return NULL;
}

const char * GetSmodeDesc (const unsigned int mask)
{
	ModeDesc* entry;

	entry = SmodeDesc;
	while(entry->mask) {
		if (entry->mask == mask) {
			return entry->desc;
		}
		entry ++;
	}
	return NULL;
}

