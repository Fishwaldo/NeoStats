/* nXml - Copyright (C) 2005 bakunin - Andrea Marchesini 
 *                                <bakunin@autistici.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
# error Use configure; make; make install
#endif

#include "nxml.h"
#include "nxml_internal.h"

int
__nxml_escape_spaces (nxml_t * doc, char **buffer, size_t * size)
{
  /* 
   * Rule [3] - S ::= (#x20 | #x9 | #xD | #xA)+
   */

  int k = 0;

  if (!*size)
    return 0;

  while ((**buffer == 0x20 || **buffer == 0x9 || **buffer == 0xd
	  || **buffer == 0xa) && *size)
    {
      if (**buffer == 0xa && doc->priv.func)
	doc->priv.line++;

      (*buffer)++;
      (*size)--;
      k++;
    }

  return k;
}

char *
__nxml_get_value (nxml_t * doc, char **buffer, size_t * size)
{
  char *attr;
  int i;
  int quot;

  if (!*size)
    return NULL;

  if (**buffer == '"')
    quot = 1;

  else if (**buffer == '\'')
    quot = 0;

  else
    return NULL;

  (*buffer)++;
  (*size)--;

  i = 0;
  while (((quot && *(*buffer + i) != '"')
	  || (!quot && *(*buffer + i) != '\'')))
    {
      if (*(*buffer + i) == '\n' && doc->priv.func)
	doc->priv.line++;

      i++;
    }

  if (quot && *(*buffer + i) != '"')
    return NULL;

  else if (!quot && *(*buffer + i) != '\'')
    return NULL;

  if (!(attr = (char *) malloc (sizeof (char) * (i + 1))))
    return NULL;

  memcpy (attr, *buffer, i);

  attr[i] = 0;

  i++;
  (*buffer) += i;
  (*size) -= i;

  return attr;
}

char *
__nxml_trim (char *tmp)
{
  /* Trim function: */
  int i = 0;
  while (tmp[i] == 0x20 || tmp[i] == 0x9 || tmp[i] == 0xd || tmp[i] == 0xa)
    tmp++;

  i = strlen (tmp);
  i--;

  while (tmp[i] == 0x20 || tmp[i] == 0x9 || tmp[i] == 0xd || tmp[i] == 0xa)
    i--;

  tmp[i + 1] = 0;

  return strdup (tmp);
}

static char *
__nxml_find_entity (nxml_t * nxml, char *str, size_t size)
{
  __nxml_doctype_entity_t *e;

  e = nxml->priv.entities;

  while (e)
    {
      if (!strncmp (e->name, str, size))
	return e->reference;
      e = e->next;
    }

  return NULL;
}

char *
__nxml_entity_trim (nxml_t * nxml, char *str)
{
  int i, j, k;
  int len = strlen (str);
  char name[1024];
  char *tmp;
  char *ret_str;
  __nxml_string_t *ret;

  if (!(ret = __nxml_string_new ()))
    return NULL;

  for (i = j = k = 0; i < len; i++)
    {
      if (str[i] == '%' || str[i] == '&')
	{
	  for (j = i; j < len; j++)
	    {
	      if (str[j] == ';')
		{
		  char *find;

		  if (k)
		    {
		      if (__nxml_string_add (ret, name, k))
			{
			  __nxml_string_free (ret);
			  return NULL;
			}

		      k = 0;
		    }

		  if ((find =
		       __nxml_find_entity (nxml, str + i + 1, j - i - 1)))
		    {
		      if (__nxml_string_add (ret, find, 0))
			{
			  __nxml_string_free (ret);
			  return NULL;
			}
		    }
		  else
		    {
		      if (__nxml_string_add (ret, str + i, j - i + 1))
			{
			  __nxml_string_free (ret);
			  return NULL;
			}
		    }

		  i += j;
		  break;
		}
	    }
	}

      else
	{

	  name[k] = str[i];
	  k++;

	  if (k == sizeof (name))
	    {
	      if (__nxml_string_add (ret, name, sizeof (name)))
		{
		  __nxml_string_free (ret);
		  return NULL;
		}
	      k = 0;
	    }
	}
    }

  if (k)
    {
      if (__nxml_string_add (ret, name, k))
	{
	  __nxml_string_free (ret);
	  return NULL;
	}
    }

  tmp = __nxml_string_free (ret);
  ret_str = __nxml_trim (tmp);
  free (tmp);

  return ret_str;
}

/* EOF */
