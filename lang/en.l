# Emacs: -*- mode: fundamental; coding: binary -*-

# United States English language file.
#
# IRC Services is copyright (c) 1996-2004 Andrew Church.
#     E-mail: <achurch@achurch.org>
# Parts written by Andrew Kempe and others.
# This program is free but copyrighted software; see the file COPYING for
# details.

# When translating this file to another language, keep in mind that the
# order of parameters for sprintf() is fixed in the source code, so your
# messages need to take the same parameters in the same order as the
# English messages do.  (Obviously, this doesn't hold for the strftime()
# format lines immediately below.)  If you can't get a natural translation
# of a message without changing the order of the parameters, please inform
# the Services developers.
#
# In help messages, "%S" (capital S, not lowercase) refers to the name of
# the service sending the message; for example, in NickServ help messages,
# "%S" is replaced by "NickServ" (or whatever it is renamed to in
# config.h).  The %S's do not count as sprintf() parameters, so they can be
# rearranged, removed, or added as necessary.
#
# Also in help messages, please try to limit line lengths to 60 characters
# of text (not including the leading tab).  This length was chosen because
# it does not cause line wrap under default settings on most current IRC
# clients.  Remember that format characters (control-B, control-_) are not
# included in that 60-character limit (since they don't show on the user's
# screen).  Also remember that format specifiers (%S, etc.) will expand
# when displayed, so remember to take this into account; you can assume
# that the length of a pseudoclient name (%S replacement) will be eight
# characters, so reduce the maximum line length by 6 for every %S on a
# line.
#
# Finally, remember to put a tab at the beginning of every line of text
# (including empty lines).  This has to be a tab, not spaces.

###########################################################################
#
# Name of this language
#
###########################################################################

# For languages other than English, this string should have the following
# format:
#     language-name-in-language (language-name-in-English)
# For example, "Español (Spanish)" or "Français (French)".

LANG_NAME
	English

###########################################################################
#
# General messages
#
###########################################################################

# strftime() format strings.  man 3 strftime for information on the
# meanings of the format specifiers.  Short version:
#	%a = weekday name (short)	%H = hour
#	%b = month name (short)		%M = minute
#	%d = day of month		%S = second
#	%Y = year			%Z = time zone

# This is used as the format string for strftime() for a date and time
# together.
STRFTIME_DATE_TIME_FORMAT
	%b %d %H:%M:%S %Y %Z
# This is used as the format string for strftime() for a date alone in long
# format (English: including weekday).
STRFTIME_LONG_DATE_FORMAT
	%a %b %d %Y
# This is used as the format string for strftime() for a date alone in
# short format (English: without weekday).
STRFTIME_SHORT_DATE_FORMAT
	%b %d %Y

# These tell our strftime() what the names of months and days are.  If you
# don't use %a, %A, %b, or %B in your strftime() strings above, you can
# leave these empty.  However, if you enter names, they MUST stay in order,
# one per line, and the list MUST be complete!

# %a
STRFTIME_DAYS_SHORT
	Sun
	Mon
	Tue
	Wed
	Thu
	Fri
	Sat
# %A
STRFTIME_DAYS_LONG
	Sunday
	Monday
	Tuesday
	Wednesday
	Thursday
	Friday
	Saturday
# %b
STRFTIME_MONTHS_SHORT
	Jan
	Feb
	Mar
	Apr
	May
	Jun
	Jul
	Aug
	Sep
	Oct
	Nov
	Dec
# %B
STRFTIME_MONTHS_LONG
	January
	February
	March
	April
	May
	June
	July
	August
	September
	October
	November
	December


# "day", "hour", "minute", and "second" in singular and plural.
# These must be preceded by spaces if the language requires them
# (e.g. in English: "10 hours", not "10hours").
STR_DAY
	 day
STR_DAYS
	 days
STR_HOUR
	 hour
STR_HOURS
	 hours
STR_MINUTE
	 minute
STR_MINUTES
	 minutes
STR_SECOND
	 second
STR_SECONDS
	 seconds
# Separator used between units of time; in English, a comma followed by a
# space (the ", " in "3 hours, 10 minutes").
STR_TIMESEP
	, 

# This is used in ChanServ/NickServ INFO displays, and is a comma followed
# by a space (", " -- trailing space is significant), or the equivalent in
# the language's character set.
COMMA_SPACE
	, 


# Various error messages.
INTERNAL_ERROR
	Internal error--unable to process request.
SERVICES_IS_BUSY
	Services is currently busy; please try again later.

UNKNOWN_COMMAND
	Unknown command %s.
UNKNOWN_COMMAND_HELP
	Unknown command %s.  Type /msg %s HELP for help.
SYNTAX_ERROR
	Syntax: %s
MORE_INFO
	Type /msg %s HELP %s for more information.
NO_HELP_AVAILABLE
	No help available for %s.
MISSING_QUOTE
	Double quote (") missing.

BAD_EMAIL
	E-mail addresses must be in the form username@hostname.  You may not use colors, bold, underline, or reverse, or any of these characters: , : ; | \ " ( ) < > [ ]
BAD_URL
	URLs must be in the form http://hostname[:port]/... (or ftp://, etc.).
BAD_USERHOST_MASK
	Mask must be in the form user@host.
BAD_NICKUSERHOST_MASK
	Mask must be in the form nick!user@host.
BAD_EXPIRY_TIME
	Invalid expiry time.

PASSWORD_INCORRECT
	Password incorrect.
PASSWORD_WARNING
	Warning: After one more incorrect password you will be disconnected.

# What's the difference between "Access denied" and "Permission denied"?
# Very little; you can safely make them the same message with no real loss
# of meaning.  If you want to make a distinction, "Access denied" is
# usually used more generally; for example, a non-oper trying to access
# OperServ gets "Access denied", while a non-Services admin trying to use
# NickServ SET NOEXPIRE gets "Permission denied".
ACCESS_DENIED
	Access denied.
PERMISSION_DENIED
	Permission denied.

PASSWORD_TRUNCATED
	Notice:  Your password has been truncated to %d characters.

NICK_TOO_LONG
	Nicknames must be no more than %d characters long.
NICK_INVALID
	%s is not a valid nickname.
NICK_X_NOT_ON_CHAN_X
	Nickname %s isn't currently on channel %s.

CHAN_INVALID
	%s is not a valid channel name.

SERV_X_NOT_FOUND
	Server %s not found.

###########################################################################

# Please do not change the following line; it is used to determine which
# version of the master (English) language file was used to create a
# translated file.

# CVS: $Id$
