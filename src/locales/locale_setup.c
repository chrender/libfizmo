
/* locale_setup.c
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

#include "locale_setup.h"


static int load_locales(char *filename, FILE *output_file, char *locale_code) {
  z_file *locale_file;
  z_ucs input;
  list *message_indices = create_list();
  size_t output_index = 0;
  size_t number_of_messages = 0;
  size_t data_index = 0;
  size_t message_length = 0;
  size_t message_index;

  locale_file = fsi->openfile(filename, FILETYPE_DATA, FILEACCESS_READ);

  fprintf(output_file, "z_ucs locale_data_%s[] = {\n  ", locale_code);
  do {
    input = parse_utf8_char_from_file(locale_file);
    if (input != UEOF) {
      do {
        if (message_length == 0) {
          add_list_element(message_indices, (void*)data_index);
          number_of_messages++;
        }
        if (output_index == 6) {
          output_index = 0;
          fprintf(output_file, ",\n  ");
        }
        else if (output_index != 0) {
          fprintf(output_file, ", ");
        }
        if (input == Z_UCS_NEWLINE) {
          fprintf(output_file, "0x%08x", 0);
          message_length = 0;
        }
        else {
          fprintf(output_file, "0x%08x", input);
          message_length++;
        }
        output_index++;
        data_index++;
        input = parse_utf8_char_from_file(locale_file);
      }
      while (input != UEOF);
      fprintf(output_file, "\n};\n\n");
    }
  }
  while (input != UEOF);
  fsi->closefile(locale_file);

  fprintf(output_file, "z_ucs *locale_messages_%s[] = {\n  ", locale_code);
  output_index = 0;
  for (message_index=0; message_index<get_list_size(message_indices);
      message_index++) {
    if (output_index == 3) {
      output_index = 0;
      fprintf(output_file, ",\n  ");
    }
    else if (output_index != 0) {
      fprintf(output_file, ", ");
    }
    fprintf(output_file, "locale_data_%s+%4zu", locale_code,
        (size_t)get_list_element(message_indices, message_index));
    output_index++;
  }
  fprintf(output_file, "\n}\n\n");

  return number_of_messages;
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


int main(int argc, char *argv[]) {
  struct dirent *dp;
  DIR *dfd;
  FILE *output_file;
  char *locale_code;
  list *locale_code_list;
  int nof_locale_codes;
  char **locale_codes;
  int *nof_strings_by_index;
  char input_filename[10];
  int i, j;

  if ((dfd = opendir(".")) == NULL) {
    fputs("Can't open dir", stderr);
    return 0;
  }

  locale_code_list = create_list();
  while ((dp = readdir(dfd)) != NULL) {
    struct stat stbuf;
    if (stat(dp->d_name,&stbuf) == -1) {
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
  nof_strings_by_index = (int*)malloc(sizeof(int) * nof_locale_codes);
  locale_codes = (char**)delete_list_and_get_ptrs(locale_code_list);

  output_file = fopen("locale_data.c", "w");
  write_disclaimer_to_file(output_file, "locale_data.c");
  fprintf(output_file,
      "\n"
      "#include <stdlib.h>\n"
      "\n"
      "#include \"../tools/z_ucs.h\"\n"
      "#include \"../tools/stringmap.h\"\n"
      "\n"
      "static stringmap *locale_map = NULL;\n"
      "\n");

  i=0;
  while (i < nof_locale_codes) {
    locale_code = locale_codes[i];
    sprintf(input_filename, "%s.txt", locale_code);
    printf("Processing \"%s\".\n", input_filename);
    nof_strings_by_index[i] = load_locales(input_filename, output_file, locale_code);
    i++;
  }

  fprintf(output_file,
      "\n"
      "void init_locales() {\n"
      "  locale_map = create_stringmap();\n\n");

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
        "  add_stringmap_element(locale_map, locale_code_%s, locale_messages_%s);\n",
        locale_code, locale_code);
    i++;
  }
  fprintf(output_file, "}\n\n");

  fclose(output_file);
  free(nof_strings_by_index);
  free(locale_codes);

  exit(0);
}

