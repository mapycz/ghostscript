/*
    jbig2dec
    
    Copyright (c) 2001 artofcode LLC.
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    $Id: jbig2_huffman.c,v 1.6 2001/06/26 00:30:00 giles Exp $
*/

/* Huffman table decoding procedures 
    -- See Annex B of the JBIG2 draft spec */

#include <stdlib.h>

#include "jbig2dec.h"
#include "jbig2_huffman.h"
#include "jbig2_hufftab.h"

#define JBIG2_HUFFMAN_FLAGS_ISOOB 1
#define JBIG2_HUFFMAN_FLAGS_ISLOW 2
#define JBIG2_HUFFMAN_FLAGS_ISEXT 4



struct _Jbig2HuffmanState {
  /* The current bit offset is equal to (offset * 8) + offset_bits.
     The MSB of this_word is the current bit offset. The MSB of next_word
     is (offset + 4) * 8. */
  uint32 this_word;
  uint32 next_word;
  int offset_bits;
  int offset;

  Jbig2WordStream *ws;
};


Jbig2HuffmanState *
jbig2_huffman_new (Jbig2WordStream *ws)
{
  Jbig2HuffmanState *result = (Jbig2HuffmanState *)malloc (sizeof(Jbig2HuffmanState));

  result->offset = 0;
  result->offset_bits = 0;
  result->this_word = ws->get_next_word (ws, 0);
  result->next_word = ws->get_next_word (ws, 4);

  result->ws = ws;

  return result;
}

int32
jbig2_huffman_get (Jbig2HuffmanState *hs,
		   const Jbig2HuffmanTable *table, bool *oob)
{
  Jbig2HuffmanEntry *entry;
  byte flags;
  int offset_bits = hs->offset_bits;
  uint32 this_word = hs->this_word;
  uint32 next_word;
  int RANGELEN;
  int32 result;

  for (;;)
    { 
      int log_table_size = table->log_table_size;
      int PREFLEN;

      entry = &table->entries[this_word >> (32 - log_table_size)];
      flags = entry->flags;
      PREFLEN = entry->PREFLEN;

      next_word = hs->next_word;
      offset_bits += PREFLEN;
      if (offset_bits >= 32)
	{
	  Jbig2WordStream *ws = hs->ws;
	  this_word = next_word;
	  hs->offset += 4;
	  next_word = ws->get_next_word (ws, hs->offset + 4);
	  offset_bits -= 32;
	  hs->next_word = next_word;
	  PREFLEN = offset_bits;
	}
      this_word = (this_word << PREFLEN) |
	(next_word >> (32 - offset_bits));
      if (flags & JBIG2_HUFFMAN_FLAGS_ISEXT)
	{
	  table = entry->u.ext_table;
	}
      else
	break;
    }
  result = entry->u.RANGELOW;
  RANGELEN = entry->RANGELEN;
  if (RANGELEN > 0)
    {
      int32 HTOFFSET;

      HTOFFSET = this_word >> (32 - RANGELEN);
      if (flags & JBIG2_HUFFMAN_FLAGS_ISLOW)
	result -= HTOFFSET;
      else
	result += HTOFFSET;

      offset_bits += RANGELEN;
      if (offset_bits >= 32)
	{
	  Jbig2WordStream *ws = hs->ws;
	  this_word = next_word;
	  hs->offset += 4;
	  next_word = ws->get_next_word (ws, hs->offset + 4);
	  offset_bits -= 32;
	  hs->next_word = next_word;
	  RANGELEN = offset_bits;
	}
      this_word = (this_word << RANGELEN) |
	(next_word >> (32 - offset_bits));
    }

  hs->this_word = this_word;
  hs->offset_bits = offset_bits;

  if (oob != NULL)
    *oob = (flags & JBIG2_HUFFMAN_FLAGS_ISOOB);

  return result;
}

#define LOG_TABLE_SIZE_MAX 8

Jbig2HuffmanTable *
jbig2_build_huffman_table (const Jbig2HuffmanParams *params)
{
  int LENCOUNT[256];
  int LENMAX = -1;
  const Jbig2HuffmanLine *lines = params->lines;
  int n_lines = params->n_lines;
  int i, j;
  int log_table_size = 0;
  Jbig2HuffmanTable *result;
  Jbig2HuffmanEntry *entries;
  int CURLEN;
  int firstcode = 0;
  int CURCODE;
  int CURTEMP;

  /* B.3, 1. */
  for (i = 0; i < params->n_lines; i++)
    {
      int PREFLEN = lines[i].PREFLEN;
      int lts;

      if (PREFLEN > LENMAX)
	{
	  for (j = LENMAX + 1; j < PREFLEN + 1; j++)
	    LENCOUNT[j] = 0;
	  LENMAX = PREFLEN;
	}
      LENCOUNT[PREFLEN]++;

      lts = PREFLEN + lines[i].RANGELEN;
      if (lts > LOG_TABLE_SIZE_MAX)
	lts = PREFLEN;
      if (lts <= LOG_TABLE_SIZE_MAX && log_table_size < lts)
	log_table_size = lts;
    }
  result = (Jbig2HuffmanTable *)malloc (sizeof(Jbig2HuffmanTable));
  result->log_table_size = log_table_size;
  entries = (Jbig2HuffmanEntry *)malloc (sizeof(Jbig2HuffmanEntry) << log_table_size);
  result->entries = entries;

  LENCOUNT[0] = 0;

  for (CURLEN = 1; CURLEN <= LENMAX; CURLEN++)
    {
      int shift = log_table_size - CURLEN;

      /* B.3 3.(a) */
      firstcode = (firstcode + LENCOUNT[CURLEN - 1]) << 1;
      CURCODE = firstcode;
      /* B.3 3.(b) */
      for (CURTEMP = 0; CURTEMP < n_lines; CURTEMP++)
	{
	  int PREFLEN = lines[CURTEMP].PREFLEN;
	  if (PREFLEN == CURLEN)
	    {
	      int RANGELEN = lines[CURTEMP].RANGELEN;
	      int start_j = CURCODE << shift;
	      int end_j = (CURCODE + 1) << shift;
	      byte eflags = 0;

	      /* todo: build extension tables */
	      if (params->HTOOB && CURTEMP == n_lines - 1)
		eflags |= JBIG2_HUFFMAN_FLAGS_ISOOB;
	      if (CURTEMP == n_lines - (params->HTOOB ? 3 : 2))
		eflags |= JBIG2_HUFFMAN_FLAGS_ISLOW;
	      if (PREFLEN + RANGELEN > LOG_TABLE_SIZE_MAX)
		{
		  for (j = start_j; j < end_j; j++)
		    {
		      entries[j].u.RANGELOW = lines[CURTEMP].RANGELOW;
		      entries[j].PREFLEN = PREFLEN;
		      entries[j].RANGELEN = RANGELEN;
		      entries[j].flags = eflags;
		    }
		}
	      else
		{
		  for (j = start_j; j < end_j; j++)
		    {
		      int32 HTOFFSET = (j >> (shift - RANGELEN)) &
			((1 << RANGELEN) - 1);
		      if (eflags & JBIG2_HUFFMAN_FLAGS_ISLOW)
			entries[j].u.RANGELOW = lines[CURTEMP].RANGELOW -
			  HTOFFSET;
		      else
			entries[j].u.RANGELOW = lines[CURTEMP].RANGELOW +
			  HTOFFSET;
		      entries[j].PREFLEN = PREFLEN + RANGELEN;
		      entries[j].RANGELEN = 0;
		      entries[j].flags = eflags;
		    }
		}
	      CURCODE++;
	    }
	}
    }

  return result;
}

#ifdef TEST
#include <stdio.h>

static uint32
test_get_word (Jbig2WordStream *self, int offset)
{
	byte	stream[] = { 0xe9, 0xcb, 0xf4, 0x00 };
	
	/* assume stream[] is at least 4 bytes */
	if (offset+3 > sizeof(stream))
		return 0;
	else
		return ( (stream[offset] << 24) |
				 (stream[offset+1] << 16) |
				 (stream[offset+2] << 8) |
				 (stream[offset+3]) );
}

int
main (int argc, char **argv)
{
  Jbig2HuffmanTable *b1;
  Jbig2HuffmanTable *b2;
  Jbig2HuffmanTable *b4;
  Jbig2HuffmanState *hs;
  Jbig2WordStream ws;
  bool oob;
  int32 code;
  
  b1 = jbig2_build_huffman_table (&jbig_huffman_params_A);
  b2 = jbig2_build_huffman_table (&jbig_huffman_params_B);
  b4 = jbig2_build_huffman_table (&jbig_huffman_params_D);
  ws.get_next_word = test_get_word;
  hs = jbig2_huffman_new (&ws);

  printf("testing jbig2 huffmann decoding...");
  printf("\t(should be 8 5 (oob) 8)\n");
  
  code = jbig2_huffman_get (hs, b4, &oob);
  if (oob) printf("(oob) ");
    else printf("%d ", code);
  
  code = jbig2_huffman_get (hs, b2, &oob);
  if (oob) printf("(oob) ");
    else printf("%d ", code);
  
  code = jbig2_huffman_get (hs, b2, &oob);
  if (oob) printf("(oob) ");
    else printf("%d ", code);
  
    code = jbig2_huffman_get (hs, b1, &oob);
  if (oob) printf("(oob) ");
    else printf("%d ", code);
  
  printf("\n");
  
  return 0;
}
#endif
