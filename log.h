/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
** $Id: log.h,v 1.4 2003/07/17 10:13:51 fishwaldo Exp $
*/


#ifndef _log_h_
#define _log_h_

/*
 * log.h
 * dynamic configuration runtime libary
 */

/* define the log levels */

/* critcal crash type notices */
#define LOG_CRITICAL   	1
/* something is majorly wrong */
#define LOG_ERROR	2
/* Hey, you should know about this type messages */
#define LOG_WARNING	3
/* did you know messages */
#define LOG_NOTICE	4
/* our normal logging level? */
#define LOG_NORMAL	5
/* lots of info about what we are doing */
#define LOG_INFO	6
/* debug notices about important functions that are going on */
#define LOG_DEBUG1	7
/* more debug notices that are usefull */
#define LOG_DEBUG2	8
/* even more stuff, that would be useless to most normal people */
#define LOG_DEBUG3	9
/* are you insane? */
#define LOG_DEBUG4	10


/* Scope of Logging Defines: */

#define LOG_CORE	0
#define LOG_MOD		1


/* this is for the neostats assert replacement. */
/* Version 2.4 and later of GCC define a magical variable _PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */

# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __NASSERT_FUNCTION    __PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __NASSERT_FUNCTION    __func__
#  else
#   define __NASSERT_FUNCTION    ((__const char *) 0)
#  endif
# endif

extern void nassert_fail(const char *expr, const char *file, const int line, const char *infunk);

#define nassert(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :                                           \
	(nassert_fail(__STRING(expr), __FILE__, __LINE__, __NASSERT_FUNCTION), 0)))
            



extern void nlog(int level, int scope, char *fmt, ...);
void *close_logs();
void init_logs();

#endif
