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

#ifndef _IRCSTRING_H_
#define _IRCSTRING_H_

#include <stdarg.h>
#include <stdio.h>

/* [v]s[n]printf replacements */
int ircsprintf(char *buf, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
int ircvsprintf(char *buf, const char *fmt, va_list args);
int ircvsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int ircsnprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

/* str[n]casecmp replacements */
int ircstrcasecmp(const char *s1, const char *s2);
int ircstrncasecmp(const char *s1, const char *s2, size_t size);

#endif /* _IRCSTRING_H_ */
