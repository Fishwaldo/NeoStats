/* mRss - Copyright (C) 2005 bakunin - Andrea Marchesini 
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

#include "mrss.h"
#include "mrss_internal.h"

char *
mrss_strerror (mrss_error_t err)
{
  switch (err)
    {
    case MRSS_OK:
      return "Success";

    case MRSS_ERR_PARSER:
      return "Parser error";

    case MRSS_ERR_VERSION:
      return "Version error";

    case MRSS_ERR_DATA:
      return "No correct paramenter in the function";

    default:
      return strerror (errno);
    }
}

mrss_error_t
mrss_element (mrss_generic_t element, mrss_element_t * ret)
{
  mrss_t *tmp;

  if (!element || !ret)
    return MRSS_ERR_DATA;

  tmp = (mrss_t *) element;
  *ret = tmp->element;
  return MRSS_OK;
}

/* EOF */
