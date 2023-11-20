
/* pattern_setup.c
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

// Doing TeX hyphenation; see The TeXbook, Appendix H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../tools/z_ucs.h"
#include "../tools/filesys.h"
#include "../tools/list.h"

#include "hyph_pattern_tools.h"


static int divide(z_ucs **patterns, int start, int end) {
  int i=start, j=end;
  z_ucs *pattern;
  z_ucs *pivot = patterns[end];

  do {
    while ( (i < end) && (cmp_pattern(patterns[i], pivot) >= 0 ) ) {
      i++;
    }

    while ( (j >= start) && (cmp_pattern(patterns[j], pivot) <= 0 ) ) {
      j--;
    }

    if (i < j) {
      pattern = patterns[i];
      patterns[i] = patterns[j];
      patterns[j] = pattern;
    }
  }
  while (i < j);

  if (cmp_pattern(patterns[i], pivot) < 0) {
    pattern = patterns[i];
    patterns[i] = patterns[end];
    patterns[end] = pattern;
  }

  return i;
}


static void sort_patterndata(z_ucs **patterns, int start, int end) {
  int div;

  if (start < end) {
    div = divide(patterns, start, end);
    sort_patterndata(patterns, start, div - 1);
    sort_patterndata(patterns, div + 1, end);
  }
}


static int load_patterns(char *filename, FILE *output_file, char *locale_code,
    bool dump_sorted_patterns) {
  z_file *patternfile;
  z_ucs input;
  z_ucs *pattern_input_buffer = NULL;
  size_t pattern_input_buffer_size = 0;
  size_t pattern_input_buffer_index = 0;
  list *input_patterns = create_list();
  z_ucs **parsed_patterns;
  size_t parsed_patterns_size;
  size_t parsed_patterns_index;
  size_t *pattern_indices = NULL;
  size_t pattern_indices_index = 0;
  size_t pattern_indices_size = 0;
  size_t pattern_data_index = 0;
  size_t output_index;
  size_t i;
  FILE *dump_file = NULL;
  size_t dump_file_name_length = 0;
  char *dump_file_name = NULL;

  patternfile = fsi->openfile(filename, FILETYPE_DATA, FILEACCESS_READ);

  do {
    input = parse_utf8_char_from_file(patternfile);
    if (input == (z_ucs)'%') {
      // Found comment, skip over it.
      do {
        input = parse_utf8_char_from_file(patternfile);
      }
      while ( (input != Z_UCS_NEWLINE) && (input != UEOF) );
    }
    else if (input != UEOF) {
      pattern_input_buffer_index = 0;
      while ( (input != Z_UCS_NEWLINE) && (input != UEOF) ) {
        // ensure there's at least space for two z_ucs since we've
        // also got to terminate the string at the end.
        if (pattern_input_buffer_index + 1 >= pattern_input_buffer_size) {
          pattern_input_buffer_size += 128;
          if ((pattern_input_buffer = realloc(
             pattern_input_buffer, pattern_input_buffer_size * sizeof(z_ucs)))
              == NULL) {
            fputs("Couldn't allocate memory.", stderr);
            exit(-1);
          }
        }
        pattern_input_buffer[pattern_input_buffer_index++] = input;
        input = parse_utf8_char_from_file(patternfile);
      }
      // end of regular pattern string, terminate string in-memory.
      pattern_input_buffer[pattern_input_buffer_index++] = 0;
      add_list_element(input_patterns, z_ucs_dup(pattern_input_buffer));
    }
  }
  while (input != UEOF);
  fsi->closefile(patternfile);

  pattern_input_buffer_index = 0;
  pattern_input_buffer_size = 0;
  free(pattern_input_buffer);
  pattern_input_buffer = NULL;

  parsed_patterns_size = get_list_size(input_patterns);
  parsed_patterns = (z_ucs**)delete_list_and_get_ptrs(input_patterns);

  sort_patterndata(parsed_patterns, 0, parsed_patterns_size - 1);

  if (dump_sorted_patterns == true) {
    dump_file_name_length
      = snprintf(NULL, 0, "%s_sorted_dump.txt", locale_code);
    dump_file_name = malloc(dump_file_name_length);
    sprintf(dump_file_name, "%s_sorted_dump.txt", locale_code);
    dump_file = fopen(dump_file_name, "w");
    parsed_patterns_index = 0;
    while (parsed_patterns_index < parsed_patterns_size) {
      i = 0;
      do {
        fputc(parsed_patterns[parsed_patterns_index][i], dump_file);
      }
      while (parsed_patterns[parsed_patterns_index][++i] != 0);
      fputc('\n', dump_file);
      parsed_patterns_index++;
    }
    fclose(dump_file);
    free(dump_file_name);
  }

  pattern_indices_index = 0;
  pattern_indices_size = parsed_patterns_size;
  pattern_indices = malloc(sizeof(size_t) * pattern_indices_size);

  fprintf(output_file, "static z_ucs pattern_data_%s[] = {\n  ", locale_code);
  parsed_patterns_index = 0;
  pattern_data_index = 0;
  output_index = 0;
  while (parsed_patterns_index < parsed_patterns_size) {
    pattern_indices[pattern_indices_index] = pattern_data_index;
    i = 0;
    do {
      if (output_index == 6) {
        output_index  = 0;
        fprintf(output_file, ",\n  ");
      }
      else if (output_index  != 0) {
        fprintf(output_file, ", ");
      }
      fprintf(output_file, "0x%08x", parsed_patterns[parsed_patterns_index][i]);
      pattern_data_index++;
      output_index++;
      i++;
    }
    while (parsed_patterns[parsed_patterns_index][i] != 0);
    if (output_index == 6) {
      output_index  = 0;
      fprintf(output_file, ",\n  ");
    }
    else if (output_index  != 0) {
      fprintf(output_file, ", ");
    }
    fprintf(output_file, "0x%08x", 0);
    output_index++;
    pattern_data_index++;
    parsed_patterns_index++;
    pattern_indices_index++;
  }
  fprintf(output_file, "\n};\n\n");

  fprintf(output_file, "static z_ucs *patterns_%s[] = {\n  ", locale_code);
  pattern_indices_index = 0;
  i = 0;
  while (pattern_indices_index < pattern_indices_size) {
    if (i == 3) {
      i = 0;
      fprintf(output_file, ",\n  ");
    }
    else if (i != 0) {
      fprintf(output_file, ", ");
    }
    fprintf(output_file, "pattern_data_%s+%*zu", locale_code, 5, pattern_indices[pattern_indices_index]);
    i++;
    pattern_indices_index++;
  }
  fprintf(output_file, "\n};\n\n");

  free(pattern_indices);

  return pattern_indices_size;
}


void write_disclaimer_to_file(FILE *output_file, char *filename) {
  fprintf(output_file, "/*\n"
      " * %s\n"
      " *\n"
      " * This file is part of fizmo.\n"
      " *\n"
      " * Copyright (c) 2010-2023 Christoph Ender.\n"
      " * All rights reserved.\n"
      " *\n"
      " * Redistribution and use in source and binary forms, with or without\n"
      " * modification, are permitted provided that the following conditions\n"
      " * are met:\n"
      " * 1. Redistributions of source code must retain the above copyright\n"
      " *    notice, this list of conditions and the following disclaimer.\n"
      " * 2. Redistributions in binary form must reproduce the above copyright\n"
      " *    notice, this list of conditions and the following disclaimer in the\n"
      " *    documentation and/or other materials provided with the distribution.\n"
      " * 3. Neither the name of the copyright holder nor the names of its\n"
      " *    contributors may be used to endorse or promote products derived\n"
      " *    from this software without specific prior written permission.\n"
      " * \n"
      " * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS\n"
      " * IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,\n"
      " * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
      " * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR\n"
      " * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
      " * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
      " * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
      " * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
      " * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
      " * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
      " * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
      " */\n", filename);
}


void print_syntax_and_exit() {
  printf("Syntax: pattern_setup [--dump-sorted-patterns]\n");
  exit(1);
}


int main(int argc, char *argv[]) {
 struct dirent *dp;
 DIR *dfd;
 FILE *output_file;
 char *locale_code;
 list *locale_code_list;
 int nof_locale_codes;
 char **locale_codes;
 int *nof_patterns_by_index;
 char input_filename[10];
 int i, j;
 bool dump_sorted_patterns = false;

 if (argc > 2) {
   print_syntax_and_exit();
 }

 if (argc == 2) {
   if (strcmp(argv[1], "--dump-sorted-patterns") != 0) {
     print_syntax_and_exit();
   }
   else {
     dump_sorted_patterns = true;
   }
 }

 if ((dfd = opendir(".")) == NULL) {
   fputs("Can't open dir", stderr);
   return 0;
 }

 locale_code_list = create_list();
 while ((dp = readdir(dfd)) != NULL)
 {
   struct stat stbuf ;
   if( stat(dp->d_name,&stbuf ) == -1 )
   {
     printf("Unable to stat file: %s\n",dp->d_name) ;
     continue ;
   }

   if ( ( stbuf.st_mode & S_IFMT ) == S_IFDIR ) {
     continue;
   }
   else {
     if (strlen(dp->d_name) == 9) { // strlen("xx_XX.txt") == 9
       locale_code= malloc(6);
       strncpy(locale_code, dp->d_name, 5);
       locale_code[5] = 0;
       add_list_element(locale_code_list, locale_code);
     }
   }
 }
 nof_locale_codes = get_list_size(locale_code_list);
 nof_patterns_by_index = (int*)malloc(sizeof(int) * nof_locale_codes);
 locale_codes = (char**)delete_list_and_get_ptrs(locale_code_list);

 output_file = fopen("hyph_patterns.c", "w");
 write_disclaimer_to_file(output_file, "hyph_patterns.c");
 fprintf(output_file,
     "\n"
     "#include <stdlib.h>\n"
     "\n"
     "#include \"../tools/z_ucs.h\"\n"
     "#include \"../tools/stringmap.h\"\n"
     "#include \"hyph_pattern_tools.h\"\n"
     "\n"
     "stringmap *pattern_map = NULL;\n"
     "\n");

 i=0;
 while (i < nof_locale_codes) {
   locale_code = locale_codes[i];
   sprintf(input_filename, "%s.txt", locale_code);
   printf("Processing \"%s\".\n", input_filename);
   nof_patterns_by_index[i] = load_patterns(input_filename, output_file, locale_code, dump_sorted_patterns);
   i++;
 }

 fprintf(output_file,
     "\n"
     "void init_patterns() {\n"
     "  pattern_map = create_stringmap();\n\n");

 i=0;
 while (i < nof_locale_codes) {
   locale_code = locale_codes[i];
   fprintf(output_file, "  hyph_patterns *hyph_patterns_%s = malloc(sizeof(hyph_patterns));\n",
       locale_code);
   fprintf(output_file, "  hyph_patterns_%s->patterns = patterns_%s;\n",
       locale_code, locale_code);
   fprintf(output_file, "  hyph_patterns_%s->number_of_patterns = %d;\n\n",
       locale_code, nof_patterns_by_index[i]);
   i++;
 }

 i=0;
 while (i < nof_locale_codes) {
   locale_code = locale_codes[i];
   fprintf(output_file, "  z_ucs locale_code_%s[] = {\n    ", locale_code);
   for (j=0; j<5; j++) {
     fprintf(output_file, "(z_ucs)'%c', ", locale_code[j]);
   }
   fprintf(output_file, "(z_ucs)0 };\n");
   i++;
 }
 fputs("\n", output_file);

 i=0;
 while (i < nof_locale_codes) {
   locale_code = locale_codes[i];
   fprintf(output_file,
       "  add_stringmap_element(pattern_map, locale_code_%s, hyph_patterns_%s);\n",
       locale_code, locale_code);
   i++;
 }
 fprintf(output_file, "}\n\n");

 fclose(output_file);
 free(nof_patterns_by_index);
 free(locale_codes);

 exit(0);
}

