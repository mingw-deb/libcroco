/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *This file is part of the Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *The Croco Library is distributed in the hope 
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

#include <stdio.h>
#include "cr-utils.h"
#include "cr-term.h"
#include "cr-selector.h"
#include "cr-declaration.h"

#ifndef __CR_STATEMENT_H__
#define __CR_STATEMENT_H__

G_BEGIN_DECLS

/**
 *@file
 *Declaration of the #CRStatement class.
 */

struct _CRStatement ;
typedef struct _CRStatement CRStatement ;

struct _CRAtMediaRule ;
typedef struct _CRAtMediaRule CRAtMediaRule ;

typedef struct _CRRuleSet CRRuleSet ;
/**
 *The abstraction of a css ruleset.
 *A ruleset is made of a list of selectors,
 *followed by a list of declarations.
 */
struct _CRRuleSet
{
	/**A list of instances of #CRSimpeSel*/
	CRSelector *sel_list ;

	/**A list of instances of #CRDeclaration*/
	CRDeclaration *decl_list ;
	
	/**
	 *The parent media rule, or NULL if
	 *no parent media rule exists.
	 */
	CRAtMediaRule *media_rule ;
} ;

/*
 *a forward declaration of CRStylesheet.
 *CRStylesheet is actually declared in
 *cr-stylesheet.h
 */
struct _CRStyleSheet ;
typedef struct _CRStyleSheet CRStyleSheet;


/**The @import rule abstraction.*/
typedef struct _CRAtImportRule CRAtImportRule ;
struct _CRAtImportRule
{
	/**the url of the import rule*/
	GString *url ;

        GList *media_list ;

	/**
	 *the stylesheet fetched from the url, if any.
	 *this is not "owned" by #CRAtImportRule which means
	 *it is not destroyed by the destructor of #CRAtImportRule.
	 */
	CRStyleSheet * sheet;
};


/**abstraction of an @media rule*/
struct _CRAtMediaRule
{
	GList *media_list ;
	CRStatement *rulesets ;	
} ;


typedef struct _CRAtPageRule CRAtPageRule ;
/**The @page rule abstraction*/
struct _CRAtPageRule
{
	/**a list of instances of #CRDeclaration*/
	CRDeclaration *decls_list ;

	/**page selector. Is a pseudo selector*/
	GString *name ;
	GString *pseudo ;
} ;

/**The @charset rule abstraction*/
typedef struct _CRAtCharsetRule CRAtCharsetRule ;
struct _CRAtCharsetRule
{
	GString * charset ;
};

/**The abstaction of the @font-face rule.*/
typedef struct _CRAtFontFaceRule CRAtFontFaceRule ;
struct _CRAtFontFaceRule
{
	/*a list of instanaces of #CRDeclaration*/
	CRDeclaration *decls_list ;
} ;


/**
 *The possible types of css2 statements.
 */
enum CRStatementType
{
	/**
	 *A generic css at-rule
	 *each unknown at-rule will
	 *be of this type.
	 */
	AT_RULE_STMT = 0,

	/**A css at-rule*/
	RULESET_STMT,

	/**A css2 import rule*/
	AT_IMPORT_RULE_STMT,

	/**A css2 media rule*/
	AT_MEDIA_RULE_STMT,

	/**A css2 page rule*/
	AT_PAGE_RULE_STMT,

	/**A css2 charset rule*/
	AT_CHARSET_RULE_STMT,

	/**A css2 font face rule*/
	AT_FONT_FACE_RULE_STMT	
} ;


/**
 *The abstraction of css statement as defined
 *in the chapter 4 and appendix D.1 of the css2 spec.
 *A statement is actually a double chained list of
 *statements.A statement can be a ruleset, an @import
 *rule, an @page rule etc ...
 */
struct _CRStatement
{
	/**
	 *The type of the statement.
	 */
	enum CRStatementType type ;

	union
	{
		CRRuleSet *ruleset ;
		CRAtImportRule *import_rule ;
		CRAtMediaRule *media_rule ;
		CRAtPageRule *page_rule ;
		CRAtCharsetRule *charset_rule ;
		CRAtFontFaceRule *font_face_rule ;
	} kind ;
	
	CRStatement *next ;
	CRStatement *prev ;
} ;


CRStatement*
cr_statement_new_ruleset (CRSelector *a_sel_list, 
			  CRDeclaration *a_decl_list,
			  CRAtMediaRule *a_media_rule) ;

CRStatement*
cr_statement_new_at_import_rule (GString *a_url,
                                 GList *a_media_list,
				 CRStyleSheet *a_sheet) ;

CRStatement *
cr_statement_new_at_media_rule (CRStatement *a_ruleset,
				GList *a_media) ;

CRStatement *
cr_statement_new_at_charset_rule (GString *a_charset) ;

CRStatement *
cr_statement_new_at_font_face_rule (CRDeclaration *a_font_decls) ;

CRStatement *
cr_statement_new_at_page_rule (CRDeclaration *a_decl_list,
			       GString *a_name,
			       GString *a_pseudo) ;
CRStatement *
cr_statement_append (CRStatement *a_this,
		     CRStatement *a_new) ;

CRStatement*
cr_statement_prepend (CRStatement *a_this,
		      CRStatement *a_new) ;

CRStatement *
cr_statement_unlink (CRStatement *a_this,
		     CRStatement *a_to_unlink) ;

enum CRStatus
cr_statement_ruleset_set_sel_list (CRStatement *a_this,
				   CRSelector *a_sel_list) ;

enum CRStatus
cr_statement_ruleset_get_sel_list (CRStatement *a_this,
				   CRSelector **a_list) ;

enum CRStatus
cr_statement_ruleset_set_decl_list (CRStatement *a_this,
				    CRDeclaration *a_list) ;

enum CRStatus
cr_statement_ruleset_append_decl2 (CRStatement *a_this,
				   GString *a_prop, CRTerm *a_value) ;

enum CRStatus
cr_statement_ruleset_append_decl (CRStatement *a_this,
				  CRDeclaration *a_decl) ;

enum CRStatus
cr_statement_at_import_rule_set_sheet (CRStatement *a_this,
				       CRStyleSheet *a_sheet) ;

enum CRStatus
cr_statement_at_import_rule_get_sheet (CRStatement *a_this,
				       CRStyleSheet **a_sheet) ;

enum CRStatus
cr_statement_at_import_rule_set_url (CRStatement *a_this,
				     GString *a_url) ;

enum CRStatus
cr_statement_at_import_rule_get_url (CRStatement *a_this,
				     GString **a_url) ;

enum CRStatus
cr_statement_at_page_rule_set_sel (CRStatement *a_this,
				   CRSelector *a_sel) ;

enum CRStatus
cr_statement_at_page_rule_get_sel (CRStatement *a_this,
				   CRSelector **a_sel) ;

enum CRStatus
cr_statement_at_page_rule_set_declarations (CRStatement *a_this,
					    CRDeclaration *a_decl_list) ;

enum CRStatus
cr_statement_at_page_rule_get_declarations (CRStatement *a_this,
					    CRDeclaration **a_decl_list) ;

enum CRStatus
cr_statement_at_charset_rule_set_charset (CRStatement *a_this,
					  GString *a_charset) ;

enum CRStatus
cr_statement_at_charset_rule_get_charset (CRStatement *a_this,
					  GString **a_charset) ;

enum CRStatus
cr_statement_at_font_face_rule_set_decls (CRStatement *a_this,
					  CRDeclaration *a_decls) ;

enum CRStatus
cr_statement_at_font_face_rule_get_decls (CRStatement *a_this,
					  CRDeclaration **a_decls) ;

enum CRStatus
cr_statement_at_font_face_rule_add_decl (CRStatement *a_this,
					 GString *a_prop,
					 CRTerm *a_value) ;

void
cr_statement_dump (CRStatement *a_this, FILE *a_fp, gulong a_indent) ;

void
cr_statement_destroy (CRStatement *a_this) ;

G_END_DECLS

#endif /*__CR_STATEMENT_H__*/
