/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *This file is part of The Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *GNU The Croco Library is distributed in the hope 
 *that it will be useful, but WITHOUT ANY WARRANTY; 
 *without even the implied warranty of 
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the 
 *GNU General Public License along with The Croco Library; 
 *see the file COPYING. If not, write to 
 *the Free Software Foundation, Inc., 
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *Copyright 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 */

/*
 *$Id$
 */

/**
 *@file
 *The definition of the #CREncHandler class.
 */


#include "cr-enc-handler.h"
#include "cr-utils.h"

#include <string.h>

struct CREncAlias {
	guchar * name ;
	enum CREncoding encoding ;
} ;

static struct CREncAlias gv_default_aliases[] =
{
	{"UTF-8", CR_UTF_8},
	{"UTF_8", CR_UTF_8},
	{"UTF8", CR_UTF_8},
	{"UTF-16", CR_UTF_16},
	{"UTF_16", CR_UTF_16},
	{"UTF16", CR_UTF_16},
	{"UCS1", CR_UCS_1},
	{"UCS-1", CR_UCS_1},
	{"UCS_1", CR_UCS_1},
	{"ISO-8859-1", CR_UCS_1},
	{"ISO_8859-1", CR_UCS_1},
	{"UCS-1", CR_UCS_1},
	{"UCS_1", CR_UCS_1},
	{"UCS4", CR_UCS_4},
	{"UCS-4", CR_UCS_4},
	{"UCS_4", CR_UCS_4},
	{"ASCII", CR_ASCII},
	{0}
} ;


static CREncHandler gv_default_enc_handlers[] =
{
	{CR_UCS_1, cr_utils_ucs1_to_utf8, cr_utils_utf8_to_ucs1,
        cr_utils_ucs1_str_len_as_utf8, cr_utils_utf8_str_len_as_ucs1},

        {CR_ISO_8859_1, cr_utils_ucs1_to_utf8, cr_utils_utf8_to_ucs1,
        cr_utils_ucs1_str_len_as_utf8, cr_utils_utf8_str_len_as_ucs1},

        {CR_ASCII, cr_utils_ucs1_to_utf8, cr_utils_utf8_to_ucs1,
         cr_utils_ucs1_str_len_as_utf8, cr_utils_utf8_str_len_as_ucs1},

        {0}
} ;


/**
 *Gets the instance of encoding handler.
 *This function implements a singleton pattern.
 *@param a_enc the encoding of the Handler.
 *@return the instance of #CREncHandler.
 */
CREncHandler *
cr_enc_handler_get_instance (enum CREncoding a_enc)
{
        gulong i = 0 ;

        for (i = 0 ;gv_default_enc_handlers[i].encoding ; i++)
        {
                if (gv_default_enc_handlers[i].encoding == a_enc)
                {
                        return 
                                (CREncHandler *)
                                &gv_default_enc_handlers[i].encoding ;
                }
        }

        return NULL ;
}


/**
 *Given an encoding name (called an alias name)
 *the function returns the matching encoding type.
 *@param a_alias_name the encoding name
 *@param a_enc output param. The returned encoding type
 *or 0 if the alias is not supported.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_enc_handler_resolve_enc_alias (guchar *a_alias_name, 
				  enum CREncoding *a_enc)
{
	gulong i = 0 ;
	guchar * alias_name_up = NULL ;
	enum CRStatus status = CR_ENCODING_NOT_FOUND_ERROR ;

	g_return_val_if_fail (a_alias_name != NULL, CR_BAD_PARAM_ERROR) ;

	alias_name_up = g_strdup (a_alias_name) ;
	g_strup (alias_name_up) ;

	for (i = 0 ; gv_default_aliases[i].name ; i++)
	{
		if (!strcmp (gv_default_aliases[i].name, alias_name_up))
		{
			*a_enc = gv_default_aliases[i].encoding ;
			status = CR_OK ;
			break ;
		}
	}

	return status ;
}


/**
 *Converts a raw input buffer into an utf8 buffer.
 *@param a_this the current instance of #CREncHandler.
 *@param a_in the input buffer to convert.
 *@param a_in_len in/out parameter. The len of the input
 *buffer to convert. After return, contains the number of
 *bytes actually consumed.
 *@param @a_out output parameter. The converted output buffer.
 *Must be freed by the buffer.
 *@param a_out_len output parameter. The length of the output buffer.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_enc_handler_convert_input (CREncHandler *a_this,
                              guchar *a_in,
                              gulong *a_in_len,
                              guchar **a_out,
                              gulong *a_out_len)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_in && a_in_len && a_out,
                              CR_BAD_PARAM_ERROR) ;

        if (a_this->decode_input == NULL) return CR_OK ;

        if (a_this->enc_str_len_as_utf8)
        {
                status =
                        a_this->enc_str_len_as_utf8 (a_in,
                                                    &a_in[*a_in_len -1],
                                                    a_out_len) ;

                g_return_val_if_fail (status == CR_OK, status) ;
        }
        else
        {
                *a_out_len = *a_in_len ;
        }

        *a_out = g_malloc0 (*a_out_len) ;

        status = a_this->decode_input (a_in, a_in_len, *a_out, a_out_len) ;

        if (status != CR_OK)
        {
                g_free (*a_out) ;
                *a_out = NULL ;
        }

        g_return_val_if_fail (status == CR_OK, status) ;

        return CR_OK ;
}
