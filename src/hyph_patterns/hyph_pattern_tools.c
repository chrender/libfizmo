
/* hyph_pattern_tools.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2010-2023 Christoph Ender.
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

/* Contains everything required by compilation and pre-compilation's
 * "pattern_setup.c".
 */

#ifndef hyph_pattern_tools_c_INCLUDED
#define hyph_pattern_tools_c_INCLUDED

#include <ctype.h>

#include "../tools/z_ucs.h"

// Returns > 0 in case s1 > s2, 0 if equal and < 0 otherwise.
int cmp_pattern(z_ucs *s1, z_ucs *s2) {
  while ( (*s1 != 0) || (*s2 != 0) ) {
    if ( (*s1 != 0) && (isdigit(*s1) != 0) ) {
      s1++;
    }

    if ( (*s2 != 0) && (isdigit(*s2) != 0) ) {
      s2++;
    }

    if (*s1 != *s2) {
      return *s2 - *s1;
    }

    s1++;
    s2++;
  }

  return 0;
}


#endif // hyph_pattern_tools_c_INCLUDED

