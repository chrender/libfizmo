
/* hyphenation.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2010-2017 Christoph Ender.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Doing TeX hyphenation; see The TeXbook, Appendix H

#ifndef hyphenation_c_INCLUDED
#define hyphenation_c_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../tools/tracelog.h"
#include "../tools/z_ucs.h"
#include "../tools/list.h"
#include "../tools/i18n.h"
#include "../tools/filesys.h"
#include "../tools/stringmap.h"
#include "../locales/libfizmo_locales.h"
#include "../hyph_patterns/hyph_patterns.h"
#include "config.h"
#include "fizmo.h"
#include "hyphenation.h"


static z_ucs *last_pattern_locale = NULL;
static z_ucs *pattern_data;
static z_ucs **patterns;
static int nof_patterns = 0;



static int load_patterns() {
  z_ucs *current_locale;
  hyph_patterns *pattern_data;
#ifdef ENABLE_TRACING
  size_t i;
#endif // ENABLE_TRACING

  current_locale = get_current_locale_name();
  TRACE_LOG("Loading patterns for locale \"");
  TRACE_LOG_Z_UCS(current_locale);
  TRACE_LOG("\".\n");
  if (last_pattern_locale != NULL) {
    free(last_pattern_locale);
  }
  last_pattern_locale = z_ucs_dup(current_locale);

  pattern_data = (hyph_patterns*)get_stringmap_value(
      pattern_map, current_locale);

  if (pattern_data != NULL) {
    TRACE_LOG("Found requested hyphenation patterns.\n");
    nof_patterns = pattern_data->number_of_patterns;
    patterns = pattern_data->patterns;
    i = 0;
    while (i < nof_patterns) {
      TRACE_LOG("Pattern %d: ", i);
      TRACE_LOG_Z_UCS(patterns[i]);
      TRACE_LOG("\n");
      i++;
    }
    TRACE_LOG("Read %d patterns.\n", nof_patterns);
    return 0;
  }
  else {
    TRACE_LOG("Couldn't find hyphenation patterns.\n");
    nof_patterns = -1;
    return -1;
  }
}


static z_ucs *get_pattern(z_ucs *subword)
{
  int bottom = 0; // lowest not yet search element
  int top = nof_patterns-1; // highest not yet searched element
  int index, cmp;

  TRACE_LOG("Looking for pattern to subword: \"");
  TRACE_LOG_Z_UCS(subword);
  TRACE_LOG("\".\n");

  while (top - bottom >= 0)
  {
    index = bottom + ( (top - bottom) / 2 );

    if ((cmp = cmp_pattern(subword, patterns[index])) == 0)
      return patterns[index];
    else if (cmp > 0)
      top = index - 1;
    else bottom = index + 1;
  }

  return NULL;
}


z_ucs *hyphenate(z_ucs *word_to_hyphenate)
{
  int i, j, k, l, start_offset, end_offset;
  int word_len, process_index;
  z_ucs *ptr;
  int word_to_hyphenate_len, score, max_score;
  z_ucs buf;
  z_ucs *word_buf, *result_buf, *result_ptr;

  if (
      (word_to_hyphenate == NULL)
      ||
      ((word_to_hyphenate_len = z_ucs_len(word_to_hyphenate)) < 1)
     )
  {
    TRACE_LOG("hyph input empty.\n");
    return NULL;
  }

  if (
      (last_pattern_locale == NULL)
      ||
      (z_ucs_cmp(last_pattern_locale, get_current_locale_name()) != 0)
     )
  {
    if (load_patterns() < 0)
    {
      TRACE_LOG("Couldn't load patterns.\n");
      return NULL;
    }
  }

  if ((result_buf = malloc(
          sizeof(z_ucs) * (word_to_hyphenate_len * 2 + 1))) == NULL)
    return NULL;

  if (z_ucs_len(word_to_hyphenate) < 4)
  {
    z_ucs_cpy(result_buf, word_to_hyphenate);
    return result_buf;
  }

  if ((word_buf = malloc(
          sizeof(z_ucs) * (word_to_hyphenate_len + 3))) == NULL)
  {
    free(result_buf);
    return NULL;
  }

  *word_buf = '.';
  z_ucs_cpy(word_buf + 1, word_to_hyphenate);
  word_buf[word_to_hyphenate_len+1] = '.';
  word_buf[word_to_hyphenate_len+2] = 0;
  word_len = word_to_hyphenate_len + 2;

  TRACE_LOG("Hyphenate: \"");
  TRACE_LOG_Z_UCS(word_buf);
  TRACE_LOG("\".\n");

  result_ptr = result_buf;

  *(result_ptr++) = *word_to_hyphenate;

  // Process all inter-letter positions. From the TeXbook, page 453:
  // "[...] except that plain TeX blocks hyphens after the very first
  // letter or before the last or second-last letter of a word." Thus,
  // we'll simply process entirely the same range here to avoid strange
  // hyphenations.
  for (i=1; i<word_to_hyphenate_len-2; i++)
  {
#ifdef ENABLE_TRACING
    buf = word_buf[i+2];
    word_buf[i+2] = 0;
    TRACE_LOG("Processing: \"");
    TRACE_LOG_Z_UCS(word_buf+i);
    TRACE_LOG("\".\n");
    word_buf[i+2] = buf;
#endif // ENABLE_TRACING

    start_offset = i;
    end_offset = i + 1;
    process_index = 1;
    max_score = 0;

    for (j=1; j<=word_len; j++)
    {
      if (end_offset > word_len - j)
        end_offset = word_len - j;

      TRACE_LOG("j: %d, start: %d, end: %d, pi: %d\n",
          j, start_offset, end_offset, process_index);

      for (k=start_offset; k<=end_offset; k++)
      {
        buf = word_buf[k + j];
        word_buf[k + j] = 0;

        if ((ptr = get_pattern(word_buf + k)) != NULL)
        {
          TRACE_LOG("Found (%d): \"", k);
          TRACE_LOG_Z_UCS(ptr);
          TRACE_LOG("\".\n");

          l = 0;
          while (l < process_index - (k - start_offset))
          {
            TRACE_LOG("l: %d, process_index: %d.\n", l, process_index);
            if (isdigit(*ptr) == 0)
              l++;
            ptr++;
          }

          score = isdigit(*ptr) ? *ptr - '0' : 0;
          if (score > max_score)
            max_score = score;
          TRACE_LOG("score (max: %d): %d.\n", max_score, score);
        }

        word_buf[k + j] = buf;
      }

      if (start_offset != 0)
        start_offset--;
      if (process_index <= i)
        process_index++;
    }

    TRACE_LOG("Finished for position %d (%c).\n", i, word_buf[i]);

    if (i > 1)
    {
      *(result_ptr++) = word_buf[i];
      if ((max_score & 1) != 0)
      {
        *(result_ptr++) = Z_UCS_SOFT_HYPEN;
        TRACE_LOG("Found hyph point.\n");
      }
    }
  }

  *(result_ptr++) = word_buf[i];
  *(result_ptr++) = word_buf[i+1];
  *(result_ptr++) = word_buf[i+2];
  *result_ptr = 0;

  TRACE_LOG("Result: \"");
  TRACE_LOG_Z_UCS(result_buf);
  TRACE_LOG("\".\n");

  free(word_buf);

  return result_buf;
}


void free_hyphenation_memory(void)
{
  if (last_pattern_locale != NULL)
  {
    free(last_pattern_locale);
    last_pattern_locale = NULL;
  }
}

#endif /* hyphenation_c_INCLUDED */

