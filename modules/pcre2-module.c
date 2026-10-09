/* -*- mode: C; mode: fold -*-
Copyright (C) 2010-2021,2022 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/
#include "config.h"

#include <stdio.h>
#include <slang.h>
#include <string.h>

#define PCRE2_CODE_UNIT_WIDTH 8	       /* unicode */
#include <pcre2.h>

SLANG_MODULE(pcre2);

static int PCRE2_Type_Id = 0;

typedef struct
{
   pcre2_code *p;
   pcre2_match_data *match_data;
   unsigned int num_matches;       /* return value of pcre2_exec (>= 1)*/
}
PCRE2_Type;

static pcre2_general_context *General_Context = NULL;
static pcre2_compile_context *Compile_Context = NULL;

static void free_pcre2_type (PCRE2_Type *pt)
{
   if (pt == NULL) return;

   if (pt->match_data != NULL)
     pcre2_match_data_free(pt->match_data);

   if (pt->p != NULL)
     pcre2_code_free (pt->p);

   SLfree ((char *) pt);
}

/* checked */
static SLang_MMT_Type *allocate_pcre_type (pcre2_code *p)
{
   PCRE2_Type *pt;
   SLang_MMT_Type *mmt;

   pt = (PCRE2_Type *) SLmalloc (sizeof (PCRE2_Type));
   if (pt == NULL)
     return NULL;
   memset ((char *) pt, 0, sizeof (PCRE2_Type));

   pt->p = p;

   /* Create match data */
   pt->match_data = pcre2_match_data_create_from_pattern(p, General_Context);
   if (pt->match_data == NULL)
     {
	free_pcre2_type (pt);
	SLang_verror (SL_INTRINSIC_ERROR, "pcre2_match_data_create_from_pattern failed");
	return NULL;
     }

   if (NULL == (mmt = SLang_create_mmt (PCRE2_Type_Id, (VOID_STAR) pt)))
     {
	free_pcre2_type (pt);
	return NULL;
     }
   return mmt;
}

/* checked */
static int _pcre_compile_1 (char *pattern, int options)
{
   pcre2_code *p;
   SLang_MMT_Type *mmt;
   uint32_t pcre_options = 0;
   PCRE2_SIZE erroffset;
   int errorcode;

   /* Convert SLang options to PCRE2 options */
   if (options & 1) pcre_options |= PCRE2_CASELESS;
   if (options & 2) pcre_options |= PCRE2_MULTILINE;

   p = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED,
                     pcre_options, &errorcode, &erroffset, Compile_Context);
   if (p == NULL)
     {
	PCRE2_UCHAR buffer[256];
	int rc = pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
	if (rc < 0) *buffer = 0;

	SLang_verror (SL_Parse_Error, "Error compiling pattern '%s' at offset %d: %s",
		      pattern, (int)erroffset, buffer);
	return -1;
     }

   if (NULL == (mmt = allocate_pcre_type (p)))
     {
	pcre2_code_free(p);
	return -1;
     }

   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return -1;
     }
   return 0;
}

static void _pcre_compile (void)
{
   char *pattern;
   int options = 0;

   switch (SLang_Num_Function_Args)
     {
      case 2:
	if (-1 == SLang_pop_integer (&options))
	  return;
       /* fall through */
      case 1:
      default:
	if (-1 == SLang_pop_slstring (&pattern))
	  return;
     }
   (void) _pcre_compile_1 (pattern, options);
   SLang_free_slstring (pattern);
}

/* returns number of matches */
/* checked */
static int _pcre_exec_1 (PCRE2_Type *pt, char *str, unsigned int len, int pos, int options)
{
   uint32_t pcre_options = 0;
   int rc;

   /* Convert SLang options to PCRE2 options */
   if (options & 1) pcre_options |= PCRE2_NOTBOL;
   if (options & 2) pcre_options |= PCRE2_NOTEOL;

   pt->num_matches = 0;
   if ((unsigned int) pos > len)
     return 0;

   rc = pcre2_match(pt->p, (PCRE2_SPTR)str, len, pos,
                    pcre_options, pt->match_data, NULL);

   if (rc == PCRE2_ERROR_NOMATCH)
     return 0;

   if (rc < 0)
     {
	PCRE2_UCHAR buffer[256];
	int erc = pcre2_get_error_message(rc, buffer, sizeof(buffer));
	if (erc < 0) *buffer= 0;
	SLang_verror (SL_INTRINSIC_ERROR, "pcre2_match failed with error %d: %s", rc, buffer);
	return -1;
     }

   /* Note: rc = 0 means the ovector was too small for all captures,
    * but it's still a successful match.
    */
   if (rc == 0)
     rc = (int) pcre2_get_ovector_count(pt->match_data);

   pt->num_matches = (unsigned int) rc;
   return rc;
}

static int _pcre_exec (void)
{
   PCRE2_Type *p;
   SLang_MMT_Type *mmt;
   char *str;
   SLang_BString_Type *bstr = NULL;
   SLstrlen_Type len;
   int pos = 0;
   int options = 0;
   int ret = -1;

   switch (SLang_Num_Function_Args)
     {
      case 4:
	if (-1 == SLang_pop_integer (&options))
	  return -1;
       /* fall through */
      case 3:
       /* fall through */
	if (-1 == SLang_pop_integer (&pos))
	  return -1;
       /* fall through */
      default:
	switch (SLang_peek_at_stack())
	  {
	   case SLANG_STRING_TYPE:
	     if (-1 == SLang_pop_slstring (&str))
	       return -1;
	     len = strlen (str);
	     break;

	   case SLANG_BSTRING_TYPE:
	   default:
	     if (-1 == SLang_pop_bstring(&bstr))
	       return -1;
	     str = (char *)SLbstring_get_pointer(bstr, &len);
	     if (str == NULL)
	       {
		  SLbstring_free (bstr);
		  return -1;
              }
	     break;
         }
     }

   if (NULL == (mmt = SLang_pop_mmt (PCRE2_Type_Id)))
     goto free_and_return;
   p = (PCRE2_Type *)SLang_object_from_mmt (mmt);

   ret = _pcre_exec_1 (p, str, len, pos, options);

free_and_return:

   SLang_free_mmt (mmt);
   if (bstr != NULL)
     SLbstring_free (bstr);
   else
     SLang_free_slstring (str);
   return ret;
}

/* checked */
static int get_nth_start_stop (PCRE2_Type *pt, unsigned int n,
			       SLuindex_Type *a, SLuindex_Type *b)
{
   PCRE2_SIZE *ovector;

   if (n >= pt->num_matches)
     return -1;

   if (NULL == (ovector = pcre2_get_ovector_pointer(pt->match_data)))
     return -1;

   if ((ovector[2*n] == PCRE2_UNSET) || (ovector[2*n+1] == PCRE2_UNSET))
     return -1;

   *a = (SLuindex_Type) ovector[2*n];
   *b = (SLuindex_Type) ovector[2*n+1];
   return 0;
}

static void _pcre_nth_match (PCRE2_Type *pt, int *np)
{
   SLuindex_Type start, stop;
   SLang_Array_Type *at;
   SLindex_Type two = 2;
   int *data;

   if (-1 == get_nth_start_stop (pt, (unsigned int) *np, &start, &stop))
     {
	SLang_push_null ();
	return;
     }

   if (NULL == (at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &two, 1)))
     return;

   data = (int *)at->data;
   data[0] = (int)start;
   data[1] = (int)stop;
   (void) SLang_push_array (at, 1);
}

static void _pcre_nth_substr (PCRE2_Type *pt, char *str, int *np)
{
   SLstrlen_Type start, stop;
   SLstrlen_Type len;

   len = strlen (str);

   if ((-1 == get_nth_start_stop (pt, (unsigned int) *np, &start, &stop))
       || (start > len) || (stop > len))
     {
	SLang_push_null ();
	return;
     }

   str = SLang_create_nslstring (str + start, stop - start);
   (void) SLang_push_string (str);
   SLang_free_slstring (str);
}

/* This function converts a slang RE to a pcre expression. */
static char *_slang_to_pcre2 (char *slpattern)
{
   char *pattern, *p, *s;
   SLstrlen_Type len;
   int in_bracket;
   char ch;

   len = strlen (slpattern);
   pattern = (char *)SLmalloc (3*len + 1);
   if (pattern == NULL)
     return NULL;

   p = pattern;
   s = slpattern;
   in_bracket = 0;
   while ((ch = *s++) != 0)
     {
	switch (ch)
	  {
	   case '{':
	   case '}':
	   case '(':
	   case ')':
	   case '#':
	   case '|':
	     if (0 == in_bracket) *p++ = '\\';
	     *p++ = ch;
	     break;

	   case '[':
	     in_bracket = 1;
	     *p++ = ch;
	     break;

	   case ']':
	     in_bracket = 0;
	     *p++ = ch;
	     break;

	   case '\\':
	     ch = *s++;
	     switch (ch)
	       {
		case 0:
		  s--;
		  break;

		case '<':
		case '>':
		  *p++ = '\\'; *p++ = 'b';
		  break;

		case '(':
		case ')':
		case '{':
		case '}':
		  *p++ = ch;
		  break;

		case 'C':
		  *p++ = '('; *p++ = '?'; *p++ = 'i'; *p++ = ')';
		  break;
		case 'c':
		  *p++ = '('; *p++ = '?'; *p++ = '-'; *p++ = 'i'; *p++ = ')';
		  break;

		default:
		  *p++ = '\\';
		  *p++ = ch;
              }
	     break;

	   default:
	     *p++ = ch;
	     break;
         }
     }
   *p = 0;

   s = SLang_create_slstring (pattern);
   SLfree (pattern);
   return s;
}

static void slang_to_pcre2 (char *pattern)
{
   /* NULL ok in code below */
   pattern = _slang_to_pcre2 (pattern);
   (void) SLang_push_string (pattern);
   SLang_free_slstring (pattern);
}

static void destroy_pcre (SLtype type, VOID_STAR f)
{
   PCRE2_Type *pt;
   (void) type;

   pt = (PCRE2_Type *) f;
   free_pcre2_type (pt);
}

#define DUMMY_PCRE2_TYPE ((SLtype)-1)
#define P DUMMY_PCRE2_TYPE
#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define S SLANG_STRING_TYPE
static SLang_Intrin_Fun_Type PCRE2_Intrinsics [] =
{
   MAKE_INTRINSIC_0("pcre2_exec", _pcre_exec, I),
   MAKE_INTRINSIC_0("pcre2_compile", _pcre_compile, V),
   MAKE_INTRINSIC_2("pcre2_nth_match", _pcre_nth_match, V, P, I),
   MAKE_INTRINSIC_3("pcre2_nth_substr", _pcre_nth_substr, V, P, S, I),
   MAKE_INTRINSIC_1("slang_to_pcre2", slang_to_pcre2, V, S),
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type PCRE2_Consts [] =
{
   /* compile options */
#ifndef PCRE2_ANCHORED
# define PCRE2_ANCHORED 0
#endif
   MAKE_ICONSTANT("PCRE2_ANCHORED", PCRE2_ANCHORED),
#ifndef PCRE2_AUTO_CALLOUT
# define PCRE2_AUTO_CALLOUT 0
#endif
   MAKE_ICONSTANT("PCRE2_AUTO_CALLOUT", PCRE2_AUTO_CALLOUT),
#ifndef PCRE2_BSR_ANYCRLF
# define PCRE2_BSR_ANYCRLF 0
#endif
   MAKE_ICONSTANT("PCRE2_BSR_ANYCRLF", PCRE2_BSR_ANYCRLF),
#ifndef PCRE2_BSR_UNICODE
# define PCRE2_BSR_UNICODE 0
#endif
   MAKE_ICONSTANT("PCRE2_BSR_UNICODE", PCRE2_BSR_UNICODE),
#ifndef PCRE2_CASELESS
# define PCRE2_CASELESS 0
#endif
   MAKE_ICONSTANT("PCRE2_CASELESS", PCRE2_CASELESS),
#ifndef PCRE2_DUPNAMES
# define PCRE2_DUPNAMES 0
#endif
   MAKE_ICONSTANT("PCRE2_DUPNAMES", PCRE2_DUPNAMES),
#ifndef PCRE2_DOLLAR_ENDONLY
# define PCRE2_DOLLAR_ENDONLY 0
#endif
   MAKE_ICONSTANT("PCRE2_DOLLAR_ENDONLY", PCRE2_DOLLAR_ENDONLY),
#ifndef PCRE2_DOTALL
# define PCRE2_DOTALL 0
#endif
   MAKE_ICONSTANT("PCRE2_DOTALL", PCRE2_DOTALL),
#ifndef PCRE2_EXTENDED
# define PCRE2_EXTENDED 0
#endif
   MAKE_ICONSTANT("PCRE2_EXTENDED", PCRE2_EXTENDED),
#ifndef PCRE2_FIRSTLINE
# define PCRE2_FIRSTLINE 0
#endif
   MAKE_ICONSTANT("PCRE2_FIRSTLINE", PCRE2_FIRSTLINE),
#ifndef PCRE2_MULTILINE
# define PCRE2_MULTILINE 0
#endif
   MAKE_ICONSTANT("PCRE2_MULTILINE", PCRE2_MULTILINE),
#ifndef PCRE2_NEVER_UTF
# define PCRE2_NEVER_UTF 0
#endif
   MAKE_ICONSTANT("PCRE2_NEVER_UTF", PCRE2_NEVER_UTF),
#ifndef PCRE2_NEWLINE_ANY
# define PCRE2_NEWLINE_ANY 0
#endif
   MAKE_ICONSTANT("PCRE2_NEWLINE_ANY", PCRE2_NEWLINE_ANY),
#ifndef PCRE2_NEWLINE_ANYCRLF
# define PCRE2_NEWLINE_ANYCRLF 0
#endif
   MAKE_ICONSTANT("PCRE2_NEWLINE_ANYCRLF", PCRE2_NEWLINE_ANYCRLF),
#ifndef PCRE2_NEWLINE_CR
# define PCRE2_NEWLINE_CR 0
#endif
   MAKE_ICONSTANT("PCRE2_NEWLINE_CR", PCRE2_NEWLINE_CR),
#ifndef PCRE2_NEWLINE_CRLF
# define PCRE2_NEWLINE_CRLF 0
#endif
   MAKE_ICONSTANT("PCRE2_NEWLINE_CRLF", PCRE2_NEWLINE_CRLF),
#ifndef PCRE2_NEWLINE_LF
# define PCRE2_NEWLINE_LF 0
#endif
   MAKE_ICONSTANT("PCRE2_NEWLINE_LF", PCRE2_NEWLINE_LF),
#ifndef PCRE2_NO_START_OPTIMIZE
# define PCRE2_NO_START_OPTIMIZE 0
#endif
   MAKE_ICONSTANT("PCRE2_NO_START_OPTIMIZE", PCRE2_NO_START_OPTIMIZE),
#ifndef PCRE2_NOTEMPTY
# define PCRE2_NOTEMPTY 0
#endif
   MAKE_ICONSTANT("PCRE2_NOTEMPTY", PCRE2_NOTEMPTY),
#ifndef PCRE2_NO_AUTO_CAPTURE
# define PCRE2_NO_AUTO_CAPTURE 0
#endif
   MAKE_ICONSTANT("PCRE2_NO_AUTO_CAPTURE", PCRE2_NO_AUTO_CAPTURE),
#ifndef PCRE2_NO_AUTO_POSSESS
# define PCRE2_NO_AUTO_POSSESS 0
#endif
   MAKE_ICONSTANT("PCRE2_NO_AUTO_POSSESS", PCRE2_NO_AUTO_POSSESS),
#ifndef PCRE2_NO_UTF8_CHECK
# define PCRE2_NO_UTF8_CHECK 0
#endif
   MAKE_ICONSTANT("PCRE2_NO_UTF_CHECK", PCRE2_NO_UTF_CHECK),
#ifndef PCRE2_UCP
# define PCRE2_UCP 0
#endif
   MAKE_ICONSTANT("PCRE2_UCP", PCRE2_UCP),
#ifndef PCRE2_UNGREEDY
# define PCRE2_UNGREEDY 0
#endif
   MAKE_ICONSTANT("PCRE2_UNGREEDY", PCRE2_UNGREEDY),
#ifndef PCRE2_UTF
# define PCRE2_UTF 0
#endif
   MAKE_ICONSTANT("PCRE2_UTF", PCRE2_UTF),
#ifndef PCRE2_MATCH_INVALID_UTF
# define PCRE2_MATCH_INVALID_UTF 0
#endif
   MAKE_ICONSTANT("PCRE2_MATCH_INVALID_UTF", PCRE2_MATCH_INVALID_UTF),
   /* exec options */
#ifndef PCRE2_NOTBOL
# define PCRE2_NOTBOL 0
#endif
   MAKE_ICONSTANT("PCRE2_NOTBOL", PCRE2_NOTBOL),
#ifndef PCRE2_NOTEOL
# define PCRE2_NOTEOL 0
#endif
   MAKE_ICONSTANT("PCRE2_NOTEOL", PCRE2_NOTEOL),
#ifndef PCRE2_NOTEMPTY
# define PCRE2_NOTEMPTY 0
#endif
   MAKE_ICONSTANT("PCRE2_NOTEMPTY", PCRE2_NOTEMPTY),
#ifndef PCRE2_PARTIAL_SOFT
# define PCRE2_PARTIAL_SOFT 0
#endif
   MAKE_ICONSTANT("PCRE2_PARTIAL_SOFT", PCRE2_PARTIAL_SOFT),
#ifndef PCRE2_DFA_SHORTEST
# define PCRE2_DFA_SHORTEST 0
#endif
   MAKE_ICONSTANT("PCRE2_DFA_SHORTEST", PCRE2_DFA_SHORTEST),
#ifndef PCRE2_DFA_RESTART
# define PCRE2_DFA_RESTART 0
#endif
   MAKE_ICONSTANT("PCRE2_DFA_RESTART", PCRE2_DFA_RESTART),
#ifndef PCRE2_PARTIAL_HARD
# define PCRE2_PARTIAL_HARD 0
#endif
   MAKE_ICONSTANT("PCRE2_PARTIAL_HARD", PCRE2_PARTIAL_HARD),
#ifndef PCRE2_NOTEMPTY_ATSTART
# define PCRE2_NOTEMPTY_ATSTART 0
#endif
   MAKE_ICONSTANT("PCRE2_NOTEMPTY_ATSTART", PCRE2_NOTEMPTY_ATSTART),

   SLANG_END_ICONST_TABLE
};

#undef P
#undef I
#undef V
#undef S

static void *do_malloc (size_t n, void *data)
{
   (void) data;
   return (void *) SLmalloc (n);
}

static void do_free (void *x, void *data)
{
   (void) data;
   SLfree ((char *) x);
}

static int register_pcre_type (void)
{
   SLang_Class_Type *cl;

   if (PCRE2_Type_Id != 0)
     return 0;

   if (General_Context == NULL)
     {
	General_Context = pcre2_general_context_create(do_malloc, do_free, NULL);
	if (General_Context == NULL)
	  return -1;
     }
   if (Compile_Context == NULL)
     {
	Compile_Context = pcre2_compile_context_create (General_Context);
	if (Compile_Context == NULL)
	  return -1;
     }

   if (NULL == (cl = SLclass_allocate_class ("PCRE2_Type")))
     return -1;

   if (-1 == SLclass_set_destroy_function (cl, destroy_pcre))
     return -1;

   /* By registering as SLANG_VOID_TYPE, slang will dynamically allocate a
    * type.
    */
   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE, sizeof (PCRE2_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   PCRE2_Type_Id = SLclass_get_class_id (cl);
   if (-1 == SLclass_patch_intrin_fun_table1 (PCRE2_Intrinsics, DUMMY_PCRE2_TYPE, PCRE2_Type_Id))
     return -1;

   return 0;
}

int init_pcre2_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == register_pcre_type ())
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, PCRE2_Intrinsics, "__PCRE2__"))
       || (-1 == SLns_add_iconstant_table (ns, PCRE2_Consts, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_pcre2_module (void)
{
   if (Compile_Context != NULL)
     {
	pcre2_compile_context_free(Compile_Context);
	Compile_Context = NULL;
     }
   if (General_Context != NULL)
     {
	pcre2_general_context_free(General_Context);
	General_Context = NULL;
     }
}

