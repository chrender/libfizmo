
/* i18n.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2009-2023 Christoph Ender.
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


#ifndef i18n_c_INCLUDED
#define i18n_c_INCLUDED

#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>

#include "i18n.h"
#include "tracelog.h"
#include "z_ucs.h"
#include "stringmap.h"
#include "list.h"
#include "filesys.h"
#include "../locales/libfizmo_locales.h"

#define LATIN1_TO_Z_UCS_BUFFER_SIZE 64

static z_ucs i18n_fallback_error_message[] = {
   'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ',
   'i', '1', '8', 'n', ' ',
   'e', 'r', 'r', 'o', 'r', '.',
   0 };

static char *locale_aliases[4][6] =
{
  { "en_GB", "en-GB", "en_US", "en-US", "en", NULL },
  { "de_DE", "de", NULL },
  { "fr_FR", "fr", NULL },
  { NULL }
};

static z_ucs *current_locale_name = NULL;
static char *current_locale_name_in_utf8 = NULL;
static char *default_locale_name_in_utf8 = NULL;
static int (*stream_output_function)(z_ucs *output) = NULL;
static void (*abort_function)(int exit_code, z_ucs *error_message) = NULL;
static stringmap *locale_modules = NULL; // "locale_module"s by module_name
static list *list_of_avaialable_locales_codes = NULL;


// Returns 0 on success, non-zero otherwise.
int register_locale_module(z_ucs *module_name, locale_module *new_module) {
  z_ucs **new_locale_names;
  z_ucs *locale_name;
  int i, j;
  bool locale_name_found;

  if (locale_modules == NULL) {
    locale_modules = create_stringmap();
  }

  if (is_name_stored_in_stringmap(
        locale_modules, new_module->module_name) != false) {
    TRACE_LOG("Locale module \"%s\" is already registered.\n",
        new_module->module_name);
    return -1;
  }

  if ((add_stringmap_element(
      locale_modules, new_module->module_name, new_module)) != 0) {
    return -1;
  }

  if (list_of_avaialable_locales_codes == NULL) {
    TRACE_LOG("Initializing list_of_avaialable_locales_codes.");
    list_of_avaialable_locales_codes = create_list();
  }

  TRACE_LOG("Updating list_of_avaialable_locales_codes.\n");
  new_locale_names = get_names_in_stringmap(new_module->messages_by_localcode);
  i = 0;
  while (new_locale_names[i] != NULL) {
    TRACE_LOG("Checking for new locale name \"");
    TRACE_LOG_Z_UCS(new_locale_names[i]);
    TRACE_LOG("\n.");
    locale_name_found = false;
    for (j=0; j<get_list_size(list_of_avaialable_locales_codes); j++) {
      locale_name = (z_ucs*)get_list_element(
          list_of_avaialable_locales_codes, j);
      TRACE_LOG("Testing against already known locale name \"");
      TRACE_LOG_Z_UCS(locale_name);
      TRACE_LOG("\".");
      if (z_ucs_cmp(locale_name, new_locale_names[i]) == 0) {
        TRACE_LOG("Match found.\n");
        locale_name_found = true;
        break;
      }
    }
    if (locale_name_found != true) {
      TRACE_LOG("No match found, adding to list.\n");
      add_list_element(list_of_avaialable_locales_codes, new_locale_names[i]);
    }
    i++;
  }
  TRACE_LOG("Finished updating list_of_avaialable_locales_codes.\n");
  free (new_locale_names);

  return 0;
}


void register_i18n_stream_output_function(
    int (*new_stream_output_function)(z_ucs *output)) {
  stream_output_function = new_stream_output_function;
}


void register_i18n_abort_function(
    void (*new_abort_function)(int exit_code, z_ucs *error_message)) {
  abort_function = new_abort_function;
}


static int i18n_send_output(z_ucs *z_ucs_data, int output_mode,
    z_ucs **string_target) {
  char *output;

  if (z_ucs_data == NULL) {
    return 0;
  }
  else if (output_mode == i18n_OUTPUT_MODE_STREAMS) {
    if (stream_output_function == NULL) {
      if ((output = dup_zucs_string_to_utf8_string(z_ucs_data)) == NULL) {
        return -1;
      }
      fputs(output, stdout);
      free(output);
      return 0;
    }
    else {
      return stream_output_function(z_ucs_data);
    }
  }
  else if (output_mode == i18n_OUTPUT_MODE_STRING) {
    (void)z_ucs_cpy(*string_target, z_ucs_data);
    *string_target += z_ucs_len(z_ucs_data);
    return 0;
  }
  else if (output_mode == i18n_OUTPUT_MODE_DEV_NULL) {
    return 0;
  }
  else {
    return -1;
  }
}


// Returns null if not found.
static locale_module *get_locale_module(z_ucs *module_name) {
  TRACE_LOG("Getting locale module '");
  TRACE_LOG_Z_UCS(module_name);
  TRACE_LOG("'.\n");

  if (locale_modules == NULL) {
    return NULL;
  }

  return (locale_module*)get_stringmap_value(locale_modules, module_name);
}


static locale_messages *get_messages_for_module_and_locale(z_ucs *module_name,
    z_ucs *locale_name) {
  locale_module *module = NULL;

  TRACE_LOG("Getting messages for module '");
  TRACE_LOG_Z_UCS(module_name);
  TRACE_LOG("' for locale '");
  TRACE_LOG_Z_UCS(locale_name);
  TRACE_LOG("'.\n");

  if ((module_name == NULL) || (locale_name == NULL)) {
    return NULL;
  }

  if ((module = get_locale_module(module_name)) == NULL) {
    return NULL;
  }

  return (locale_messages*)get_stringmap_value(
      module->messages_by_localcode, locale_name);
}


static void i18n_exit(int exit_code, z_ucs *error_message) {
  char *output;

  if ( (error_message == NULL) || (*error_message == 0) ) {
    error_message = i18n_fallback_error_message;
  }

  if (abort_function != NULL) {
    TRACE_LOG("Aborting using custom abort function.\n");
    abort_function(exit_code, error_message);
  }
  else {
    TRACE_LOG("Aborting using default method: fputs and exit(code).\n");
    if ((output = dup_zucs_string_to_utf8_string(error_message)) != NULL) {
      fputs(output, stderr);
      free(output);
    }
  }

  exit(exit_code);
}


char *get_default_locale_name() {
  if (default_locale_name_in_utf8 == NULL) {
    if ((default_locale_name_in_utf8
          = dup_zucs_string_to_utf8_string(default_locale_name)) == NULL) {
      return NULL;
    }
  }

  return default_locale_name_in_utf8;
}


// "string_code" is one of the codes defined in "utf8.h".
// "ap" is the va_list initialized in the various i18n-methods.
// "output_mode" is either "i18n_OUTPUT_MODE_DEV_NULL" for no output
//  at all (useful for string length measuring), "i18n_OUTPUT_MODE_STREAMS"
//  for sending output to "streams_utf8_output" and "i18n_OUTPUT_MODE_STRING"
//  to write the output to a string.
static long i18n_translate_from_va_list(z_ucs *module_name, int string_code,
    va_list ap, int output_mode, z_ucs *string_target) {
  z_ucs *locale_name;
  locale_messages *messages;
  z_ucs *index;
  char parameter_types[11]; // May each contain 's', 'z' or 'd'. Using 11
                            // instead of ten so that a null byte may be
                            // placed after a char to print the error
                            // message as string.
  char *string_parameters[10]; // pointers to the parameters.
  z_ucs *z_ucs_string_parameters[10];
  char formatted_parameters[10][MAXIMUM_FORMATTED_PARAMTER_LENGTH + 1];
  z_ucs *start;
  uint8_t match_stage;
  int i,k;
  int n;
  z_ucs buf;
  size_t length;
  z_ucs *start_index;
  char index_char;
  z_ucs z_ucs_buffer[LATIN1_TO_Z_UCS_BUFFER_SIZE];
  char *ptr;

  locale_name
    = current_locale_name != NULL
    ? current_locale_name
    : default_locale_name;

  TRACE_LOG("Trying to get messages for module '");
  TRACE_LOG_Z_UCS(module_name);
  TRACE_LOG("' and locale '");
  TRACE_LOG_Z_UCS(locale_name);
  TRACE_LOG("'.\n");

  if ((messages = get_messages_for_module_and_locale(
          module_name, locale_name)) == NULL) {
    TRACE_LOG("Messages not found.\n");
    i18n_exit(-1, NULL);
  }

  TRACE_LOG("Got messages at %p with %d messages.\n",
      module, messages->nof_messages, module->messages);

  if (string_code >= messages->nof_messages) {
    TRACE_LOG("String %d code too large, exiting.\n", string_code);
    i18n_exit(-1, NULL);
  }

  index = messages->messages[string_code];

  if (index == NULL) {
    return -1;
  }

  TRACE_LOG("Translating string code %d at %p.\n", string_code, index);

  n = 0;
  while ((index = z_ucs_chr(index, (z_ucs)'\\')) != NULL) {
    index++;
    start_index = index;
    index_char = zucs_char_to_latin1_char(*index);

    if (index_char == '{') {
      TRACE_LOG("'{' found.\n");
      index++;
      index_char = zucs_char_to_latin1_char(*index);
    }
    else {
      index = start_index;
      continue;
    }

    if ((index_char >= '0') && (index_char <= '9')) {
      TRACE_LOG("'[0-9]' found.\n");
      index++;
      index_char = zucs_char_to_latin1_char(*index);
    }
    else {
      index = start_index;
      continue;
    }

    if (
        (index_char == 's')
        ||
        (index_char == 'z')
        ||
        (index_char == 'd')
        ||
        (index_char == 'x')
       ) {
      TRACE_LOG("'[szdx]' found.\n");
      parameter_types[n] = index_char;
      index++;
      index_char = zucs_char_to_latin1_char(*index);
    }
    else {
      index = start_index;
      continue;
    }

    if (index_char == '}') {
      TRACE_LOG("'}' found.\n");
      index++;
      index_char = zucs_char_to_latin1_char(*index);
      n++;
    }
    else {
      index = start_index;
      continue;
    }
  }

  TRACE_LOG("Found %d parameter codes.\n", n);

  if (n == 0) {
    TRACE_LOG("No parameter.\n");
    // In case we don't have a single parameter, we can just print
    // everything right away and quit.

    if (i18n_send_output(
          messages->messages[string_code],
          output_mode,
          (string_target != NULL ? &string_target : NULL)) != 0) {
      return -1;
    }
    else {
      return z_ucs_len(messages->messages[string_code]);
    }
  }

  length = 0;

  for (i=0; i<n; i++) {
    // parameter_types[0-n] are always defined, thus using "usedef" is okay.

    if (parameter_types[i] == 's') {
      string_parameters[i] = va_arg(ap, char*);
      TRACE_LOG("p#%d: %s\n", i, string_parameters[i]);
      length += strlen(string_parameters[i]);
    }
    else if (parameter_types[i] == 'z') {
      z_ucs_string_parameters[i] = va_arg(ap, z_ucs*);
      length += z_ucs_len(z_ucs_string_parameters[i]);
    }
    else if (parameter_types[i] == 'd') {
      (void)snprintf(formatted_parameters[i],
          MAXIMUM_FORMATTED_PARAMTER_LENGTH,
          "%ld",
          (long)va_arg(ap, long));
      TRACE_LOG("p#%d: %s\n", i, formatted_parameters[i]);
      length += strlen(formatted_parameters[i]);
    }
    else if (parameter_types[i] == 'x') {
      (void)snprintf(formatted_parameters[i],
          MAXIMUM_FORMATTED_PARAMTER_LENGTH,
          "%lx",
          (unsigned long)va_arg(ap, long));
      length += strlen(formatted_parameters[i]);
    }
    else {
      TRACE_LOG("Invalid parameter type: %c.\n", parameter_types[i]);
      parameter_types[i+1] = '\0';
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_INVALID_PARAMETER_TYPE_P0S,
          -1,
          parameter_types+i);
    }
  }

  TRACE_LOG("Length: %zd.\n", length);

  start = messages->messages[string_code];
  i = 0;
  match_stage = 0;

  while (start[i] != 0) {
    if (match_stage == 1) {
      // We've already found a leading backslash.

      if ((start[i] == Z_UCS_BACKSLASH) && (match_stage == 1)) {
        // Found another backslash, so output.
        (void)latin1_string_to_zucs_string(
            z_ucs_buffer,
            "\\",
            LATIN1_TO_Z_UCS_BUFFER_SIZE);

        if (i18n_send_output(z_ucs_buffer,
              output_mode,
              (string_target != NULL ? &string_target : NULL))
            != 0) {
          i18n_translate_and_exit(
              libfizmo_module_name,
              i18n_libfizmo_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
              -1,
              "i18n_send_output");
        }

        match_stage = 0;
        length++;
        i++;
      }

      else if (start[i] == (z_ucs)'{') {
        // Here we've found a parameter. First, output everything up to
        // the parameter excluding "\{". In order to achive that, we'll
        // replace the first byte that shouldn't be printed with the
        // string-terminating 0 and restore it after that for the next
        // use of this message.
        buf = start[i-1];
        start[i-1] = 0;
        if (i18n_send_output(
              start,
              output_mode,
              (string_target != NULL ? &string_target : NULL))
            != 0) {
          i18n_translate_and_exit(
              libfizmo_module_name,
              i18n_libfizmo_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
              -1,
              "i18n_send_output");
        }

        length += z_ucs_len(start);
        start[i-1] = buf;

        // After that, output parameter (subtract 0x30 == ASCII:'0')
        k = (int)(start[i+1] - 0x30);

        if (parameter_types[k] == 's') {
          ptr = string_parameters[k];
          TRACE_LOG("%s\n", ptr);
          while (ptr != NULL) {
            ptr = latin1_string_to_zucs_string(
                z_ucs_buffer,
                ptr,
                LATIN1_TO_Z_UCS_BUFFER_SIZE);

            if (i18n_send_output(
                  z_ucs_buffer,
                  output_mode,
                  (string_target != NULL ? &string_target : NULL))
                !=0 ) {
              i18n_translate_and_exit(
                  libfizmo_module_name,
                  i18n_libfizmo_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
                  -1,
                  "i18n_send_output");
            }
          }
        }
        else if (parameter_types[k] == 'z') {
          if (i18n_send_output(
                z_ucs_string_parameters[k],
                output_mode,
                (string_target != NULL ? &string_target : NULL))
              != 0) {
            i18n_translate_and_exit(
                libfizmo_module_name,
                i18n_libfizmo_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
                -1,
                "i18n_send_output");
          }
        }
        else {
          ptr = formatted_parameters[k];
          while (ptr != NULL) {
            ptr = latin1_string_to_zucs_string(
                z_ucs_buffer,
                ptr,
                LATIN1_TO_Z_UCS_BUFFER_SIZE);

            if (i18n_send_output(
                  z_ucs_buffer,
                  output_mode,
                  (string_target != NULL ? &string_target : NULL))
                != 0) {
              i18n_translate_and_exit(
                  libfizmo_module_name,
                  i18n_libfizmo_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
                  -1,
                  "i18n_send_output");
            }
          }
        }

        start += i + 4;
        i = 0;
        match_stage = 0;
      }
      else {
        i18n_translate_and_exit(
            libfizmo_module_name,
            i18n_libfizmo_INVALID_BACKSLASH_SEQUENCE_IN_LOCALIZATION_DATA,
            -1);
      }
    }
    else {
      if ((start[i] == Z_UCS_BACKSLASH) && (match_stage == 0)) {
        // Found leading backslash;
        match_stage = 1;
        i++;
      }
      else {
        // Found nothing, next char (non memchar since operating on z_ucs)
        i++;
      }
    }
  }

  if (i != 0) {
    if (i18n_send_output(
          start,
          output_mode,
          (string_target != NULL ? &string_target : NULL))
        != 0) {
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
          -1,
          "i18n_send_output");
    }
  }

  length += z_ucs_len(start);

  TRACE_LOG("Final length:%zd.\n", length);

  return length;
}


void i18n_translate_and_exit(
    z_ucs *module_name,
    int string_code,
    int exit_code,
    ...)
{
  va_list ap;
  size_t message_length;
  z_ucs *error_message;

  TRACE_LOG("Exiting with message code %d.\n", string_code);

  va_start(ap, exit_code);
  message_length = i18n_translate_from_va_list(
      module_name,
      string_code,
      ap,
      i18n_OUTPUT_MODE_DEV_NULL,
      NULL);
  va_end(ap);

  TRACE_LOG("Message length: %zu.\n", message_length);

  if ((error_message = (z_ucs*)malloc((message_length+3) * sizeof(z_ucs)))
      == NULL) {
    return;
  }
    
  TRACE_LOG("Error message at: %p.\n", error_message);

  va_start(ap, exit_code);
  // Translate message into "error_message":
  if (i18n_translate_from_va_list(
        module_name,
        string_code,
        ap,
        i18n_OUTPUT_MODE_STRING,
        error_message)
      < 0) {
    TRACE_LOG("Code < 0\n");
    free(error_message);
    i18n_exit(exit_code, NULL);
  }
  va_end(ap);

  TRACE_LOG("Exit message: \"");
  // error_message is now defined:
  TRACE_LOG_Z_UCS(error_message);
  TRACE_LOG("\".\n");

  error_message[message_length] = Z_UCS_NEWLINE;
  error_message[message_length+1] = 0;

  i18n_exit(exit_code, error_message);
}


static size_t _i18n_va_translate(z_ucs *module_name, int string_code, va_list ap)
{
  return i18n_translate_from_va_list(
      module_name,
      string_code,
      ap,
      i18n_OUTPUT_MODE_STREAMS,
      NULL);
}


size_t i18n_translate(z_ucs *module_name, int string_code, ...)
{
  va_list ap;
  size_t result;
  
  va_start(ap, string_code);
  result = _i18n_va_translate(module_name, string_code, ap); 
  va_end(ap);

  return result;
}


size_t i18n_message_length(z_ucs *module_name, int string_code, ...)
{
  va_list ap;
  size_t result;

  va_start(ap, string_code);

  result
    = i18n_translate_from_va_list(
        module_name,
        string_code,
        ap,
        i18n_OUTPUT_MODE_DEV_NULL,
        NULL);

  va_end(ap);

  return result;
}


// Returnes malloc()ed string which needs to be free()d later on.
z_ucs *i18n_translate_to_string(z_ucs* module_name, int string_code, ...)
{
  va_list ap;
  size_t message_length;
  z_ucs *result;

  va_start(ap, string_code);
  if ((message_length = i18n_translate_from_va_list(
          module_name,
        string_code,
        ap,
        i18n_OUTPUT_MODE_DEV_NULL,
        NULL)) < 1)
    return NULL;
  va_end(ap);

  if ((result = (z_ucs*)malloc((message_length+1) * sizeof(z_ucs))) == NULL) {
    return NULL;
  }

  va_start(ap, string_code);
  // The "i18n_translate_from_va_list"-call defines "result".
  if (i18n_translate_from_va_list(
        module_name,
        string_code,
        ap,
        i18n_OUTPUT_MODE_STRING,
        result) == -1) {
    free(result);
    return NULL;
  }
  va_end(ap);

  return result;
}


z_ucs **get_available_locale_names() {
  return (z_ucs**)get_list_null_terminated_ptrs(
      list_of_avaialable_locales_codes);
}


z_ucs *get_current_locale_name() {
  return current_locale_name != NULL
    ? current_locale_name
    : default_locale_name;
}


char *get_current_locale_name_in_utf8() {
  return current_locale_name_in_utf8 != NULL
    ? current_locale_name_in_utf8
    : get_default_locale_name();
}


int set_current_locale_name(char *new_locale_name) {
  char *locale_dir_name = NULL;
  z_ucs *locale_dup;
  char *locale_dup_utf8;
  int i, j;

  if (new_locale_name == NULL) {
    return -1;
  }

  // Check if the locale name given was an alias: If, in case "en" was
  // given as locale name, this should be interpreted as "en_GB" instead --
  // see the "locale_aliases" definition above.
  i = 0;
  while (locale_aliases[i][0] != NULL) {
    // Test all aliases for current locale name:
    j = 1;
    while (locale_aliases[i][j] != NULL) {
      if (strcmp(locale_aliases[i][j], new_locale_name) == 0)
        break;
      j++;
    }

    if (locale_aliases[i][j] != NULL) {
      // We've found an alias.
      new_locale_name = locale_aliases[i][0];
      TRACE_LOG("Locale name \"%s\" is an alias for \"%s\".\n",
          new_locale_name, new_locale_name);
      break;
    }

    i++;
  }

  if ((locale_dup = dup_utf8_string_to_zucs_string(new_locale_name)) == NULL) {
    return -1;
  }

  if ((locale_dup_utf8 = strdup(new_locale_name)) == NULL) {
    free(locale_dup);
    free(locale_dup_utf8);
    return -1;
  }

  if (current_locale_name != NULL) {
    free(current_locale_name);
    free(current_locale_name_in_utf8);
  }

  current_locale_name = locale_dup;
  current_locale_name_in_utf8 = strdup(locale_dup_utf8);

  TRACE_LOG("New locale name: '");
  TRACE_LOG_Z_UCS(current_locale_name);
  TRACE_LOG("'.\n");
  return 0;
}


void free_i18n_memory(void) {
  if (locale_modules != NULL) {
    free(locale_modules);
    locale_modules = NULL;
  }

  if (current_locale_name != NULL) {
    free(current_locale_name);
    current_locale_name = NULL;
  }

  if (current_locale_name_in_utf8 != NULL) {
    free(current_locale_name_in_utf8);
    current_locale_name_in_utf8 = NULL;
  }

  if (default_locale_name_in_utf8 == NULL) {
    free(default_locale_name_in_utf8);
    default_locale_name_in_utf8 = NULL;
  }
}

#endif /* i18n_c_INCLUDED */

