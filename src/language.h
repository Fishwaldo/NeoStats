/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
** Portions copyright (c) 1996-2004 Andrew Church.
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

#ifndef LANGUAGE_H
#define LANGUAGE_H

/*************************************************************************/

/* Languages.  Never insert anything in (or delete anything from) the
 * middle of this list, or everybody will start getting the wrong language!
 * If you want to change the order the languages are displayed in for
 * NickServ HELP SET LANGUAGE, do it in language.c.
 */
#define LANG_EN	0	/* United States English */
#define LANG_UNUSED1	1	/* Unused; was Japanese (JIS encoding) */
#define LANG_JA_EUC	2	/* Japanese (EUC encoding) */
#define LANG_JA_SJIS	3	/* Japanese (SJIS encoding) */
#define LANG_ES		4	/* Spanish */
#define LANG_PT		5	/* Portugese */
#define LANG_FR		6	/* French */
#define LANG_TR		7	/* Turkish */
#define LANG_IT		8	/* Italian */
#define LANG_DE		9	/* German */
#define LANG_NL		10	/* Dutch */
#define LANG_HU		11	/* Hungarian */

#define NUM_LANGS	12	/* Number of languages */
#define LANG_DEFAULT	-1	/* "Use the default" setting */

/* Sanity-check on default language value */
#if DEF_LANGUAGE < 0 || DEF_LANGUAGE >= NUM_LANGS
# error Invalid value for DEF_LANGUAGE: must be >= 0 and < NUM_LANGS
#endif

/*************************************************************************/

/* Flags for maketime() `flags' parameter. */

#define MT_DUALUNIT	0x0001	/* Allow two units (e.g. X hours Y mins) */
#define MT_SECONDS	0x0002	/* Allow seconds (default minutes only) */

/*************************************************************************/

/* External symbol declarations (see language.c for documentation). */

extern int langlist[NUM_LANGS+1];

int lang_init(void);
void lang_fini(void);
int have_language(int language);
const char *getstring_lang(int language, int index);
int setstring(int old, int new);
int strftime_lang(char *buf, int size, int language, int format, time_t time);
char *maketime(int language, time_t time, int flags);
void expires_in_lang(char *buf, int size, int lang, time_t seconds);
void syntax_error(const char *service, const User *u, const char *command, int msgnum);

/*************************************************************************/
/* Definitions of language string constants. */
#include "langstrs.h"

/*************************************************************************/

#endif	/* LANGUAGE_H */
