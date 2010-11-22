
/* config.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2009-2010 Christoph Ender.
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef config_c_INCLUDED 
#define config_c_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "../tools/tracelog.h"
#include "config.h"
#include "fizmo.h"
#include "math.h"
#include "output.h"
#include "../tools/types.h"
#include "../tools/i18n.h"
#include "../locales/libfizmo_locales.h"

#define BUFSIZE 5

//int system_charset = SYSTEM_CHARSET_ASCII;
bool auto_adapt_upper_window = true;
bool auto_open_upper_window = true;
bool skip_active_routines_stack_check_warning = false;
char true_value[] = "true";
char false_value[] = "false";
char empty_string[] = "";


struct configuration_option
{
  char *name;
  char *value;
};


struct configuration_option configuration_options[] = {

  // String values:
  { "background-color", NULL },
  { "command-filename", NULL },
  { "foreground-color", NULL },
  { "i18n-search-path", NULL },
  { "locale", NULL },
  { "random-mode", NULL },
  { "savegame-path", NULL },
  { "transcript-filename", NULL },
  { "z-code-path", NULL },

  // Boolean values:
  { "disable-color", NULL },
  { "disable-external-streams", NULL },
  { "disable-restore", NULL },
  { "disable-save", NULL },
  { "disable-sound", NULL },
  { "enable-color", NULL },
  { "enable-font3-conversion", NULL },
  { "quetzal-umem", NULL },
  { "restore-after-save-and-quit-file-before-read", NULL },
  { "save-and-quit-file-before-read", NULL },
  { "set-tandy-flag", NULL },
  { "start-command-recording-when-story-starts", NULL },
  { "start-file-input-when-story-starts", NULL },
  { "start-script-when-story-starts", NULL },
  { "sync-transcript", NULL },

  // NULL terminates the option list.
  { NULL, NULL }

};


static char *expand_configuration_value(char *unexpanded_value)
{
  static char *homedir = NULL;
  static int homedir_len = -1;
  struct passwd *pw_entry;
  char *ptr = unexpanded_value;
  int resultlen;
  char *result, *resultindex;
  char *var_name;
  char buf;

  if (unexpanded_value == NULL)
    return NULL;

  if (homedir == NULL)
  {
    pw_entry = getpwuid(getuid());
    if (pw_entry->pw_dir == NULL)
      homedir = empty_string;
    else
      homedir = strdup(pw_entry->pw_dir);
    homedir_len = strlen(homedir);
  }

  resultlen = 0;
  while (*ptr != 0)
  {
    if (*ptr == '$')
    {
      ptr++;
      if (*ptr == '(')
      {
        ptr++;
        var_name = ptr;
        while ( (*ptr != 0) && (*ptr != ')') )
          ptr++;
        if (*ptr != ')')
          return NULL;
        buf = *ptr;
        *ptr = 0;

        if (strcasecmp(var_name, "HOME") == 0)
        {
          resultlen += strlen(homedir);
        }
        else
        {
          *ptr = buf;
          return NULL;
        }

        *ptr = buf;
        ptr++;
      }
      else
        return NULL;
    }
    else
    {
      ptr++;
      resultlen++;
    }
  }

  TRACE_LOG("result len: %d.\n", resultlen);
  result = fizmo_malloc(resultlen + 1);
  resultindex = result;

  ptr = unexpanded_value;
  while (*ptr != 0)
  {
    if (*ptr == '$')
    {
      ptr++;
      if (*ptr == '(')
      {
        ptr++;
        var_name = ptr;
        while ( (*ptr != 0) && (*ptr != ')') )
          ptr++;
        if (*ptr != ')')
          return NULL;
        buf = *ptr;
        *ptr = 0;

        if (strcasecmp(var_name, "HOME") == 0)
        {
          strcpy(resultindex, homedir);
          resultindex += homedir_len;
        }
        else
        {
          *ptr = buf;
          // Can't ever reach this point due to loop 1.
        }

        *ptr = buf;
        ptr++;
      }
    }
    else
    {
      *resultindex = *ptr;
      ptr++;
      resultindex++;
    }
  }

  *resultindex = 0;

  TRACE_LOG("result expanded value: %s.\n", result);
  return result;
}


int set_configuration_value(char *key, char* new_unexpanded_value)
{
  int i, return_code;
  char *current_value, *new_value = NULL;
  char buf[BUFSIZE];
  short color_code;

  if (key == NULL)
    return -1;

  if (new_unexpanded_value != NULL)
    if ((new_value = expand_configuration_value(new_unexpanded_value)) == NULL)
      return -1;

  i = 0;

  while (configuration_options[i].name != NULL)
  {

    if (strcmp(configuration_options[i].name, key) == 0)
    {
      // Parse option values which cannot be simply copied:

      if (strcasecmp(key, "locale") == 0)
      {
        set_current_locale_name(new_value);
        free(new_value);
        return 0;
      }
      else if (strcasecmp(key, "random-mode") == 0)
      {
        if (new_value == NULL)
          return -1;
        else if (strcasecmp(new_value, "random") == 0)
        {
          if (configuration_options[i].value != NULL)
            free(configuration_options[i].value);
          configuration_options[i].value = new_value;
          seed_random_generator();
        }
        else if (strcasecmp(new_value, "predictable") == 0)
        {
          if (configuration_options[i].value != NULL)
            free(configuration_options[i].value);
          configuration_options[i].value = new_value;
          seed_random_generator();
        }
        else
          return -1;
      }
      else if (strcasecmp(key, "i18n-search-path") == 0)
      {
        // Forward to i18n, since this is in tools and cannot access the
        // "config.c" file.
        return_code = set_i18n_search_path(new_value);
        free(new_value);
        return return_code;
      }

      // Options for values which can simply be copied.
      else if (
          (strcasecmp(key, "z-code-path") == 0)
          ||
          (strcasecmp(key, "savegame-path") == 0)
          ||
          (strcasecmp(key, "transcript-filename") == 0)
          ||
          (strcasecmp(key, "command-filename") == 0)
          )
      {
        if (configuration_options[i].value != NULL)
          free(configuration_options[i].value);
        configuration_options[i].value = new_value;
        return 0;
      }

      // Color options
      else if (strcasecmp(key, "foreground-color") == 0)
      {
        if ((color_code = color_name_to_z_colour(new_value)) == -1)
        {
          free(new_value);
          return -1;
        }
        free(new_value);
        if (snprintf(buf, BUFSIZE, "%d", color_code) >= BUFSIZE)
          return -1;
        configuration_options[i].value = fizmo_strdup(buf);
        default_foreground_colour = color_code;
        return 0;
      }
      else if (strcasecmp(key, "background-color") == 0)
      {
        if ((color_code = color_name_to_z_colour(new_value)) == -1)
        {
          free(new_value);
          return -1;
        }
        free(new_value);
        if (snprintf(buf, BUFSIZE, "%d", color_code) >= BUFSIZE)
          return -1;
        configuration_options[i].value = fizmo_strdup(buf);
        default_background_colour = color_code;
        return 0;
      }

      // Non-primitive boolean options
      else if (strcasecmp(key, "enable-color") == 0)
      {
        if (
            (new_value == NULL)
            ||
            (*new_value == 0)
            ||
            (strcasecmp(new_value, true_value) == 0)
           )
        {
          free(new_value);
          current_value = get_configuration_value("disable-color");
          if (strcasecmp(current_value, true_value) == 0)
            return -1;
          configuration_options[i].value = fizmo_strdup(true_value);
          return 0;
        }
        else if ((new_value != NULL) && (strcasecmp(new_value, false_value)==0))
        {
          free(new_value);
          if (configuration_options[i].value != NULL)
            free(configuration_options[i].value);
          configuration_options[i].value = fizmo_strdup(false_value);
          return -1;
        }
        else
        {
          free(new_value);
          return -1;
        }
      }
      else if (strcasecmp(key, "disable-color") == 0)
      {
        if (
            (new_value == NULL)
            ||
            (*new_value == 0)
            ||
            (strcasecmp(new_value, true_value) == 0)
           )
        {
          free(new_value);
          current_value = get_configuration_value("enable-color");
          if (strcasecmp(current_value, true_value) == 0)
            return -1;
          configuration_options[i].value = fizmo_strdup(true_value);
          return 0;
        }
        else if ((new_value != NULL) && (strcasecmp(new_value, false_value)==0))
        {
          free(new_value);
          if (configuration_options[i].value != NULL)
            free(configuration_options[i].value);
          configuration_options[i].value = fizmo_strdup(false_value);
          return -1;
        }
        else
        {
          free(new_value);
          return -1;
        }
      }

      // Boolean options
      else if (
          (strcasecmp(key, "save-and-quit-file-before-read") == 0)
          ||
          (strcasecmp(key, "disable-save") == 0)
          ||
          (strcasecmp(key, "disable-restore") == 0)
          ||
          (strcasecmp(key, "restore-after-save-and-quit-file-before-read") == 0)
          ||
          (strcasecmp(key, "disable-external-streams") == 0)
          ||
          (strcasecmp(key, "start-script-when-story-starts") == 0)
          ||
          (strcasecmp(key, "disable-sound") == 0)
          ||
          (strcasecmp(key, "enable-font3-conversion") == 0)
          ||
          (strcasecmp(key, "sync-transcript") == 0)
          ||
          (strcasecmp(key, "start-command-recording-when-story-starts") == 0)
          ||
          (strcasecmp(key, "command-filename") == 0)
          ||
          (strcasecmp(key, "start-file-input-when-story-starts") == 0)
          ||
          (strcasecmp(key, "random-mode") == 0)
          ||
          (strcasecmp(key, "quetzal-umem") == 0)
          ||
          (strcasecmp(key, "set-tandy-flag") == 0)
          )
      {
        if (
            (new_value == NULL)
            ||
            (*new_value == 0)
            ||
            (strcasecmp(new_value, true_value) == 0)
           )
        {
          if (new_value != NULL)
            free(new_value);
          if (configuration_options[i].value != NULL)
            free(configuration_options[i].value);
          configuration_options[i].value = fizmo_strdup(true_value);
          return 0;
        }
        else if ((new_value != NULL) && (strcasecmp(new_value, false_value)==0))
        {
          free(new_value);
          if (configuration_options[i].value != NULL)
            free(configuration_options[i].value);
          configuration_options[i].value = fizmo_strdup(false_value);
          return -1;
        }
        else
        {
          free(new_value);
          return -1;
        }
      }
      else
      {
        i18n_translate_and_exit(
            libfizmo_module_name,
            i18n_libfizmo_UNKNOWN_CONFIGURATION_OPTION_P0S,
            -0x0101,
            key);
      } 
    }

    i++;
  }

  return -1;
}


char *get_configuration_value(char *key)
{
  int i = 0;

  if (key == NULL)
    return NULL;

  TRACE_LOG("Retrieving config value: %s.\n", key);

  if (strcmp(key, "i18n-search-path") == 0)
  {
    // Forward to i18n, since this is in tools and cannot access the
    // "config.c" file.
    return get_i18n_search_path();
  }
  else
  {
    while (configuration_options[i].name != NULL)
    {
      if (strcmp(configuration_options[i].name, key) == 0)
      {
        // Boolean options
        if (
            (strcasecmp(key, "save-and-quit-file-before-read") == 0)
            ||
            (strcasecmp(key, "disable-save") == 0)
            ||
            (strcasecmp(key, "disable-restore") == 0)
            ||
            (strcasecmp(key, "restore-after-save-and-quit-file-before-read")==0)
            ||
            (strcasecmp(key, "disable-external-streams") == 0)
            ||
            (strcasecmp(key, "start-script-when-story-starts") == 0)
            ||
            (strcasecmp(key, "disable-sound") == 0)
            ||
            (strcasecmp(key, "enable-font3-conversion") == 0)
            ||
            (strcasecmp(key, "sync-transcript") == 0)
            ||
            (strcasecmp(key, "start-command-recording-when-story-starts") == 0)
            ||
            (strcasecmp(key, "command-filename") == 0)
            ||
            (strcasecmp(key, "start-file-input-when-story-starts") == 0)
            ||
            (strcasecmp(key, "random-mode") == 0)
            ||
            (strcasecmp(key, "quetzal-umem") == 0)
            ||
            (strcasecmp(key, "set-tandy-flag") == 0)
            ||
            (strcasecmp(key, "enable-color") == 0)
            ||
            (strcasecmp(key, "disable-color") == 0)
           )
        {
          if (configuration_options[i].value == NULL)
          {
            TRACE_LOG("return \"false\".\n");
            return false_value;
          }
          else
          {
            TRACE_LOG("return \"%s\".\n", configuration_options[i].value);
            return configuration_options[i].value;
          }
        }
      }

      // String options
      else if (
          (strcasecmp(key, "locale") == 0)
          ||
          (strcasecmp(key, "random-mode") == 0)
          ||
          (strcasecmp(key, "z-code-path") == 0)
          ||
          (strcasecmp(key, "savegame-path") == 0)
          ||
          (strcasecmp(key, "transcript-filename") == 0)
          ||
          (strcasecmp(key, "command-filename") == 0)
          ||
          (strcasecmp(key, "foreground-color") == 0)
          ||
          (strcasecmp(key, "background-color") == 0)
         )
      {
        return configuration_options[i].value;
      }

      i++;
    }
    return NULL;
  }
}


/*
char **get_valid_configuration_options(char *key, ...)
{
  char **result;
  int result_index;
  size_t memory_size;
  va_list ap;
  char *ptr;
  int i;

  if (strcmp(key, "locale") == 0)
  {
    va_start(ap, key);
    ptr = va_arg(ap, char*);
    va_end(ap);

    if (ptr == NULL)
      return NULL;

    result_index = 0;
    for (i=0; i<NUMBER_OF_LOCALES; i++)
      if (strcmp(locales[i].module_name, "fizmo") == 0)
        result_index++;

    memory_size = (result_index + 1) * sizeof(char*);

    result = (char**)fizmo_malloc(memory_size);

    result_index = 0;
    for (i=0; i<NUMBER_OF_LOCALES; i++)
      if (strcmp(locales[i].module_name, "fizmo") == 0)
        result[result_index++] = fizmo_strdup(locales[i].language_code);

    result[result_index] = NULL;

    return result;
  }
  else
    return NULL;
}
*/


#endif /* config_c_INCLUDED */

