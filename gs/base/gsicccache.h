/* Copyright (C) 2001-2009 Artifex Software, Inc.
   All Rights Reserved.
  
   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/
   or contact Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134,
   San Rafael, CA  94903, U.S.A., +1(415)492-9861, for further information.
*/


/*  Header for the ICC profile link cache */


#ifndef gsicccache_INCLUDED
#  define gsicccache_INCLUDED

#include "gx.h"

gsicc_link_cache_t* gsicc_cache_new(gs_memory_t *memory);

void
gsicc_init_buffer(gsicc_bufferdesc_t *buffer_desc, unsigned char num_chan, unsigned char bytes_per_chan,
                  bool has_alpha, bool alpha_first, bool is_planar, int plane_stride, int row_stride,
                  int num_rows, int pixels_per_row);

static gsicc_link_t * gsicc_add_link(gsicc_link_cache_t *link_cache, void *link_handle,
               void *ContextPtr, gsicc_hashlink_t hashcode, gs_memory_t *memory);

static void gsicc_link_free(gsicc_link_t *icc_link, gs_memory_t *memory);

static void
gsicc_get_cspace_hash(gsicc_manager_t *icc_manager, cmm_profile_t *profile, int64_t *hash);

static void gsicc_compute_linkhash(gsicc_manager_t *icc_manager, cmm_profile_t *input_profile, 
                   cmm_profile_t *output_profile, 
                   gsicc_rendering_param_t *rendering_params, gsicc_hashlink_t *hash);

static gsicc_link_t* gsicc_findcachelink(gsicc_hashlink_t hashcode,gsicc_link_cache_t *icc_cache, 
                                   bool includes_proof);

static gsicc_link_t* gsicc_find_zeroref_cache(gsicc_link_cache_t *icc_cache);

static void gsicc_remove_link(gsicc_link_t *link,gsicc_link_cache_t *icc_cache, 
                              gs_memory_t *memory);

gsicc_link_t* gsicc_get_link(gs_imager_state * pis, gs_color_space  *input_colorspace, 
                    gs_color_space *output_colorspace, 
                    gsicc_rendering_param_t *rendering_params, gs_memory_t *memory, bool include_softproof);

gsicc_link_t* gsicc_get_link_profile(gs_imager_state *pis, cmm_profile_t *gs_input_profile, 
                    cmm_profile_t *gs_output_profile, 
                    gsicc_rendering_param_t *rendering_params, gs_memory_t *memory, bool include_softproof);


void gsicc_release_link(gsicc_link_t *icclink);


void gsicc_get_icc_buff_hash(unsigned char *buffer, int64_t *hash);

static void gsicc_get_buff_hash(unsigned char *data,unsigned int num_bytes,int64_t *hash);

static void rc_gsicc_cache_free(gs_memory_t * mem, void *ptr_in, client_name_t cname);

#endif

