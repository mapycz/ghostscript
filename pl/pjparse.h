/* Portions Copyright (C) 2001 artofcode LLC.
   Portions Copyright (C) 1996, 2001 Artifex Software Inc.
   Portions Copyright (C) 1988, 2000 Aladdin Enterprises.
   This software is based in part on the work of the Independent JPEG Group.
   All Rights Reserved.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/ or
   contact Artifex Software, Inc., 101 Lucas Valley Road #110,
   San Rafael, CA  94903, (415)492-9861, for further information. */
/*$Id$ */

/* A simplified version of the PJL parser and states for demonstration
   purposes.  It implements a subset of pjl - specifically PDL
   settable PJL environmant variables.  Clients should initialize the
   pjl parser with pjl_process_init() and release it with
   pjl_process_destroy().  The principle means of receiving
   environment variables is by the function call pjl_get_envvar() see
   below.  The return value of this function call will return the
   appropriate environment value requested.  Note there is no
   mechanism for clients to receive pjl default or factory default
   setting.  We assume clients are PDL's and only have to
   reset their state values to the pjl environment.  See PJLTRM for
   more details */

#ifndef pjparse_INCLUDED
#  define pjparse_INCLUDED

#include "scommon.h"  /* stream_cursor_read */

/* opaque definition for the parser state */
typedef struct pjl_parser_state_s pjl_parser_state;

/* type definition for environment variable */
#ifndef PJL_ENVAR_T
#define PJL_ENVAR_T
typedef char pjl_envvar_t;
#endif /* PJL_ENVVAR_T */
/* ---------------- Procedural interface ---------------- */

/* Initialize the PJL parser and state. */
pjl_parser_state *pjl_process_init(P1(gs_memory_t *mem));

/* Destroy an instance of the the PJL parser and state. */
void pjl_process_destroy(P2(pjl_parser_state *pst, gs_memory_t *mem));

/* Process a buffer of PJL commands. */
/* Return 1 if we are no longer in PJL mode. */
int pjl_process(P3(pjl_parser_state *pst, void *pstate,
		   stream_cursor_read *pr));

/* Discard the remainder of a job.  Return true when we reach a UEL. */
/* The input buffer must be at least large enough to hold an entire UEL. */
bool pjl_skip_to_uel(P1(stream_cursor_read *pr));
/* return the current setting of a pjl environment variable.  The
   input parameter should be the exact string used in PJLTRM.
   Sample Usage: 
         char *formlines = pjl_get_envvar(pst, "formlines");
	 if (formlines) {
	     int fl = atoi(formlines);
	     .
	     .
	 }
   Both variables and values are case insensitive.
*/
pjl_envvar_t *pjl_get_envvar(P2(pjl_parser_state *pst, const char *pjl_var));

/* compare a pjl environment variable to a string values. */
int pjl_compare(P2(const pjl_envvar_t *s1, const char *s2));

/* map a pjl symbol set name to a pcl integer */
int pjl_map_pjl_sym_to_pcl_sym(P1(const pjl_envvar_t *symname));

/* pjl environment variable to integer. */
int pjl_vartoi(P1(const pjl_envvar_t *s));

/* pjl envioronment variable to float. */
floatp pjl_vartof(P1(const pjl_envvar_t *s));

/* convert a pjl designated fontsource to a subdirectory pathname.  */
char *pjl_fontsource_to_path(P2(const pjl_parser_state *pjls,
				const pjl_envvar_t *fontsource));

/* Change to next highest priority font source.  The following events
   automatically change the value of the FONTSOURCE variable to the
   next highest priority font source containing a default-marked font:
   if the currently set font source is C, C1, or C2, and the cartridge
   is removed from the printer; if the currently set font source is S
   and all soft fonts are deleted; if the currently set font source is
   S, while the currently set font number is the highest-numbered soft
   font, and any soft font is deleted.  Ideally this function would be
   solely responsible for these activities, with the current
   architecture we depend in part on pcl to keep up with font resource
   bookkeeping.  PJLTRM is not careful to define distinguish between
   default font source vs environment font source.  Both are set when
   the font source is changed. */
void pjl_set_next_fontsource(P1(pjl_parser_state* pst));

/* tell pjl that a soft font is being deleted.  We return 0 if no
   state change is required and 1 if the pdl should update its font
   state.  (see discussion above) */
int pjl_register_permanent_soft_font_deletion(P2(pjl_parser_state *pst, int font_number));

/* request that pjl add a soft font and return a pjl font number for
   the font.   */
int pjl_register_permanent_soft_font_addition(P1(pjl_parser_state *pst));

/* set the initial environment to the default environment, this should
   be done at the beginning of each job */
void pjl_set_init_from_defaults(P1(pjl_parser_state *pst));

/* returns the size of an named resource, presumably the client can
   use this number (bytes) to allocate memory for the object. 0
   returned if the object is not found */
long int pjl_get_named_resource_size(P2(pjl_parser_state *pst, char *name));

/* returns the contents of the resource in "mem" allocated by the
   client.  Returns < 0 if it fails to find the object */
int pjl_get_named_resource(P3(pjl_parser_state *pst, char *name, byte *data));

#endif				/* pjparse_INCLUDED */
