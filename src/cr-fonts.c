/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of 
 * the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 *See COPYRIGHTS file for copyright information
 */

#include "cr-fonts.h"
#include <string.h>


static enum CRStatus
cr_font_family_to_string_real (CRFontFamily *a_this,
                               gboolean a_walk_list,
                               GString **a_string)
{
        guchar * name = NULL ;
        enum CRStatus result = CR_OK ;

        if (!*a_string)
        {
                *a_string = g_string_new (NULL) ;
                g_return_val_if_fail (*a_string,
                                      CR_INSTANCIATION_FAILED_ERROR) ;
        }
        
        if (!a_this)
        {
                g_string_append_printf (*a_string, "NULL") ;
                return CR_OK ;
        }

        switch (a_this->type)
        {
        case FONT_FAMILY_SANS_SERIF:
                name = (guchar*) "sans-serif" ;
                break ;

        case FONT_FAMILY_SERIF:
                name = (guchar*) "sans-serif" ;
                break ;

        case FONT_FAMILY_CURSIVE:
                name = (guchar*) "cursive" ;
                break ;

        case FONT_FAMILY_FANTASY:
                name = (guchar*) "fantasy" ;
                break ;

        case FONT_FAMILY_MONOSPACE:
                name = (guchar*) "monospace" ;
                break ;

        case FONT_FAMILY_NON_GENERIC:
                name = (guchar*) a_this->name;
                break ;

        default:
                name = (guchar*) NULL ;
                break ;
        }

        if (name)
        {
                if (a_this->prev)
                {
                        g_string_append_printf (*a_string, ", %s", name) ;
                }
                else
                {
                        g_string_append (*a_string, name) ;
                }
        }
        if (a_walk_list == TRUE && a_this->next)
        {
                result = cr_font_family_to_string_real (a_this->next,
                                                        TRUE, a_string) ;
        }
        return result ;
}

static const gchar *
cr_predefined_absolute_font_size_to_string (enum CRPredefinedAbsoluteFontSize a_code)
{
        gchar *str = NULL ;
        switch (a_code)
        {
        case FONT_SIZE_XX_SMALL:
                str = (gchar*)"font-size-xx-small" ;
                break ;
        case FONT_SIZE_X_SMALL:
                str = (gchar*)"font-size-x-small" ;
                break ;
        case FONT_SIZE_SMALL:
                str = (gchar*)"font-size-small" ;
                break ;
        case FONT_SIZE_MEDIUM:
                str = (gchar*)"font-size-medium" ;
                break ;
        case FONT_SIZE_LARGE:
                str = (gchar*)"font-size-large" ;
                break ;
        case FONT_SIZE_X_LARGE:
                str = (gchar*)"font-size-x-large" ;
                break ;
        case FONT_SIZE_XX_LARGE:
                str = (gchar*)"font-size-xx-large" ;
                break ;
        default:
                str = (gchar*) "unknown predefined absolute font size value";
        }
        return str ;
}

static const gchar *
cr_relative_font_size_to_string (enum CRRelativeFontSize a_code)
{
        gchar *str = NULL ;

        switch (a_code)
        {
        case FONT_SIZE_LARGER:
                str = (gchar*)"font-size-larger" ;
                break ;
        case FONT_SIZE_SMALLER:
                str = (gchar*)"font-size-smaller" ;
                break ;
        default:
                str = (gchar*)"unknown relative font size value" ;
                break ;
        }
        return str ;
}

CRFontFamily *
cr_font_family_new (enum CRFontFamilyType a_type, guchar *a_name)
{
	CRFontFamily *result = NULL ;

	result = g_try_malloc (sizeof (CRFontFamily)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRFontFamily)) ;
	result->type = a_type ;

	cr_font_family_set_name (result, a_name) ;

	return result ;
}


guchar *
cr_font_family_to_string (CRFontFamily *a_this,
                          gboolean a_walk_font_family_list)
{
        enum CRStatus status = CR_OK ;
        guchar *result = NULL ;
        GString *stringue = NULL ;

        if (!a_this)
        {
                result = g_strdup ("NULL") ;
                g_return_val_if_fail (result, NULL) ;
                return result ;
        }        
        status = cr_font_family_to_string_real (a_this,
                                                a_walk_font_family_list,
                                                &stringue) ;

        if (status == CR_OK && stringue)
        {
                result = stringue->str ;
                g_string_free (stringue, FALSE) ;
                stringue = NULL ;
                
        }
        else
        {
                if (stringue)
                {
                        g_string_free (stringue, TRUE) ;
                        stringue = NULL ;
                }
        }

        return result ;
}
enum CRStatus
cr_font_family_set_name (CRFontFamily *a_this, guchar *a_name)
{
	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	/*
	 *only non generic font families can have a name
	 */

	if (a_this->type != FONT_FAMILY_NON_GENERIC)
	{
		return CR_BAD_PARAM_ERROR ;
	}

	if (a_this->name)
	{
		g_free (a_this->name) ;
		a_this->name = NULL ;
	}
       
	a_this->name = a_name ;
	return CR_OK ;
}


CRFontFamily *
cr_font_family_append (CRFontFamily *a_this, 
		       CRFontFamily *a_family_to_append)
{
	CRFontFamily *cur_ff = NULL ;

	g_return_val_if_fail (a_family_to_append,
			      NULL) ;

	if (!a_this)
		return a_family_to_append ;

	for (cur_ff = a_this ; 
	     cur_ff && cur_ff->next; 
	     cur_ff = cur_ff->next ) ;
	
	cur_ff->next = a_family_to_append;
	a_family_to_append->prev = cur_ff ;

	return a_this ;

	
}

CRFontFamily *
cr_font_family_prepend (CRFontFamily *a_this, 
			CRFontFamily *a_family_to_prepend)
{
	g_return_val_if_fail (a_this && a_family_to_prepend,
			      NULL) ;

	if (!a_this)
		return a_family_to_prepend ;

	a_family_to_prepend->next = a_this ;
	a_this->prev = a_family_to_prepend ;

	return CR_OK ;	
}


enum CRStatus
cr_font_family_destroy (CRFontFamily *a_this)
{
	CRFontFamily *cur_ff = NULL ;

	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	for (cur_ff = a_this ; 
	     cur_ff && cur_ff->next ;
	     cur_ff = cur_ff->next) 
		;

	for (; cur_ff ; cur_ff = cur_ff->prev)
	{
		if (a_this->name)
		{
			g_free (a_this->name) ;
			a_this->name = NULL ;
		}

		if (cur_ff->next)
		{
			g_free (cur_ff->next) ;
			
		}

		if (cur_ff->prev == NULL)
		{
			g_free (a_this) ;
		}
	}

	return CR_OK ;		
}

/***************************************************
 *'font-size' manipulation functions definitions
 ***************************************************/

CRFontSize *
cr_font_size_new (void)
{
        CRFontSize *result = NULL ;

        result = g_try_malloc (sizeof (CRFontSize)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRFontSize)) ;

        return result ;        
}

enum CRStatus
cr_font_size_clear (CRFontSize *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        switch (a_this->type)
        {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
        case RELATIVE_FONT_SIZE:
        case INHERITED_FONT_SIZE:
                memset (a_this, 0, sizeof (CRFontSize)) ;
                break ;

        case ABSOLUTE_FONT_SIZE:
                if (a_this->value.absolute)
                {
                        cr_num_destroy (a_this->value.absolute) ;
                }
                memset (a_this, 0, sizeof (CRFontSize)) ;
                break ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;
        }

        return CR_OK ;
}

enum CRStatus
cr_font_size_copy (CRFontSize *a_dst, CRFontSize *a_src)
{
        g_return_val_if_fail (a_dst && a_src, CR_BAD_PARAM_ERROR) ;

        switch (a_src->type)
        {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
        case RELATIVE_FONT_SIZE:
        case INHERITED_FONT_SIZE:
                cr_font_size_clear (a_dst) ;
                memcpy (a_dst, a_src, sizeof (CRFontSize)) ;
                break ;

        case ABSOLUTE_FONT_SIZE:                
                if (a_src->value.absolute)
                {
                        cr_font_size_clear (a_dst) ;
                        if (!a_dst->value.absolute)
                        {
                                a_dst->value.absolute = cr_num_new () ;
                        }
                        cr_num_copy (a_dst->value.absolute,
                                     a_src->value.absolute) ;
                        a_dst->type = a_src->type ;
                }
                break ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;
        }
        return CR_OK ;
}

gchar*
cr_font_size_to_string (CRFontSize *a_this)
{
        gchar *str = NULL ;
        
        if (!a_this)
        {
                str = g_strdup ("NULL") ;
                g_return_val_if_fail (str, NULL) ;
                return str ;
        }
        switch (a_this->type)
        {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
                str = g_strdup (cr_predefined_absolute_font_size_to_string
                                (a_this->value.predefined)) ;
                break ;
        case ABSOLUTE_FONT_SIZE:
                str = cr_num_to_string (a_this->value.absolute) ;
                break ;
        case RELATIVE_FONT_SIZE:
                str = g_strdup (cr_relative_font_size_to_string
                                (a_this->value.relative)) ;
                break ;
        case INHERITED_FONT_SIZE:
                str = g_strdup ("inherited") ;
                break ;
        default:
                break ;
        }
        return str ;
}


gchar *
cr_font_size_adjust_to_string (CRFontSizeAdjust *a_this)
{
        gchar *str = NULL ;

        if (!a_this)
        {
                str = g_strdup ("NULL") ;
                g_return_val_if_fail (str, NULL) ;
                return str ;
        }

        switch (a_this->type)
        {
        case FONT_SIZE_ADJUST_NONE:
                str = g_strdup ("font-size-adjust-none") ;
                break ;
        case FONT_SIZE_ADJUST_NUMBER:
                if (a_this->num)
                        str = cr_num_to_string (a_this->num) ;
                else
                        str = g_strdup ("font-size-adjust-NULL") ;
                break ;
        case FONT_SIZE_ADJUST_INHERIT:
                str = g_strdup ("font-size-adjust-inherit") ;
        }
        return str ;
}

const gchar *
cr_font_style_to_string (enum CRFontStyle a_code)
{
        gchar *str = NULL ;

        switch (a_code)
        {
        case FONT_STYLE_NORMAL:
                str = (gchar*)"font-style-normal" ;
                break ;
        case FONT_STYLE_ITALIC:
                str = (gchar*)"font-style-italic" ;
                break ;
        case FONT_STYLE_OBLIQUE:
                str = (gchar*)"font-style-oblique" ;
                break ;
        case FONT_STYLE_INHERIT:
                str = (gchar*)"font-style-inherit" ;
                break ;
        default:
                str = (gchar*)"font-style" ;
                break ;
        }
        return str ;
}

const gchar *
cr_font_variant_to_string (enum CRFontVariant a_code)
{
        gchar *str = NULL ;
        
        switch (a_code)
        {
        case FONT_VARIANT_NORMAL:
                str = (gchar*) "font-variant-normal";
                break ;
        case FONT_VARIANT_SMALL_CAPS:
                str = (gchar*) "font-variant-small-caps" ;
                break ;
        case FONT_VARIANT_INHERIT:
                str = (gchar*) "font-variant-inherent" ;
                break ;
        }
        return str ;
}

const gchar *
cr_font_weight_to_string (enum CRFontWeight a_code) 
{
        gchar *str = NULL ;
        
        switch (a_code)
        {
        case FONT_WEIGHT_NORMAL:
                str = (gchar*) "font-weight-normal";
                break ;
        case FONT_WEIGHT_BOLD:
                str = (gchar*) "font-weight-bold";
                break ;
        case FONT_WEIGHT_BOLDER:
                str = (gchar*) "font-weight-bolder";
                break ;
        case FONT_WEIGHT_LIGHTER:
                str = (gchar*) "font-weight-lighter";
                break ;
        case FONT_WEIGHT_100:
                str = (gchar*) "font-weight-100";
                break ;
        case FONT_WEIGHT_200:
                str = (gchar*) "font-weight-200";
                break ;
        case FONT_WEIGHT_300:
                str = (gchar*) "font-weight-300";
                break ;
        case FONT_WEIGHT_400:
                str = (gchar*) "font-weight-400";
                break ;
        case FONT_WEIGHT_500:
                str = (gchar*) "font-weight-500";
                break ;
        case FONT_WEIGHT_600:
                str = (gchar*) "font-weight-600";
                break ;
        case FONT_WEIGHT_700:
                str = (gchar*) "font-weight-700";
                break ;
        case FONT_WEIGHT_800:
                str = (gchar*) "font-weight-800";
                break ;
        case FONT_WEIGHT_900:
                str = (gchar*) "font-weight-900";
                break ;
        case FONT_WEIGHT_INHERIT:
                str = (gchar*) "font-weight-inherit";
                break ;
        default:
                str = (gchar*) "unknown font-weight property value";
                break ;
        }
        return str ;
}

const gchar *
cr_font_stretch_to_string (enum CRFontStretch a_code)
{
        gchar *str = NULL ;

        switch (a_code)
        {
        case FONT_STRETCH_NORMAL:
                str = (gchar*)"font-stretch-normal" ;
                break ;
        case FONT_STRETCH_WIDER:
                str = (gchar*)"font-stretch-wider" ;
                break ;
        case FONT_STRETCH_NARROWER:
                str = (gchar*)"font-stretch-narrower" ;
                break ;
        case FONT_STRETCH_ULTRA_CONDENSED:
                str = (gchar*)"font-stretch-ultra-condensed" ;
                break ;
        case FONT_STRETCH_EXTRA_CONDENSED:
                str = (gchar*)"font-stretch-extra-condensed" ;
                break ;
        case FONT_STRETCH_CONDENSED:
                str = (gchar*)"font-stretch-condensed" ;
                break ;
        case FONT_STRETCH_SEMI_CONDENSED:
                str = (gchar*)"font-stretch-semi-condensed" ;
                break ;
        case FONT_STRETCH_SEMI_EXPANDED:
                str = (gchar*)"font-stretch-semi-expanded" ;
                break ;
        case FONT_STRETCH_EXPANDED:
                str = (gchar*)"font-stretch-expanded" ;
                break ;
        case FONT_STRETCH_EXTRA_EXPANDED:
                str = (gchar*)"font-stretch-extra-expaned" ;
                break ;
        case FONT_STRETCH_ULTRA_EXPANDED:
                str = (gchar*)"font-stretch-ultra-expanded" ;
                break ;
        case FONT_STRETCH_INHERIT:
                str = (gchar*)"font-stretch-inherit" ;
                break ;
        }
        return str ;
}

void
cr_font_size_destroy (CRFontSize *a_font_size)
{
        g_return_if_fail (a_font_size) ;

        if (a_font_size->type == ABSOLUTE_FONT_SIZE
            && a_font_size->value.absolute)
        {
                cr_num_destroy (a_font_size->value.absolute) ;
                a_font_size->value.absolute = NULL ;
        }
}

/*******************************************************
 *'font-size-adjust' manipulation function definition
 *******************************************************/

CRFontSizeAdjust *
cr_font_size_adjust_new (void)
{
        CRFontSizeAdjust *result = NULL ;

        result = g_try_malloc (sizeof (CRFontSizeAdjust)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRFontSizeAdjust)) ;

        return result ;
}

void
cr_font_size_adjust_destroy (CRFontSizeAdjust *a_this)
{
        g_return_if_fail (a_this) ;

        if (a_this->type == FONT_SIZE_ADJUST_NUMBER
            && a_this->num)
        {
                cr_num_destroy (a_this->num) ;
                a_this->num = NULL ;
        }
}