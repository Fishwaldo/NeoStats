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
#include "services.h"
#include "users.h"
#include "channels.h"
#include "modules.h"

typedef struct mode_data {
	unsigned int mask;
	unsigned int flags;
	unsigned char sjoin;
} mode_data;

unsigned char UmodeChRegNick = 'r';
static char ModeStringBuf[64];
static char PrefixStringBuf[64];
unsigned int ircd_supported_umodes = 0;
unsigned int ircd_supported_smodes = 0;
unsigned int ircd_supported_cmodes = 0;
unsigned int ircd_supported_cumodes = 0;

static mode_data ircd_cmodes[MODE_TABLE_SIZE];
static mode_data ircd_umodes[MODE_TABLE_SIZE];
static mode_data ircd_smodes[MODE_TABLE_SIZE];

static char ircd_cmode_char_map[32];
static char ircd_umode_char_map[32];
static char ircd_smode_char_map[32];

typedef struct ModeDesc {
	unsigned int mask;
	const char *desc;
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

/** @brief BuildModeTable
 *
 *  Build internal mode tables by translating the protocol information
 *  into a faster indexed lookup table
 *
 * @return 
 */
unsigned int
BuildModeTable (char *mode_char_map, mode_data * dest, const mode_init * src, unsigned int flagall)
{
	unsigned int maskall = 0;

	while(src->mode) 
	{
		dlog(DEBUG4, "Adding mode %c", src->mode);
		dest[(int)src->mode].mask = src->mask;
		dest[(int)src->mode].flags = src->flags;
		dest[(int)src->mode].flags |= flagall;
		dest[(int)src->mode].sjoin = src->sjoin;
		/* Build supported modes mask */
		maskall |= src->mask;
		src ++;
	}
	return maskall;
}

/** @brief InitModeTables
 *
 *  Build internal mode tables by translating the protocol information
 *  into a faster indexed lookup table
 *
 * @return 
 */
int
InitModeTables (const mode_init* chan_umodes, const mode_init* chan_modes, const mode_init* user_umodes, const mode_init* user_smodes)
{
	/* build cmode lookup table */
	memset(&ircd_cmodes, 0, sizeof(ircd_cmodes));
	dlog(DEBUG4, "Build channel mode table...");
	ircd_supported_cmodes = BuildModeTable (ircd_cmode_char_map, ircd_cmodes, chan_modes, 0);
	/* build cumode lookup table */
	dlog(DEBUG4, "Build channel user mode table...");
	ircd_supported_cumodes = BuildModeTable (ircd_cmode_char_map, ircd_cmodes, chan_umodes, NICKPARAM);
	/* build umode lookup table */
	memset(&ircd_umodes, 0, sizeof(ircd_umodes));
	dlog(DEBUG4, "Build user mode table...");
	ircd_supported_umodes = BuildModeTable (ircd_umode_char_map, ircd_umodes, user_umodes, 0);
	/* build smode lookup table */
	if (user_smodes) {
		memset(&ircd_smodes, 0, sizeof(ircd_smodes));
		dlog(DEBUG4, "Build user smode table...");
		ircd_supported_smodes = BuildModeTable (ircd_smode_char_map, ircd_smodes, user_smodes, 0);
	}
	/* Check for registered nick support */
	if(ircd_supported_umodes & UMODE_REGNICK) {
		UmodeChRegNick = UmodeMaskToChar(UMODE_REGNICK);
	}
	/* preset our umode mask so we do not have to calculate in real time */
	me.servicesumodemask = UmodeStringToMask(me.servicesumode);
	return NS_SUCCESS;
};

/** @brief UmodeMaskToString
 *
 *  Translate a mode mask to the string equivalent
 *
 * @return 
 */
char *
ModeMaskToString (mode_data* mode_table, const long mask) 
{
	int i, j;

	ModeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < MODE_TABLE_SIZE; i++) {
		if (mask & mode_table[i].mask) {
			ModeStringBuf[j] = i;
			j++;
		}
	}
	ModeStringBuf[j] = '\0';
	return ModeStringBuf;
}

/** @brief UmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
unsigned int 
ModeStringToMask (mode_data* mode_table, const char *ModeString)
{
	unsigned int Umode = 0;
	int add = 0;
	char *tmpmode;

	/* Walk through mode string and convert to mask */
	tmpmode = (char*)ModeString;
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
				Umode |= mode_table[(int)*tmpmode].mask;
				break;
			} else {
				Umode &= ~mode_table[(int)*tmpmode].mask;
				break;
			}
		}
		tmpmode++;
	}
	return Umode;
}

/** @brief UmodeMaskToString
 *
 *  Translate a mode mask to the string equivalent
 *
 * @return 
 */
char *
UmodeMaskToString(const unsigned int Umode) 
{
	return ModeMaskToString(ircd_umodes, Umode);
}

/** @brief UmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
unsigned int
UmodeStringToMask(const char *UmodeString)
{
	return ModeStringToMask (ircd_umodes, UmodeString);
}

/** @brief SmodeMaskToString
 *
 *  Translate a smode mask to the string equivalent
 *
 * @return 
 */
char *
SmodeMaskToString(const unsigned int Smode) 
{
	return ModeMaskToString(ircd_smodes, Smode);
}

/** @brief SmodeStringToMask
 *
 *  Translate a smode string to the mask equivalent
 *
 * @return 
 */
unsigned int
SmodeStringToMask (const char *SmodeString)
{
	return ModeStringToMask (ircd_smodes, SmodeString);
}

/** @brief CmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
unsigned int
CmodeStringToMask (const char *UmodeString)
{
	return ModeStringToMask (ircd_cmodes, UmodeString);
}

char *CmodeMaskToString (const unsigned int mask)
{
	return ModeMaskToString (ircd_cmodes, mask);
}

char *CmodeMaskToPrefixString (const unsigned int mask)
{
	int i, j;

	j = 0;
	for (i = 0; i < MODE_TABLE_SIZE; i++) {
		if (mask & ircd_cmodes[i].mask) {
			PrefixStringBuf[j] = ircd_cmodes[i].sjoin;
			j++;
		}
	}
	PrefixStringBuf[j] = '\0';
	return PrefixStringBuf;
}


int IsBotMode (const char mode)
{
	if(ircd_umodes[(int)mode].mask & UMODE_BOT) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

int UmodeCharToMask (const char mode)
{
	return ircd_umodes[(int)mode].mask;
}

int SmodeCharToMask (const char mode)
{
	return ircd_smodes[(int)mode].mask;
}

int CmodeCharToMask (const char mode)
{
	return ircd_cmodes[(int)mode].mask;
}

int CmodeCharToFlags (const char mode)
{
	return ircd_cmodes[(int)mode].flags;
}

char ModeMaskToChar (char *mode_char_map, unsigned int mask)
{
    int bitcount = 0;
	
	while (mask >>= 1) {
		bitcount ++;
	}
    return mode_char_map[bitcount];
}

char UmodeMaskToChar (const unsigned int mask)
{
	return ModeMaskToChar (ircd_umode_char_map, mask);
}

char SmodeMaskToChar (const unsigned int mask)
{
	return ModeMaskToChar (ircd_smode_char_map, mask);
}

char CmodeMaskToChar (const unsigned int mask)
{
	return ModeMaskToChar (ircd_cmode_char_map, mask);
}

unsigned int CmodePrefixToMask (const char prefix)
{
	int i;

	for (i = 0; i < MODE_TABLE_SIZE; i++)
	{
		if (ircd_cmodes[i].sjoin == prefix) {
			return ircd_cmodes[i].mask;
		}
	}
	return 0;
}

char CmodePrefixToChar (const char prefix)
{
	int i;

	for (i = 0; i < MODE_TABLE_SIZE; i++)
	{
		if (ircd_cmodes[i].sjoin == prefix) {
			return i;
		}
	}
	return 0;
}

char CmodeMaskToPrefix (const unsigned int mask)
{
	return ircd_cmodes[(int) ModeMaskToChar (ircd_umode_char_map, mask)].sjoin;
}

char CmodeCharToPrefix (const char mode)
{
	return (ircd_cmodes[(int)mode].sjoin);
}

const char *GetUmodeDesc (const unsigned int mask)
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

const char *GetSmodeDesc (const unsigned int mask)
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

/** @brief Check if a mode is set on a Channel
 * 
 * used to check if a mode is set on a channel
 * 
 * @param c channel to check
 * @param mode is the mode to check, as a LONG
 *
 * @returns 1 on match, 0 on no match, -1 on error
 *
*/
int
CheckChanMode (Channel *c, const unsigned int mask)
{
	ModesParm *m;
	lnode_t *mn;

	if (!c) {
		nlog (LOG_WARNING, "CheckChanMode: tied to check modes of empty channel");
		return -1;
	}
	if (c->modes & mask) {
		/* its a match */
		return 1;
	}
	/* if we get here, we have to check the modeparm list first */
	mn = list_first (c->modeparms);
	while (mn) {
		m = lnode_get (mn);
		if (m->mask & mask) {
			/* its a match */
			return 1;
		}
		mn = list_next (c->modeparms, mn);
	}
	return 0;
}


/** @brief Compare channel modes from the channel hash
 *
 * used in mode_data to compare modes (list_find argument)
 *
 * @param v actually its a ModeParm struct
 * @param mask is the mode as a long
 *
 * @return 0 on match, 1 otherwise.
*/

static int
comparemode (const void *v, const void *mask)
{
	ModesParm *m = (void *) v;

	if (m->mask == (unsigned int) mask) {
		return 0;
	} else {
		return 1;
	}
}

/** @brief Process a mode change on a channel
 *
 * process a mode change on a channel adding and deleting modes as required
 *
 * @param origin usually the server that sent the mode change. Not used
 * @param av array of variables to pass
 * @param ac number of variables n av
 *
 * @return 0 on error, number of modes processed on success.
*/

int ChanModeHandler (Channel* c, char *modes, int j, char **av, int ac)
{
	int modeexists;
	ModesParm *m;
	lnode_t *mn;
	int add = 0;

	while (*modes) {
		unsigned int mask;
		unsigned int flags;      

		mask = CmodeCharToMask (*modes);
		flags = CmodeCharToFlags (*modes);

		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if (flags&NICKPARAM) {
				ChanUserMode (av[0], av[j], add, mask);
				j++;
			} else if (add) {
				/* mode limit and mode key replace current values */
				if (mask == CMODE_LIMIT) {
					c->limit = atoi(av[j]);
					j++;
				} else if (mask == CMODE_KEY) {
					strlcpy (c->key, av[j], KEYLEN);
					j++;
				} else if (flags) {
					mn = list_first (c->modeparms);
					modeexists = 0;
					while (mn) {
						m = lnode_get (mn);
						if ((m->mask == mask) && !ircstrcasecmp (m->param, av[j])) {
							dlog(DEBUG1, "ChanMode: Mode %c (%s) already exists, not adding again", *modes, av[j]);
							j++;
							modeexists = 1;
							break;
						}
						mn = list_next (c->modeparms, mn);
					}
					if (modeexists != 1) {
						m = ns_calloc (sizeof (ModesParm));
						m->mask = mask;
						strlcpy (m->param, av[j], PARAMSIZE);
						if (list_isfull (c->modeparms)) {
							nlog (LOG_CRITICAL, "ChanMode: modelist is full adding to channel %s", c->name);
							do_exit (NS_EXIT_ERROR, "List full - see log file");
						}
						lnode_create_append (c->modeparms, m);
						j++;
					}
				} else {
					c->modes |= mask;
				}
			} else {
				if(mask == CMODE_LIMIT) {
					c->limit = 0;
				} else if (mask == CMODE_KEY) {
					c->key[0] = 0;
					j++;
				} else if (flags) {
					mn = list_find (c->modeparms, (void *) mask, comparemode);
					if (!mn) {
						dlog(DEBUG1, "ChanMode: can't find mode %c for channel %s", *modes, c->name);
					} else {
						list_delete (c->modeparms, mn);
						m = lnode_get (mn);
						lnode_destroy (mn);
						ns_free (m);
					}
				} else {
					c->modes &= ~mask;
				}
			}
		}
		modes++;
	}
	return j;
}

int ChanMode (char *origin, char **av, int ac)
{
	Channel *c;
	CmdParams * cmdparams;
	int i, j = 2;

	c = find_chan (av[0]);
	if (!c) {
		return 0;
	}	
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->channel = c;
	AddStringToList(&cmdparams->av, origin, &cmdparams->ac);
	for (i = 0; i < ac; i++) {
		AddStringToList(&cmdparams->av, av[i], &cmdparams->ac);	
	}
	SendAllModuleEvent(EVENT_CMODE, cmdparams);
	ns_free(cmdparams);	
	j = ChanModeHandler (c, av[1], j, av, ac);
	return j;
}

/** @brief Process a mode change that affects a user on a channel
 *
 * process a mode change on a channel that affects a user
 *
 * @param c Channel Struct of channel mode being changed
 * @param u User struct of user that mode is affecting
 * @param add 1 means add, 0 means remove mode
 * @param mask is the long int of the mode
 *
 * @return Nothing
*/

void
ChanUserMode (const char *chan, const char *nick, int add, const unsigned int mask)
{
	Chanmem *cm;
	Channel *c;
	Client *u;

	u = find_user(nick);
	if (!u) {
		nlog (LOG_WARNING, "ChanUserMode: can't find user %s", nick);
		return;
	}
	c = find_chan(chan);
	if (!c) {
		nlog (LOG_WARNING, "ChanUserMode: can't find channel %s", chan);
		return;
	}
	cm = lnode_find (c->chanmembers, u->name, comparef);
	if (!cm) {
		nlog (LOG_WARNING, "ChanUserMode: %s is not a member of channel %s", u->name, c->name);
		return;
	}
	if (add) {
		dlog(DEBUG2, "ChanUserMode: Adding mode %x to Channel %s User %s", mask, c->name, u->name);
		cm->flags |= mask;
	} else {
		dlog(DEBUG2, "ChanUserMode: Deleting Mode %x to Channel %s User %s", mask, c->name, u->name);
		cm->flags &= ~mask;
	}
}

void 
dumpchanmodes (CmdParams* cmdparams, Channel* c)
{
	lnode_t *cmn;
	ModesParm *m;
	int i;

	irc_prefmsg (ns_botptr, cmdparams->source, __("Mode:       %s", cmdparams->source), UmodeMaskToString (c->modes));
	cmn = list_first (c->modeparms);
	while (cmn) {
		m = lnode_get (cmn);
		for (i = 0; i < MODE_TABLE_SIZE; i++) {
			if (m->mask & ircd_cmodes[i].mask) {
				irc_prefmsg (ns_botptr, cmdparams->source, __("Modes:      %c Parms %s", cmdparams->source), i, m->param);
			}
		}
		cmn = list_next (c->modeparms, cmn);
	}

}
