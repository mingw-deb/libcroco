/* -*- Mode: C; indent-tabs-mode: ni; c-basic-offset: 8 -*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 *$Id$
 */

#include <string.h>
#include "cr-statement.h"


/**
 *@file
 *Definition of the #CRStatement class.
 */

#define DECLARATION_INDENT_NB 2

static void
cr_statement_clear (CRStatement *a_this)
{
	g_return_if_fail (a_this) ;

	switch (a_this->type)
	{
	case AT_RULE_STMT:
		break ;
	case RULESET_STMT:
		if (!a_this->kind.ruleset)
			return ;
		if (a_this->kind.ruleset->sel_list)
		{
			cr_selector_unref 
				(a_this->kind.ruleset->sel_list) ;
			a_this->kind.ruleset->sel_list = NULL ;
		}
		if (a_this->kind.ruleset->decl_list)
		{
			cr_declaration_destroy
				(a_this->kind.ruleset->decl_list) ;
			a_this->kind.ruleset->decl_list = NULL ;
		}
		g_free (a_this->kind.ruleset) ;
		a_this->kind.ruleset = NULL ;
		break ;

	case AT_IMPORT_RULE_STMT:
		if (!a_this->kind.import_rule)
			return ;
		if (a_this->kind.import_rule->url)
		{
			g_string_free 
				(a_this->kind.import_rule->url,
				 TRUE) ;
			a_this->kind.import_rule->url = NULL ;
		}
		g_free (a_this->kind.import_rule) ;
		a_this->kind.import_rule = NULL ;
		break ;

	case AT_MEDIA_RULE_STMT:
		if (!a_this->kind.media_rule)
			return;
		if (a_this->kind.media_rule->rulesets)
		{
			cr_statement_destroy 
				(a_this->kind.media_rule->rulesets) ;
			a_this->kind.media_rule->rulesets = NULL ;
		}
		if (a_this->kind.media_rule->media_list)
		{
			GList *cur = NULL ;
			
			for (cur = a_this->kind.media_rule->media_list;
			     cur ; cur = cur->next)
			{
				if (cur->data)
				{
					g_string_free ((GString*)cur->data,
						       TRUE) ;
					cur->data = NULL ;
				}
				
			}
			g_list_free 
				(a_this->kind.media_rule->media_list) ;
			a_this->kind.media_rule->media_list = NULL ;
		}
		g_free (a_this->kind.media_rule) ;
		a_this->kind.media_rule = NULL ;
		break ;

	case AT_PAGE_RULE_STMT:
		if (!a_this->kind.page_rule)
			return ;

		if (a_this->kind.page_rule->decls_list)
		{
			cr_declaration_destroy
				(a_this->kind.page_rule->decls_list) ;
			a_this->kind.page_rule->decls_list = NULL ;
		}
		if (a_this->kind.page_rule->name)
		{
			g_string_free (a_this->kind.page_rule->name,
				       TRUE) ;
			a_this->kind.page_rule->name = NULL ;
		}
		if (a_this->kind.page_rule->pseudo)
		{
			g_string_free (a_this->kind.page_rule->pseudo,
				       TRUE) ;
			a_this->kind.page_rule->pseudo = NULL ;
		}

		g_free (a_this->kind.page_rule) ;
		a_this->kind.page_rule = NULL ;
		break ;

	case AT_CHARSET_RULE_STMT:
		if (!a_this->kind.charset_rule)
			return ;

		if (a_this->kind.charset_rule->charset)
		{
			g_string_free 
				(a_this->kind.charset_rule->charset, 
				 TRUE) ;
			a_this->kind.charset_rule->charset = NULL ;
		}
		g_free (a_this->kind.charset_rule) ;
		a_this->kind.charset_rule = NULL;
		break ;

	case AT_FONT_FACE_RULE_STMT:
		if (!a_this->kind.font_face_rule)
			return ;

		if (a_this->kind.font_face_rule->decls_list)
		{
			cr_declaration_unref 
				(a_this->kind.font_face_rule->decls_list);
			a_this->kind.font_face_rule->decls_list = NULL ;
		}
		g_free (a_this->kind.font_face_rule) ;
		a_this->kind.font_face_rule = NULL ;
		break ;

	default:
		break ;
	}
}

/**
 *Dumps a ruleset statement to a file.
 *@param a_this the current instance of #CRStatement.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of indentation white spaces to add.
 */
static void
cr_statement_dump_ruleset (CRStatement *a_this, FILE *a_fp, glong a_indent)
{
	guchar *str = NULL, *tmp_str = NULL ;

	g_return_if_fail (a_this && a_this->type == RULESET_STMT) ;

	if (a_this->kind.ruleset->sel_list)
	{
		if (a_indent)
			cr_utils_dump_n_chars (' ', a_fp, a_indent) ;

		cr_selector_dump (a_this->kind.ruleset->sel_list, a_fp) ;
	}

	if (a_this->kind.ruleset->decl_list)
	{
		fprintf (a_fp," {\n") ;
		cr_declaration_dump (a_this->kind.ruleset->decl_list,
				     a_fp, a_indent + DECLARATION_INDENT_NB,
				     TRUE) ;
		fprintf (a_fp,"\n") ;
		cr_utils_dump_n_chars (' ', a_fp, a_indent) ;
		fprintf (a_fp,"}") ;
	}

	if (str)
	{
		g_free (str) ;
		str = NULL ;
	}
	if (tmp_str)
	{
		g_free (tmp_str) ;
		tmp_str = NULL ;
	}
}

/**
 *Dumps a font face rule statement to a file.
 *@param a_this the current instance of font face rule statement.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of white space indentation.
 */
static void
cr_statement_dump_font_face_rule (CRStatement *a_this , FILE *a_fp,
				  glong a_indent)
{
	g_return_if_fail (a_this 
			  && a_this->type == AT_FONT_FACE_RULE_STMT) ;
	
	if (a_this->kind.font_face_rule->decls_list)
	{
		cr_utils_dump_n_chars (' ', a_fp, a_indent) ;

		if (a_indent)
			cr_utils_dump_n_chars (' ', a_fp, a_indent) ;
		
		fprintf (a_fp,"@font-face {\n") ;
		cr_declaration_dump 
			(a_this->kind.font_face_rule->decls_list,
			 a_fp, a_indent + DECLARATION_INDENT_NB, TRUE) ;
		fprintf (a_fp,"\n}") ;
	}
}

/**
 *Dumps an @charset rule statement to a file.
 *@param a_this the current instance of the @charset rule statement.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of indentation white spaces.
 */
static void
cr_statement_dump_charset (CRStatement *a_this, FILE *a_fp,
			   gulong a_indent)
{
	guchar *str = NULL ;

	g_return_if_fail (a_this 
			  && a_this->type == AT_CHARSET_RULE_STMT) ;
	
	if (a_this->kind.charset_rule
	    && a_this->kind.charset_rule->charset)
	{
		str = g_strndup (a_this->kind.charset_rule->charset->str,
				 a_this->kind.charset_rule->charset->len) ;

		g_return_if_fail (str) ;

		cr_utils_dump_n_chars (' ', a_fp, a_indent) ;
		fprintf (a_fp,"@charset \"%s\" ;", str) ;
		if (str)
		{
			g_free (str) ;
			str = NULL; 
		}
	}
}

/**
 *Dumps an @page rule statement on stdout.
 *@param a_this the statement to dump on stdout.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of indentation white spaces.
 */
static void
cr_statement_dump_page (CRStatement *a_this, FILE *a_fp, gulong a_indent)
{
	guchar *str = NULL ;

	g_return_if_fail (a_this 
			  && a_this->type == AT_PAGE_RULE_STMT
			  && a_this->kind.page_rule) ;

	cr_utils_dump_n_chars (' ', a_fp, a_indent) ;
	fprintf (a_fp,"@page") ;

	if (a_this->kind.page_rule->name)
	{
		str = g_strndup (a_this->kind.page_rule->name->str,
				 a_this->kind.page_rule->name->len) ;
		g_return_if_fail (str) ;
		fprintf (a_fp," %s", str) ;
		if (str)
		{
			g_free (str) ;
			str = NULL ;
		}
	}
	else
	{
		fprintf (a_fp," ") ;
	}

	if (a_this->kind.page_rule->pseudo)
	{
		str = g_strndup (a_this->kind.page_rule->pseudo->str,
				 a_this->kind.page_rule->pseudo->len) ;
		g_return_if_fail (str) ;
		fprintf (a_fp,":%s", str) ;
		if (str)
		{
			g_free (str) ;
			str = NULL ;
		}
	}

	if (a_this->kind.page_rule->decls_list)
	{
		fprintf (a_fp," {\n") ;
		cr_declaration_dump 
			(a_this->kind.page_rule->decls_list,
			 a_fp, a_indent + DECLARATION_INDENT_NB, TRUE) ;
		fprintf (a_fp,"\n}\n") ;
	}
}

/**
 *Dumps an @media rule statement to a file.
 *@param a_this the statement to dump.
 *@param a_fp the destination file pointer
 *@param a_indent the number of white spaces indentation.
 */
static void
cr_statement_dump_media_rule (CRStatement *a_this, FILE *a_fp,
			      gulong a_indent)
{
	GList *cur = NULL ;

	g_return_if_fail (a_this->type == AT_MEDIA_RULE_STMT) ;
	
	if (a_this->kind.media_rule)
	{
		cr_utils_dump_n_chars (' ', a_fp, a_indent) ;
		fprintf (a_fp,"@media") ;
		for (cur = a_this->kind.media_rule->media_list ; cur ;
			cur = cur->next)
		{
			if (cur->data)
			{
				guchar *str = g_strndup 
					(((GString*)cur->data)->str,
					 ((GString*)cur->data)->len) ;
				if (str)
				{
					if (cur->prev)
					{
						fprintf (a_fp,",") ;
					}
					fprintf (a_fp," %s", str) ;
					g_free (str) ; str = NULL ;
				}
			}
		}
		fprintf (a_fp," {\n") ;
		cr_statement_dump (a_this->kind.media_rule->rulesets, 
				   a_fp, a_indent + DECLARATION_INDENT_NB) ;
		fprintf (a_fp,"\n}") ;
	}
}

/**
 *Dumps an @import rule statement to a file.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of white space indentations.
 */
static void
cr_statement_dump_import_rule (CRStatement *a_this, FILE *a_fp,
			       gulong a_indent)
{
	g_return_if_fail (a_this 
			  && a_this->type == AT_IMPORT_RULE_STMT
			  && a_this->kind.import_rule) ;

	if (a_this->kind.import_rule->url)
	{
		guchar *str = NULL ;

		str = g_strndup (a_this->kind.import_rule->url->str,
				 a_this->kind.import_rule->url->len) ;
		cr_utils_dump_n_chars (' ', a_fp, a_indent) ;

		if (str)
		{
			fprintf (a_fp,"@import url(\"%s\")", str) ;
			g_free (str) ;
		}
		else /*there is no url, so no import rule, get out!*/
			return ;

		if (a_this->kind.import_rule->media_list)
		{
			GList *cur = NULL ;

			for (cur = a_this->kind.import_rule->media_list ;
			     cur; cur = cur->next)
			{
				if (cur->data)
				{
					GString *gstr = cur->data ;

					if (cur->prev)
					{
						fprintf (a_fp,", ") ;
					}

					str = g_strndup (gstr->str,
							 gstr->len) ;
					if (str)
					{
						fprintf (a_fp,str) ;
						g_free (str) ;
					}
				}
			}
			fprintf (a_fp," ;") ;
		}
	}
}


/**
 *Creates a new instance of #CRStatement of type
 *#CRRulSet.
 *@param a_sel_list the list of #CRSimpleSel (selectors)
 *the rule applies to.
 *@param a_decl_list the list of instances of #CRDeclaration
 *that composes the ruleset.
 *@param a_media_types a list of instances of GString that
 *describe the media list this ruleset applies to.
 *@return the new instance of #CRStatement or NULL if something
 *went wrong.
 */
CRStatement*
cr_statement_new_ruleset (CRStyleSheet * a_sheet,
			  CRSelector *a_sel_list, 
			  CRDeclaration *a_decl_list,
			  CRAtMediaRule *a_media_rule)
{
	CRStatement *result = NULL ;

	g_return_val_if_fail (a_sheet, NULL) ;

	result = g_try_malloc (sizeof (CRStatement)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRStatement)) ;
	result->type = RULESET_STMT ;
	result->kind.ruleset = g_try_malloc (sizeof (CRRuleSet)) ;

	if (!result->kind.ruleset)
	{
		cr_utils_trace_info ("Out of memory") ;
		if (result) g_free (result) ;
		return NULL ;
	}

	memset (result->kind.ruleset, 0, sizeof (CRRuleSet)) ;
	result->kind.ruleset->sel_list = a_sel_list;
	if (a_sel_list)
		cr_selector_ref (a_sel_list) ;
	result->kind.ruleset->decl_list = a_decl_list;
	result->kind.ruleset->media_rule = a_media_rule;

	cr_statement_set_parent_sheet (result, a_sheet) ;

	return result ;
}

/**
 *Instanciates an instance of #CRStatement of type
 *AT_MEDIA_RULE_STMT (@media ruleset).
 *@param a_ruleset the ruleset statements contained
 *in the @media rule.
 *@param a_media, the media string list. A list of GString pointers.
 */
CRStatement *
cr_statement_new_at_media_rule (CRStyleSheet *a_sheet,
				CRStatement *a_rulesets,
				GList *a_media)
{
	CRStatement *result = NULL ;

	g_return_val_if_fail (a_sheet, NULL) ;

	if (a_rulesets)
		g_return_val_if_fail (a_rulesets->type == RULESET_STMT,
				      NULL) ;

	result = g_try_malloc (sizeof (CRStatement)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRStatement)) ;
	result->type = AT_MEDIA_RULE_STMT ;
	
	result->kind.media_rule = g_try_malloc (sizeof (CRAtMediaRule)) ;
	if (!result->kind.media_rule)
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		return NULL ;
	}
	memset (result->kind.media_rule, 0, sizeof (CRAtMediaRule)) ;
	result->kind.media_rule->rulesets = a_rulesets ;
	result->kind.media_rule->media_list = a_media ;
	cr_statement_set_parent_sheet (result, a_sheet) ;

	return result ;
}


/**
 *Creates a new instance of #CRStatment of type
 *#CRAtImportRule.
 *@param a_url the url to connect to the get the file
 *to be imported.
 *@param a_sheet the imported parsed stylesheet.
 *@return the newly built instance of #CRStatement.
 */
CRStatement*
cr_statement_new_at_import_rule (CRStyleSheet *a_container_sheet,
				 GString *a_url,
				 GList *a_media_list,
				 CRStyleSheet *a_imported_sheet)
{
	CRStatement *result = NULL ;

	g_return_val_if_fail (a_container_sheet, NULL) ;

	result = g_try_malloc (sizeof (CRStatement)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRStatement)) ;
	result->type = AT_IMPORT_RULE_STMT ;

	result->kind.import_rule = 
		g_try_malloc (sizeof (CRAtImportRule)) ;

	if (!result->kind.import_rule)
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		return NULL ;
	}

	memset (result->kind.import_rule, 0, sizeof (CRAtImportRule)) ;
	result->kind.import_rule->url = a_url;
	result->kind.import_rule->media_list = a_media_list ;
	result->kind.import_rule->sheet = a_imported_sheet;
	cr_statement_set_parent_sheet (result, a_container_sheet) ;

	return result ;
}

/**
 *Creates a new instance of #CRStatement of type
 *#CRAtPageRule.
 *@param a_decl_list a list of instances of #CRDeclarations
 *which is actually the list of declarations that applies to
 *this page rule.
 *@param a_selector the page rule selector.
 *@return the newly built instance of #CRStatement or NULL
 *in case of error.
 */
CRStatement *
cr_statement_new_at_page_rule (CRStyleSheet *a_sheet,
	                       CRDeclaration *a_decl_list,
			       GString *a_name,
			       GString *a_pseudo)
{
	CRStatement *result = NULL ;

	g_return_val_if_fail (a_sheet, NULL) ;

	result = g_try_malloc (sizeof (CRStatement)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRStatement)) ;
	result->type = AT_PAGE_RULE_STMT ;

	result->kind.page_rule = g_try_malloc (sizeof (CRAtPageRule)) ;

	if (!result->kind.page_rule)
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		return NULL ;
	}

	memset (result->kind.page_rule, 0, sizeof (CRAtPageRule)) ;
	result->kind.page_rule->decls_list = a_decl_list;
	result->kind.page_rule->name = a_name ;
	result->kind.page_rule->name = a_pseudo ;
	cr_statement_set_parent_sheet (result, a_sheet) ;

	return result ;
}

/**
 *Creates a new instance of #CRStatement of type
 *#CRAtCharsetRule.
 *@param a_charset the string representing the charset.
 *@return the newly built instance of #CRStatement or NULL
 *if an error arises.
 */
CRStatement *
cr_statement_new_at_charset_rule (CRStyleSheet *a_sheet,
				  GString *a_charset)
{
	CRStatement * result = NULL ;

	g_return_val_if_fail (a_sheet, NULL) ;

	result = g_try_malloc (sizeof (CRStatement)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRStatement)) ;
	result->type = AT_CHARSET_RULE_STMT ;

	result->kind.charset_rule = g_try_malloc 
		(sizeof (CRAtCharsetRule)) ;

	if (!result->kind.charset_rule)
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		return NULL ;
	}
	memset (result->kind.charset_rule, 0, sizeof (CRAtCharsetRule)) ;
	result->kind.charset_rule->charset = a_charset ;
	cr_statement_set_parent_sheet (result, a_sheet) ;

	return result ;
}

/**
 *Creates an instance of #CRStatement of type #CRAtFontFaceRule.
 *@param a_font_decls a list of instances of #CRDeclaration. Each declaration
 *is actually a font declaration.
 *@return the newly built instance of #CRStatement.
 */
CRStatement *
cr_statement_new_at_font_face_rule (CRStyleSheet *a_sheet,
				    CRDeclaration *a_font_decls)
{
	CRStatement *result = NULL ;
	
	g_return_val_if_fail (a_sheet, NULL) ;

	result = g_try_malloc (sizeof (CRStatement)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRStatement)) ;
	result->type = AT_FONT_FACE_RULE_STMT ;

	result->kind.font_face_rule = g_try_malloc 
		(sizeof (CRAtFontFaceRule)) ;

	if (!result->kind.font_face_rule)
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		return NULL ;
	}
	memset (result->kind.font_face_rule, 0, 
		sizeof (CRAtFontFaceRule));
	
	result->kind.font_face_rule->decls_list = a_font_decls ;
	cr_statement_set_parent_sheet (result, a_sheet) ;

	return result ;
}

/**
 *Sets the container stylesheet.
 *@param a_this the current instance of #CRStatement.
 *@param a_sheet the sheet that contains the current statement.
 *@return CR_OK upon successfull completion, an errror code otherwise.
 */
enum CRStatus
cr_statement_set_parent_sheet (CRStatement *a_this, 
			       CRStyleSheet *a_sheet)
{
	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;
	a_this->parent_sheet = a_sheet ;
	return CR_OK ;
}

/**
 *Gets the sheets that contains the current statement.
 *@param a_this the current #CRStatement.
 *@param a_sheet out parameter. A pointer to the sheets that
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_get_parent_sheet (CRStatement *a_this, 
			       CRStyleSheet **a_sheet)
{
	g_return_val_if_fail (a_this && a_sheet, 
			      CR_BAD_PARAM_ERROR) ;
	*a_sheet  = a_this->parent_sheet ;
	return CR_OK ;
}

/**
 *Appends a new statement to the statement list.
 *@param a_this the current instance of the statement list.
 *@param a_this a_new the new instance of #CRStatement to append.
 *@return the new list statement list, or NULL in cas of failure.
 */
CRStatement *
cr_statement_append (CRStatement *a_this,
		     CRStatement *a_new)
{
	CRStatement * cur = NULL ;

	g_return_val_if_fail (a_new, NULL) ;
	
	if (!a_this)
	{
		return a_new ;
	}

	/*walk forward in the current list to find the tail list element*/
	for (cur = a_this ; cur && cur->next ; cur = cur->next) ;

	cur->next = a_new ;
	a_new->prev = cur ;

	return a_this ;
}

/**
 *Prepends the an instance of #CRStatement to
 *the current statement list.
 *@param a_this the current instance of #CRStatement.
 *@param a_new the new statement to prepend.
 *@return the new list with the new statement prepended,
 *or NULL in case of an error.
 */
CRStatement*
cr_statement_prepend (CRStatement *a_this,
		      CRStatement *a_new)
{
	CRStatement *cur = NULL ;

	g_return_val_if_fail (a_new, NULL) ;

	if (!a_this)
		return a_new ;

	a_new->next = a_this ;
	a_this->prev = a_new ;

	/*walk backward in the prepended list to find the head list element*/
	for (cur = a_new ; cur && cur->prev ; cur = cur->prev) ;
	
	return cur ;
}

/**
 *Unlinks a statement from the statements list.
 *@param a_this the current statements list.
 *@param a_to_unlink the statement to unlink from the list.
 *@return the new list where a_to_unlink has been unlinked
 *from, or NULL in case of error.
 */
CRStatement *
cr_statement_unlink (CRStatement *a_this,
		     CRStatement *a_to_unlink)
{
	CRStatement *cur = NULL, *next = NULL, *prev = NULL ;

	g_return_val_if_fail (a_this && a_to_unlink, NULL) ;

	/*make sure a_to_unlink belongs to the list headed by a_this*/
	for (cur = a_this ; cur && (cur != a_to_unlink) ; cur = cur->next) ;
	
	if (!cur) return NULL ;

	next = a_to_unlink->next ;
	prev = a_to_unlink->prev ;
	
	if (prev)
		prev->next = next ;
	
	if (next)
		next->prev = prev ;

	return a_to_unlink ;
}


/**
 *Sets a selector list to a ruleset statement.
 *@param a_this the current ruleset statement.
 *@param a_sel_list the selector list to set. Note
 *that this function increments the ref count of a_sel_list.
 *The sel list will be destroyed at the destruction of the
 *current instance of #CRStatement.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_ruleset_set_sel_list (CRStatement *a_this,
				   CRSelector *a_sel_list)
{
	g_return_val_if_fail (a_this && a_this->type == RULESET_STMT,
			      CR_BAD_PARAM_ERROR) ;
	
	if (a_this->kind.ruleset->sel_list)
		cr_selector_unref (a_this->kind.ruleset->sel_list) ;
	
	a_this->kind.ruleset->sel_list = a_sel_list ;

	if (a_sel_list)
		cr_selector_ref (a_sel_list) ;

	return CR_OK ;	
}

/**
 *Gets a pointer to the selector list contained in
 *the current ruleset statement.
 *@param a_this the current ruleset statement.
 *@param a_list out parameter. The returned selector list,
 *if and only if the function returned CR_OK.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_ruleset_get_sel_list (CRStatement *a_this,
				   CRSelector **a_list)
{
	g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
			      && a_this->kind.ruleset, CR_BAD_PARAM_ERROR) ;
	
	*a_list = a_this->kind.ruleset->sel_list ;

	return CR_OK ;
}


/**
 *Sets a declaration list to the current ruleset statement.
 *@param a_this the current ruleset statement.
 *@param a_list the declaration list to be added to the current
 *ruleset statement.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_ruleset_set_decl_list (CRStatement *a_this,
				    CRDeclaration *a_list)
{
	g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
			      && a_this->kind.ruleset, CR_BAD_PARAM_ERROR);

	if (a_this->kind.ruleset->decl_list == a_list)
		return CR_OK ;

	if (a_this->kind.ruleset->sel_list)
	{
		cr_declaration_destroy (a_this->kind.ruleset->decl_list) ;
	}

	a_this->kind.ruleset->sel_list = NULL;

	return CR_OK ;
}


/**
 *Appends a declaration to the current ruleset statement.
 *@param a_this the current statement.
 *@param a_prop the property of the declaration.
 *@param a_value the value of the declaration.
 *@return CR_OK uppon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_statement_ruleset_append_decl2 (CRStatement *a_this,
				   GString *a_prop, CRTerm *a_value)
{
	CRDeclaration * new_decls = NULL ;

	g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
			      && a_this->kind.ruleset, CR_BAD_PARAM_ERROR) ;

	new_decls = cr_declaration_append2 
		(a_this->kind.ruleset->decl_list, a_prop, a_value) ;
	g_return_val_if_fail (new_decls, CR_ERROR) ;
	a_this->kind.ruleset->decl_list = new_decls ;

	return CR_OK ;
}

/**
 *Appends a declaration to the current statement.
 *@param a_this the current statement.
 *@param a_declaration the declaration to append.
 *@return CR_OK upon sucessfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_statement_ruleset_append_decl (CRStatement *a_this,
				  CRDeclaration *a_decl)
{
	CRDeclaration * new_decls = NULL ;

	g_return_val_if_fail (a_this && a_this->type == RULESET_STMT
			      && a_this->kind.ruleset, CR_BAD_PARAM_ERROR) ;

	new_decls = cr_declaration_append
		(a_this->kind.ruleset->decl_list, a_decl) ;
	g_return_val_if_fail (new_decls, CR_ERROR) ;
	a_this->kind.ruleset->decl_list = new_decls ;

	return CR_OK ;
}


/**
 *Sets a stylesheet to the current @import rule.
 *@param a_this the current @import rule.
 *@param a_sheet the stylesheet. The stylesheet is owned
 *by the current instance of #CRStatement, that is, the 
 *stylesheet will be destroyed when the current instance
 *of #CRStatement will be destroyed.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_set_imported_sheet (CRStatement *a_this,
						CRStyleSheet *a_sheet)
{
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_IMPORT_RULE_STMT
			      && a_this->kind.import_rule,
			      CR_BAD_PARAM_ERROR) ;

	a_this->kind.import_rule->sheet = a_sheet ;

	return CR_OK ;
}


/**
 *Gets the stylesheet contained by the @import rule statement.
 *@param a_this the current @import rule statement.
 *@param a_sheet out parameter. The returned stylesheet if and
 *only if the function returns CR_OK.
 *@return CR_OK upon sucessfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_get_imported_sheet (CRStatement *a_this,
						CRStyleSheet **a_sheet)
{
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_IMPORT_RULE_STMT
			      && a_this->kind.import_rule,
			      CR_BAD_PARAM_ERROR) ;
	*a_sheet = a_this->kind.import_rule->sheet ;
	return CR_OK ;

}

/**
 *Sets an url to the current @import rule statement.
 *@param a_this the current @import rule statement.
 *@param a_url the url to set.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_import_rule_set_url (CRStatement *a_this,
				     GString *a_url)
{
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_IMPORT_RULE_STMT
			      && a_this->kind.import_rule,
			      CR_BAD_PARAM_ERROR) ;

	if (a_this->kind.import_rule->url)
	{
		g_string_free (a_this->kind.import_rule->url, TRUE) ;
	}
	
	a_this->kind.import_rule->url = a_url ;

	return CR_OK ;
}


/**
 *Gets the url of the @import rule statement.
 *@param the current @import rule statement.
 *@param a_url out parameter. The returned url if
 *and only if the function returned CR_OK.
 */
enum CRStatus
cr_statement_at_import_rule_get_url (CRStatement *a_this,
				     GString **a_url)
{
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_IMPORT_RULE_STMT
			      && a_this->kind.import_rule,
			      CR_BAD_PARAM_ERROR) ;

	*a_url = a_this->kind.import_rule->url ;

	return CR_OK ;
}

/**
 *Sets a declaration list to the current @page rule statement.
 *@param a_this the current @page rule statement.
 *@param a_decl_list the declaration list to add. Will be freed
 *by the current instance of #CRStatement when it is destroyed.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_page_rule_set_declarations (CRStatement *a_this,
					    CRDeclaration *a_decl_list)
{
	g_return_val_if_fail (a_this
			      && a_this->type == AT_PAGE_RULE_STMT
			      && a_this->kind.page_rule,
			      CR_BAD_PARAM_ERROR) ;

	if (a_this->kind.page_rule->decls_list)
	{
		cr_declaration_unref (a_this->kind.page_rule->decls_list);
	}

	a_this->kind.page_rule->decls_list = a_decl_list ;

	if (a_decl_list)
	{
		cr_declaration_ref (a_decl_list) ;
	}

	return CR_OK ;
}


/**
 *Gets the declaration list associated to the current @page rule
 *statement.
 *@param a_this the current  @page rule statement.
 *@param a_decl_list out parameter. The returned declaration list.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_page_rule_get_declarations (CRStatement *a_this,
					    CRDeclaration **a_decl_list)
{
	g_return_val_if_fail (a_this
			      && a_this->type == AT_PAGE_RULE_STMT
			      && a_this->kind.page_rule,
			      CR_BAD_PARAM_ERROR) ;

	*a_decl_list = a_this->kind.page_rule->decls_list ;

	return CR_OK ;
}


/**
 *Sets the charset of the current @charset rule statement.
 *@param a_this the current @charset rule statement.
 *@param a_charset the charset to set.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_charset_rule_set_charset (CRStatement *a_this,
					  GString *a_charset)
{
	g_return_val_if_fail (a_this
			      && a_this->type == AT_CHARSET_RULE_STMT
			      && a_this->kind.charset_rule,
			      CR_BAD_PARAM_ERROR) ;

	if (a_this->kind.charset_rule->charset)
	{
		g_string_free (a_this->kind.charset_rule->charset,
			       TRUE) ;
	}

	a_this->kind.charset_rule->charset = a_charset ;
	return CR_OK ;
}


/**
 *Gets the charset string associated to the current
 *@charset rule statement.
 *@param a_this the current @charset rule statement.
 *@param a_charset out parameter. The returned charset string if
 *and only if the function returned CR_OK.
 */
enum CRStatus
cr_statement_at_charset_rule_get_charset (CRStatement *a_this,
					  GString **a_charset)
{
	g_return_val_if_fail (a_this
			      && a_this->type == AT_CHARSET_RULE_STMT
			      && a_this->kind.charset_rule,
			      CR_BAD_PARAM_ERROR) ;

	*a_charset = a_this->kind.charset_rule->charset ;

	return CR_OK ;
}


/**
 *Sets a declaration list to the current @font-face rule statement.
 *@param a_this the current @font-face rule statement.
 *@param a_decls the declarations list to set.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_font_face_rule_set_decls (CRStatement *a_this,
					  CRDeclaration *a_decls)
{
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_FONT_FACE_RULE_STMT
			      && a_this->kind.font_face_rule,
			      CR_BAD_PARAM_ERROR) ;

	if (a_this->kind.font_face_rule->decls_list)
	{
		cr_declaration_unref 
			(a_this->kind.font_face_rule->decls_list) ;
	}

	a_this->kind.font_face_rule->decls_list = a_decls;
	cr_declaration_ref (a_decls) ;
	
	return CR_OK ;
}


/**
 *Gets the declaration list associated to the current instance
 *of @font-face rule statement.
 *@param a_this the current @font-face rule statement.
 *@param a_decls out parameter. The returned declaration list if
 *and only if this function returns CR_OK.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_font_face_rule_get_decls (CRStatement *a_this,
					  CRDeclaration **a_decls)
{
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_FONT_FACE_RULE_STMT
			      && a_this->kind.font_face_rule,
			      CR_BAD_PARAM_ERROR) ;

	*a_decls = a_this->kind.font_face_rule->decls_list ;
	
	return CR_OK ;
}


/**
 *Adds a declaration to the current @font-face rule
 *statement.
 *@param a_this the current @font-face rule statement.
 *@param a_prop the property of the declaration.
 *@param a_value the value of the declaration.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_statement_at_font_face_rule_add_decl (CRStatement *a_this,
					 GString *a_prop,
					 CRTerm *a_value)
{
	CRDeclaration *decls = NULL ;
	
	g_return_val_if_fail (a_this 
			      && a_this->type == AT_FONT_FACE_RULE_STMT
			      && a_this->kind.font_face_rule,
			      CR_BAD_PARAM_ERROR) ;

	decls = cr_declaration_append2 
		(a_this->kind.font_face_rule->decls_list,
		 a_prop, a_value) ;

	g_return_val_if_fail (decls, CR_ERROR) ;
		
	if (a_this->kind.font_face_rule->decls_list == NULL)
		cr_declaration_ref (decls) ;

	a_this->kind.font_face_rule->decls_list = decls ;

	return CR_OK ;
}

/**
 *Dumps the css2 statement to a file.
 *@param a_this the current css2 statement.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of white space indentation characters.
 */
void
cr_statement_dump (CRStatement *a_this, FILE *a_fp, gulong a_indent)
{
	g_return_if_fail (a_this) ;

	if (a_this->prev)
	{
		fprintf (a_fp,"\n\n") ;
	}

	switch (a_this->type)
	{
	case RULESET_STMT:
		cr_statement_dump_ruleset (a_this, a_fp, a_indent) ;
		break ;

	case AT_FONT_FACE_RULE_STMT:
		cr_statement_dump_font_face_rule 
			(a_this, a_fp, a_indent);
		break ;

	case AT_CHARSET_RULE_STMT:
		cr_statement_dump_charset (a_this, a_fp, a_indent) ;
		break ;

	case AT_PAGE_RULE_STMT:
		cr_statement_dump_page (a_this, a_fp, a_indent) ;
		break ;

	case AT_MEDIA_RULE_STMT:
		cr_statement_dump_media_rule (a_this, a_fp, a_indent) ;
		break ;

	case AT_IMPORT_RULE_STMT:
		cr_statement_dump_import_rule (a_this, a_fp, a_indent) ;
		break ;

	default :
		fprintf (a_fp, "Statement unrecognized at %s:%d", 
			 __FILE__, __LINE__) ;
		break ;
	}
}

/**
 *Destructor of #CRStatement.
 */
void
cr_statement_destroy (CRStatement *a_this)
{
	CRStatement *cur = NULL ;

	g_return_if_fail (a_this) ;
	
	/*go get the tail of the list*/
	for (cur = a_this ; cur && cur->next; cur = cur->next)
	{
		cr_statement_clear (cur) ;
	}
	
	if (cur)
		cr_statement_clear (cur) ;

	if (cur->prev == NULL)
	{
		g_free (a_this);
		return ;
	}

	/*walk backward and free next element*/
	for (cur = cur->prev ; cur && cur->prev; cur = cur->prev)
	{
		if (cur->next)
		{
			g_free (cur->next) ;
			cur->next = NULL ;
		}		
	}

	if (!cur)
		return ;

	/*free the one remaining list*/
	if (cur->next)
	{
		g_free (cur->next) ;		
		cur->next = NULL ;
	}

	g_free (cur) ;
	cur = NULL ;
}