
#ifndef UNREALMODES_H
#define UNREALMODES_H


aCtab cFlagTab[] = {
	{MODE_LIMIT, 'l', 0, 1},
	{MODE_VOICE, 'v', 1, 1},
	{MODE_HALFOP, 'h', 1, 1},
	{MODE_CHANOP, 'o', 1, 1},
	{MODE_PRIVATE, 'p', 0, 0},
	{MODE_SECRET, 's', 0, 0},
	{MODE_MODERATED, 'm', 0, 0},
	{MODE_NOPRIVMSGS, 'n', 0, 0},
	{MODE_TOPICLIMIT, 't', 0, 0},
	{MODE_INVITEONLY, 'i', 0, 0},
	{MODE_KEY, 'k', 0, 1},
	{MODE_RGSTR, 'r', 0, 0},
	{MODE_RGSTRONLY, 'R', 0, 0},
	{MODE_NOCOLOR, 'c', 0, 0},
	{MODE_CHANPROT, 'a', 1, 1},
	{MODE_CHANOWNER, 'q', 1, 1},
	{MODE_OPERONLY, 'O', 0, 0},
	{MODE_ADMONLY, 'A', 0, 0},
	{MODE_LINK, 'L', 0, 1},
	{MODE_NOKICKS, 'Q', 0, 0},
	{MODE_BAN, 'b', 0, 1},
	{MODE_STRIP, 'S', 0, 0},	/* works? */
	{MODE_EXCEPT, 'e', 0, 0},	/* exception ban */
	{MODE_NOKNOCK, 'K', 0, 0},	/* knock knock (no way!) */
	{MODE_NOINVITE, 'V', 0, 0},	/* no invites */
	{MODE_FLOODLIMIT, 'f', 0, 1},	/* flood limiter */
	{MODE_NOHIDING, 'H', 0, 0},	/* no +I joiners */
	{MODE_STRIPBADWORDS, 'G', 0, 0},	/* no badwords */
	{MODE_NOCTCP, 'C', 0, 0},	/* no CTCPs */
	{MODE_AUDITORIUM, 'u', 0, 0},
	{MODE_ONLYSECURE, 'z', 0, 0},
	{MODE_NONICKCHANGE, 'N', 0, 0},
	{0x0, 0x0, 0x0}
};


#endif
