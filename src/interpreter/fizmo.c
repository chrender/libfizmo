
/* fizmo.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2009-2011 Christoph Ender.
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


#ifndef fizmo_c_INCLUDED 
#define fizmo_c_INCLUDED

#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "fizmo.h"
#include "zpu.h"
#include "text.h"
#include "math.h"
#include "stack.h"
#include "misc.h"
#include "streams.h"
#include "config.h"
#include "savegame.h"
#include "output.h"
#include "iff.h"
#include "filelist.h"
#include "routine.h"
#include "variable.h"
#include "../tools/z_ucs.h"
#include "../tools/types.h"
#include "../tools/i18n.h"
#include "../tools/tracelog.h"
#include "../tools/filesys.h"
#include "../locales/libfizmo_locales.h"

#ifndef DISABLE_OUTPUT_HISTORY
#include "history.h"
#endif /* DISABLE_OUTPUT_HISTORY */

#ifndef DISABLE_BLOCKBUFFER
#include "blockbuf.h"
#endif // DISABLE_BLOCKBUFFER

#define MAX_CONFIG_OPTION_LENGTH 512


struct z_story *active_z_story;

struct z_screen_interface *active_interface = NULL;
struct z_sound_interface *active_sound_interface = NULL;

uint8_t ver = 0;
uint8_t *header_extension_table;
uint8_t header_extension_table_size;

//static int16_t maximum_z_story_size[]
//  = { 128, 128, 128, 256, 256, 512, 512, 512 };

static int enable_script_after_start = 0;

#ifndef DISABLE_CONFIGFILES
char *fizmo_config_dir_name = NULL;
char *xdg_config_home = NULL;
static char *default_xdg_config_home = DEFAULT_XDG_CONFIG_HOME;
static char *default_xdg_config_dirs = DEFAULT_XDG_CONFIG_DIRS;
static bool fizmo_config_dir_name_initialized = false;
static bool xdg_config_dir_name_initialized = false;
static bool config_files_were_parsed = false;
#endif // DISABLE_CONFIGFILES

#ifndef DISABLE_BLOCKBUFFER
/*@null@*/ BLOCKBUF *upper_window_buffer = NULL;
#endif // DISABLE_BLOCKBUFFER


//long size;


static void load_blorb(struct z_story *result, char* blorb_input_filename)
{
  //struct z_story *result;
  int resource_chunk_size;
  int nof_resources;
  int nof_execs, nof_loops;
  uint32_t resource_number, resource_offset, size;
  int images_parsed, sounds_parsed;
  struct z_story_blorb_sound *blorb_sound;
  char buf[5];
  int i;

  if (find_chunk("RIdx", result->blorb_file) == -1)
  {
    // FIXME: Better error message.
    fsi->closefile(result->blorb_file);
    exit(-1);
  }

  if (read_chunk_length(result->blorb_file) == -1)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
        -0x0101,
        "read_chunk_length",
        errno);

  resource_chunk_size = get_last_chunk_length();
  nof_resources = (resource_chunk_size - 4) / 12;

  // Skip next number of resources.
  if ((fsi->setfilepos(result->blorb_file, 4, SEEK_CUR)) != 0)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
        -0x0101,
        "setfilepos",
        errno);

  TRACE_LOG("Number of resources in blorb file: %d.\n", nof_resources);

  // Count number of images and sounds.
  nof_execs = 0;
  buf[4] = '\0';

  while (nof_resources > 0)
  {
    if (fsi->getchars(buf, 4, result->blorb_file) != 4)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_ERROR_WHILE_READING_FILE_P0S,
          -0x0106,
          blorb_input_filename);

    TRACE_LOG("Type descriptor: %s\n", buf);
    if (strcmp(buf, "Pict") == 0)
      result->nof_images++;
    else if (strcmp(buf, "Snd ") == 0)
      result->nof_sounds++;
    else if (strcmp(buf, "Exec") == 0)
      nof_execs++;
    else
      // Unknown resource.
      exit(-1);

    // Skip number and position.
    if ((fsi->setfilepos(result->blorb_file, 8, SEEK_CUR)) != 0)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
          -0x0101,
          "setfilepos",
          errno);

    nof_resources--;
  }

  TRACE_LOG("Images: %d, Sounds: %d, Execs: %d.\n",
      result->nof_images, result->nof_sounds, nof_execs);

  if ( (result->z_story_file != NULL) && (nof_execs != 1) )
  {
    // FIXME: Error message.
    exit(-1);
  }

  result->blorb_sounds = (struct z_story_blorb_sound*)fizmo_malloc(
      result->nof_sounds * sizeof(struct z_story_blorb_sound));

  result->blorb_images = (struct z_story_blorb_image*)fizmo_malloc(
      result->nof_images * sizeof(struct z_story_blorb_image));

  find_chunk("RIdx", result->blorb_file);

  // Skip chunk length and number of resources.
  if ((fsi->setfilepos(result->blorb_file, 8, SEEK_CUR)) != 0)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
        -0x0101,
        "setfilepos",
        errno);

  nof_resources = result->nof_images + result->nof_sounds + nof_execs;
  images_parsed = 0;
  sounds_parsed = 0;

  while (nof_resources > 0)
  {
    if (fsi->getchars(buf, 4, result->blorb_file) != 4)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_ERROR_WHILE_READING_FILE_P0S,
          -0x0106,
          blorb_input_filename);

    resource_number = read_four_byte_number(result->blorb_file);
    resource_offset = read_four_byte_number(result->blorb_file);

    TRACE_LOG("Parsing resource #%d.\n", resource_number);

    if (strcmp(buf, "Pict") == 0)
    {
      TRACE_LOG("Image %d at %d, index: %d.\n",
          resource_number, resource_offset, sounds_parsed);
      result->blorb_images[images_parsed].resource_number = resource_number;
      result->blorb_images[images_parsed].blorb_offset = resource_offset;
      images_parsed++;
    }
    else if (strcmp(buf, "Snd ") == 0)
    {
      TRACE_LOG("Sound %d at %d, index: %d.\n",
          resource_number, resource_offset, sounds_parsed);
      result->blorb_sounds[sounds_parsed].resource_number = resource_number;
      result->blorb_sounds[sounds_parsed].blorb_offset = resource_offset;
      result->blorb_sounds[sounds_parsed].v3_number_of_loops = 1;
      sounds_parsed++;
    }
    else if (strcmp(buf, "Exec") == 0)
    {
      TRACE_LOG("Executable %d at %d.\n", resource_number, resource_offset);
    }
    else
    {
      TRACE_LOG("Unknown resource \"%s\", number %d at %d.\n",
          buf, resource_number, resource_offset);
    }

    nof_resources--;
  }

  // Now we have saved all the offsets. The next step is to parse image
  // and sound types.
  for (i=0; i<result->nof_images; i++)
  {
    if ((fsi->setfilepos(result->blorb_file,
            result->blorb_images[i].blorb_offset,SEEK_SET))
        != 0)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
          -0x0101,
          "setfilepos",
          errno);

    if (fsi->getchars(buf, 4, result->blorb_file) != 4)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_ERROR_WHILE_READING_FILE_P0S,
          -0x0106,
          blorb_input_filename);

    size = read_four_byte_number(result->blorb_file);

    result->blorb_images[i].blorb_offset += 8;

    if (strcmp(buf, "PNG ") == 0)
    {
      result->blorb_images[i].type = Z_BLORB_IMAGE_PNG;
      result->blorb_images[i].size = size;
    }
    else if (strcmp(buf, "JPEG") == 0)
    {
      result->blorb_images[i].type = Z_BLORB_IMAGE_JPEG;
      result->blorb_images[i].size = size;
    }
    else
    {
      // FIXME: Blorb file specification gives no resource type for
      // a placeholder, so I'll assume everything else is a plcaeholder.
      result->blorb_images[i].type = Z_BLORB_IMAGE_PLACEHOLDER;
      result->blorb_images[i].size = size;
    }
  }

  for (i=0; i<result->nof_sounds; i++)
  {
    TRACE_LOG("Seeking %ld.\n", result->blorb_sounds[i].blorb_offset);
    if ((fsi->setfilepos(result->blorb_file,
            result->blorb_sounds[i].blorb_offset, SEEK_SET))
        != 0)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
          -0x0101,
          "setfilepos",
          errno);

    if (fsi->getchars(buf, 4, result->blorb_file) != 4)
    {
      TRACE_LOG("%s\n", strerror(errno));
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_ERROR_WHILE_READING_FILE_P0S,
          -0x0106,
          blorb_input_filename);
    }

    //TRACE_LOG("Sound type: \"%s\".\n", buf);

    size = read_four_byte_number(result->blorb_file);

    if (fsi->getchars(buf, 4, result->blorb_file) != 4)
    {
      TRACE_LOG("%s\n", strerror(errno));
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_ERROR_WHILE_READING_FILE_P0S,
          -0x0106,
          blorb_input_filename);
    }

    TRACE_LOG("Sound type: \"%s\".\n", buf);

    //result->blorb_sounds[i].blorb_offset += 8;

    if (strcmp(buf, "AIFF") == 0)
    {
      result->blorb_sounds[i].type = Z_BLORB_SOUND_AIFF;
      result->blorb_sounds[i].size = size;
    }
    else if (strcmp(buf, "OGGV") == 0)
    {
      result->blorb_sounds[i].type = Z_BLORB_SOUND_OGG;
      result->blorb_sounds[i].size = size;
    }
    else if (strcmp(buf, "MOD ") == 0)
    {
      result->blorb_sounds[i].type = Z_BLORB_SOUND_MOD;
      result->blorb_sounds[i].size = size;
    }
    else if (strcmp(buf, "SONG") == 0)
    {
      result->blorb_sounds[i].type = Z_BLORB_SOUND_SONG;
      result->blorb_sounds[i].size = size;
    }
    else
    {
      TRACE_LOG("Unknown sound type: \"%s\".\n", buf);
      // FIXME: Error message.

      // exit(-2);
      // No exit here, since http://www.ifarchive.org/if-archive/games/zcode/
      // french/scarabeekatana.zblorb contains a "OggS" sound type which will
      // cause game startup to abort at this point.

      result->blorb_sounds[i].type = Z_BLORB_SOUND_UNKNOWN;
      result->blorb_sounds[i].size = size;
    }
  }

  if ( (ver < 5) && (result->nof_sounds > 0) )
  {
    if (find_chunk("Loop", result->blorb_file) == 0)
    {
      nof_resources = read_four_byte_number(result->blorb_file) / 8;
      TRACE_LOG("Number of loop entries: %d.\n", nof_resources);

      for (i=0; i<nof_resources; i++)
      {
        resource_number = read_four_byte_number(result->blorb_file);
        nof_loops = read_four_byte_number(result->blorb_file);

        TRACE_LOG("Trying to find resource #%d.\n", resource_number);
        if ((blorb_sound = get_sound_blorb_index(
                result, resource_number)) != NULL)
        {
          TRACE_LOG("Resource found, setting nof_loops to %d.\n",
              nof_loops);
          blorb_sound->v3_number_of_loops = nof_loops;
        }
      }
    }
  }

  if (find_chunk("Fspc", result->blorb_file) == 0)
  {
    if ((fsi->setfilepos(result->blorb_file, 4, SEEK_CUR)) != 0)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
          -0x0101,
          "setfilepos",
          errno);

    result->frontispiece_image_no = read_four_byte_number(result->blorb_file);
  }
}


static struct z_story *load_z_story(char *input_filename, char *blorb_filename)
{
  struct z_story *result;
  int z_file_version;
  char *ptr, *ptr2;
  char *cwd = NULL;
  long len;
  long story_size = -1;
  long story_file_exec_offset = -1;
#ifndef DISABLE_FILELIST
  struct z_story_list_entry *story_data;
#endif

  result = (struct z_story*)fizmo_malloc(sizeof(struct z_story));

  result->nof_images = 0;
  result->nof_sounds = 0;
  result->blorb_file = NULL;
  result->frontispiece_image_no = -1;

  // First, check if the input file is a blorb file.
  result->z_story_file = open_simple_iff_file(input_filename, IFF_MODE_READ);

  if ( (result->z_story_file == NULL) && (blorb_filename != NULL) )
  {
    // In case the supplied code file is not in IFF format and a blorb
    // file was supplied, try to use the blorb file.
    result->blorb_file = open_simple_iff_file(blorb_filename, IFF_MODE_READ);
    TRACE_LOG("%p\n", result->blorb_file);
  }

  if ( (result->z_story_file != NULL) || (result->blorb_file != NULL) )
  {
    // We've got some IFF file supplied.

    if (result->z_story_file != NULL)
    {
      // Check if the supplied z-code IFF file actually contains code.
      if (
          (is_form_type(result->z_story_file, "IFRS") != true)
          ||
          (find_chunk("ZCOD", result->z_story_file) == -1)
         )
      {
        // FIXME: Better error message.
        fsi->closefile(result->z_story_file);
        exit(-1);
      }
      else
      {
        if (read_chunk_length(result->z_story_file) == -1)
          i18n_translate_and_exit(
              libfizmo_module_name,
              i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
              -0x0101,
              "read_chunk_length",
              errno);

        if ((story_file_exec_offset
              = fsi->getfilepos(result->z_story_file)) == -1)
          i18n_translate_and_exit(
              libfizmo_module_name,
              i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
              -0x0101,
              "getfilepos",
              errno);

        story_size = get_last_chunk_length();
      }

      // It's not possible to use two blorb files simultaneously: In case
      // our story file is a blorb file, we'll also have to load resources
      // from it.
      result->blorb_file = result->z_story_file;
    }

    load_blorb(result, input_filename);
  }

  if (result->z_story_file == NULL)
  {
    TRACE_LOG("Parsing raw z-code file.\n");

    if ((result->z_story_file
          = fsi->openfile(input_filename, FILETYPE_DATA, FILEACCESS_READ))
        == NULL)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_COULD_NOT_OPEN_FILE_NAMED_P0S,
          -0x0101,
          input_filename);

    // No IFF file, the game is supposed to be a zcode file.
    if ((fsi->setfilepos(result->z_story_file, 0, SEEK_END)) != 0)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
          -0x0101,
          "setfilepos",
          errno);

    if ((story_size = fsi->getfilepos(result->z_story_file)) == -1)
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
          -0x0101,
          "getfilepos",
          errno);

    story_file_exec_offset = 0;
  }

  ptr = fizmo_strdup(input_filename);
  if ((ptr2 = strrchr(ptr, '/')) != NULL)
  {
    *ptr2 = '\0';
    ptr2++;
    cwd = fsi->get_cwd();
    fsi->ch_dir(ptr);
  }
  else
  {
    ptr2 = ptr;
  }
  result->absolute_directory_name = fsi->get_cwd();
  len = strlen(result->absolute_directory_name) + strlen(ptr2) + 2;

  result->absolute_file_name = (char*)fizmo_malloc(len);

  strcpy(result->absolute_file_name, result->absolute_directory_name);
  strcat(result->absolute_file_name, "/");
  strcat(result->absolute_file_name, ptr2);
  if (cwd != NULL)
    fsi->ch_dir(cwd);
  free(cwd);

  result->story_file_exec_offset = story_file_exec_offset;

  if ((fsi->setfilepos(
          result->z_story_file, story_file_exec_offset, SEEK_SET)) != 0)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
        -0x0101,
        "setfilepos",
        errno);

  if ((z_file_version = fsi->getchar(result->z_story_file)) == EOF)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_ERROR_READING_FIRST_STORY_BYTE_FROM_P0S,
        -0x0102,
        input_filename);

  result->version = (uint8_t)z_file_version;
  TRACE_LOG("Game is version %d.\n", result->version);

  if ((result->version < 1) || (result->version > 8))
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_UNKNOWN_STORY_VERSION_P0D,
        -0x0103,
        (long int)result->version);

  /*
  // No size check: Even the original Beyond Zork breaks the size barrier.
  if (story_size > (long)maximum_z_story_size[result->version-1] * 1024l)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_STORY_FILE_OF_SIZE_P0D_TOO_LARGE_ONLY_P1D_BYTES_ALLOWED,
        -0x0104,
        (long int)story_size,
        (long int)maximum_z_story_size[result->version-1] * 1024l);
  */

  result->memory = (uint8_t*)fizmo_malloc((size_t)story_size);

  *(result->memory) = result->version;

  TRACE_LOG("Loading %li bytes from \"%s\".\n", story_size, input_filename);

  if (fsi->getchars(
        result->memory+1, (size_t)(story_size - 1), result->z_story_file)
      != (size_t)(story_size - 1))
  {
    if (fsi->closefile(result->z_story_file) == EOF)
      (void)i18n_translate(
          libfizmo_module_name,
          i18n_libfizmo_ERROR_WHILE_CLOSING_FILE_P0S,
          -0x0107,
          input_filename);

    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_ERROR_WHILE_READING_FILE_P0S,
        -0x0106,
        input_filename);
  }

  result->high_memory_end = result->memory + story_size - 1;

  //result->memory[0x1e] = 1;
  //result->memory[0x1f] = 2;

  result->static_memory = result->memory + load_word(result->memory + 0xe);

  result->dynamic_memory_end = result->static_memory - 1;

  result->static_memory_end
    = result->memory + (story_size > 0xffff ? 0xffff : story_size);

  result->high_memory = result->memory + load_word(result->memory + 0x4);

  result->global_variables = result->memory + load_word(result->memory + 0xc);

  if (result->version >= 6)
  {
    result->routine_offset= 8 * load_word(result->memory + 0x28);
    result->string_offset = 8 * load_word(result->memory + 0x2a);
  }
  else
  {
    result->routine_offset= 0;
    result->string_offset = 0;
  }

  result->abbreviations_table = result->memory + load_word(result->memory+0x18);

  result->property_defaults = result->memory + load_word(result->memory+0xa)-2;

  if (result->version <= 3)
  {
    result->object_size = 9;
    result->object_tree
      = result->memory
      + load_word(result->memory + 0xa)
      + 31*2
      - result->object_size;
    result->maximum_object_number = 255;
    result->maximum_property_number = 31;
    result->maximum_attribute_number = 31;
    result->object_node_number_index = 4;
    result->object_property_index = 7;
  }
  else
  {
    result->object_size = 14;
    result->object_tree
      = result->memory
      + load_word(result->memory + 0xa)
      + 63*2
      - result->object_size;
    result->maximum_object_number = 65535;
    result->maximum_property_number = 63;
    result->maximum_attribute_number = 48;
    result->object_node_number_index = 6;
    result->object_property_index = 12;
  }

  if (result->version == 1)
    result->alphabet_table = alphabet_table_v1;
  else if ((result->version >= 5) && (load_word(result->memory + 0x34)  != 0))
    result->alphabet_table = result->memory + load_word(result->memory + 0x34);
  else
    result->alphabet_table = alphabet_table_after_v1;

  result->dictionary_table = result->memory + load_word(result->memory+0x8);

  // 8.2.1: In Versions 1 and 2, all games are "score games". In Version 3,
  // if bit 1 of 'Flags 1' is clear then the game is a "score game"; if it
  // is set, then the game is a "time game". 
  if (result->version <= 2)
  {
    result->score_mode = SCORE_MODE_SCORE_AND_TURN;
  }
  else if (result->version == 3)
  {
    if ((result->memory[1] & 0x2) == 0)
      result->score_mode = SCORE_MODE_SCORE_AND_TURN;
    else
      result->score_mode = SCORE_MODE_TIME;
  }
  else
    result->score_mode = SCORE_MODE_UNKNOWN;

  if (enable_script_after_start != 0)
    result->memory[0x11] |= 1;

  result->release_code = load_word(result->memory + 0x2);
  memcpy(result->serial_code, result->memory+0x12, 6);
  result->serial_code[6] = (char)0;
  result->checksum = load_word(result->memory + 0x1c);

  header_extension_table = result->memory + load_word(result->memory + 0x36);
  header_extension_table_size = load_word(header_extension_table);

#ifdef ENABLE_TRACING
  if (active_sound_interface != NULL)
  { TRACE_LOG("Sound interface available.\n"); }
  else
  { TRACE_LOG("No sound interface available.\n"); }
#endif // ENABLE_TRACING

  if (result->version == 6)
  {
    if (
        (active_sound_interface != NULL)
        //&&
        //(strcmp(get_configuration_value("force-8bit-sound"), "true") != 0)
       )
      result->memory[1] |= 0x20;
  }
  else if (
      (
       (active_sound_interface == NULL)
       //||
       //(strcmp(get_configuration_value("force-8bit-sound"), "true") == 0)
      )
      &&
      (result->version >= 5)
      )
    result->memory[0x11] &= 0x7f;

  // No mouse currently implemented.
  result->memory[0x11] &= 0xdf;

  TRACE_LOG("flags2: %x.\n", result->memory[0x11]);

  if (result->version >= 6)
    result->max_nof_color_pairs = 11 * 11; // (2 to 12) ^2
  else if (result->version == 5)
    result->max_nof_color_pairs = 8 * 8; // (2 to 9) ^2
  else
    result->max_nof_color_pairs = 0;

#ifndef DISABLE_FILELIST
  detect_and_add_single_z_file(input_filename, blorb_filename);

  if ((story_data = get_z_story_entry_from_list(
        result->serial_code,
        result->release_code,
        result->checksum)) != NULL)
  {
    if ( (result->blorb_file == NULL) && (story_data->blorbfile != NULL) )
    {
      TRACE_LOG("Load blorb: %s\n", story_data->blorbfile);

      if ((result->blorb_file = open_simple_iff_file(
              story_data->blorbfile, IFF_MODE_READ)) != NULL)
        load_blorb(result, story_data->blorbfile);
    }
    result->title = fizmo_strdup(story_data->title);
    free_z_story_list_entry(story_data);
  }
  else
  {
    result->title = NULL;
  }
#else
  result->title = NULL;
#endif

  return result;
}


struct z_story_blorb_image *get_image_blorb_index(struct z_story *story,
    int resource_number)
{
  int i;

  if ( (resource_number < 0) || (resource_number > story->nof_images) )
    return NULL;

  for (i=0; i<story->nof_images; i++)
    if (story->blorb_images[i].resource_number == resource_number)
      return &story->blorb_images[i];

  return NULL;
}


struct z_story_blorb_sound *get_sound_blorb_index(struct z_story *story,
    int resource_number)
{
  int i;

  for (i=0; i<story->nof_sounds; i++)
    if (story->blorb_sounds[i].resource_number == resource_number)
      return &story->blorb_sounds[i];

  return NULL;
}


int close_interface(z_ucs /*@null@*/ *error_message)
{
  int return_code;

  if (active_interface != NULL)
  {
    return_code = active_interface->close_interface(error_message);
    active_interface = NULL;
  }
  else
    return_code = 0;

  /*@-globstate@*/
  return return_code;
  /*@+globstate@*/
}


void *fizmo_malloc(size_t size)
{
  void *result;

  if ((result = malloc(size)) == NULL)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_MALLOC_P0D_RETURNED_NULL_PROBABLY_OUT_OF_MEMORY,
        -1,
        size);

  return result;
}


void *fizmo_realloc(void *ptr, size_t size)
{
  void *result;

  if ((result = realloc(ptr, size)) == NULL)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_REALLOC_P0D_RETURNED_NULL_PROBABLY_OUT_OF_MEMORY,
        -1,
        size);

  return result;
}


char *fizmo_strdup(char *s1)
{
  void *result;

  if ((result = strdup(s1)) == NULL)
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_FUNCTION_CALL_MALLOC_P0D_RETURNED_NULL_PROBABLY_OUT_OF_MEMORY,
        -1,
        strlen(s1));

  return result;
}


int ensure_mem_size(char **ptr, int *current_size, int size)
{
  if (size > *current_size)
  {
    if ((*ptr = fizmo_realloc(*ptr, size)) == NULL)
      return -1;
    *current_size = size;
  }
  return 0;
}


#ifndef DISABLE_CONFIGFILES
char *get_xdg_config_dir_name()
{
  char *config_dir_used;
  struct passwd *pw_entry;

  if (xdg_config_dir_name_initialized == true)
    return xdg_config_home;

  config_dir_used = getenv("XDG_CONFIG_HOME");
  if ( (config_dir_used == NULL) || (strlen(config_dir_used) == 0) )
    config_dir_used = default_xdg_config_home;

  if (*config_dir_used != '/')
  {
    pw_entry = getpwuid(getuid());
    if (pw_entry->pw_dir == NULL)
      return NULL;

    xdg_config_home = (char*)fizmo_malloc(
        strlen(pw_entry->pw_dir)
        + strlen(config_dir_used)
        + 2);
    sprintf(xdg_config_home, "%s/%s", pw_entry->pw_dir, config_dir_used);
  }
  else
  {
    xdg_config_home = fizmo_strdup(config_dir_used);
  }

  // REVISIT: free(xdg_config_home) on end.

  xdg_config_dir_name_initialized = true;

  return xdg_config_home;
}
#endif // DISABLE_CONFIGFILES


#ifndef DISABLE_CONFIGFILES
char *get_fizmo_config_dir_name()
{
  char *config_dir_used = NULL;

  if (bool_equal(fizmo_config_dir_name_initialized, false))
  {
    config_dir_used = get_xdg_config_dir_name();

    fizmo_config_dir_name = (char*)fizmo_malloc(
        strlen(config_dir_used)
        + strlen(FIZMO_CONFIG_DIR_NAME)
        + 2);
    sprintf(fizmo_config_dir_name, "%s/%s",
        config_dir_used, FIZMO_CONFIG_DIR_NAME);
  }

  fizmo_config_dir_name_initialized = true;

  return fizmo_config_dir_name;
}
#endif // DISABLE_CONFIGFILES


#ifndef DISABLE_CONFIGFILES
void ensure_dot_fizmo_dir_exists()
{
  char *dir_name = get_fizmo_config_dir_name();
  char *xdg_config_dir_name;
  DIR *dir;

  // XDG guide says: If, when attempting to write a file, the destination
  // directory is non-existant an attempt should be made to create it with
  // permission 0700.

  if (dir_name == NULL)
    return;

  if ((dir = opendir(dir_name)) != NULL)
  {
    closedir(dir);
    return;
  }

  xdg_config_dir_name = get_xdg_config_dir_name();

  if ((dir = opendir(xdg_config_dir_name)) == NULL)
  {
    if (mkdir(xdg_config_dir_name, S_IRWXU) == -1)
      return;
  }
  else
    closedir(dir);

  mkdir(dir_name, S_IRWXU);
}
#endif // DISABLE_CONFIGFILES


/*@external@*/ void fizmo_new_screen_size(uint8_t width, uint8_t height)
{
  if (ver >= 4)
  {
    TRACE_LOG("Writing %d to $20, %d to $21.\n", height, width);
    z_mem[0x20] = height;
    z_mem[0x21] = width;
  }

  if (ver >= 5)
  {
    // 8.4.3: In Version 5 and later, the screen's width and height in
    // units should be written to the words at $22 and $24.
    // of the header, respectively. In Version 6, these bytes are the
    // other way round (height in $27, width in $26). The width of a
    // font is defined as the width of its '0' character.
    TRACE_LOG("Writing %d to $22, %d to $24.\n", width, height);

    z_mem[0x22] = width >> 8;
    z_mem[0x23] = width & 0xff;

    z_mem[0x24] = height >> 8;
    z_mem[0x25] = height & 0xff;
  }
}


void write_interpreter_info_into_header()
{
  uint16_t width, height;

  if (active_interface == NULL)
    return;

  TRACE_LOG("Linking interface \"%s\" to active story.\n",
      active_interface->get_interface_name());

  // Write status line informiation to the header
  if (ver <= 3)
  {
    // In Versions 1 to 3, a status line should be printed by the
    // interpreter, as follows. In Version 3, it must set bit 4 of
    // 'Flags 1' in the header if it is unable to produce a status line.
    if (active_interface->is_status_line_available() == true)
    {
      TRACE_LOG("Status line is availiable.\n");
      z_mem[1] |= 0x10;
    }
    else
    {
      TRACE_LOG("Status line is not availiable.\n");
      z_mem[1] &= (0xff & ~0x10);
    }

    if (active_interface->is_split_screen_available() == true)
      z_mem[1] |= 0x20;

    if (active_interface->is_variable_pitch_font_default() == true)
      z_mem[1] |= 0x40;

    if (strcmp(get_configuration_value("set-tandy-flag"), "true") == 0)
      z_mem[1] |= 0x08;
  }

  if (ver >= 4)
  {
    z_mem[0x1e] = FIZMO_INTERPRETER_NUMBER;
    z_mem[0x1f] = FIZMO_INTERPRETER_REVISION

    if (active_interface->is_bold_face_available() == true)
    {
      TRACE_LOG("Boldface is available.\n");
      z_mem[1] |= 0x04;
    }
    else
    {
      TRACE_LOG("Boldface is not available.\n");
      z_mem[1] &= (0xff & ~0x04);
    }

    if (active_interface->is_italic_available() == true)
      z_mem[1] |= 0x08;

    if (active_interface->is_fixed_space_font_available() == true)
      z_mem[1] |= 0x10;

    // Write information about timed input
    if (active_interface->is_timed_keyboard_input_available() == true)
      z_mem[1] |= 0x80;

    width = active_interface->get_screen_width();
    height = active_interface->get_screen_height();

    fizmo_new_screen_size(width, height);

    if (ver >= 5)
    {
      // Write color information into the header
      if (active_interface->is_colour_available() == true)
        z_mem[1] |= 0x1;

      // picture support / character font support for beyond zork
      if (active_interface->is_character_graphics_font_availiable() == false)
        z_mem[0x11] &= 0xf7;

      current_foreground_colour
        = active_interface->get_default_foreground_colour();

      current_background_colour
        = active_interface->get_default_background_colour();

      upper_window_foreground_colour = current_foreground_colour;
      upper_window_background_colour = current_background_colour;
      lower_window_foreground_colour = current_foreground_colour;
      lower_window_background_colour = current_background_colour;

      z_mem[0x2c] = (uint8_t)current_background_colour;
      z_mem[0x2d] = (uint8_t)current_foreground_colour;

      z_mem[0x26] = 1;
      z_mem[0x27] = 1;

      if (ver >= 6)
      {
        if (active_interface->is_picture_displaying_available() == true)
          z_mem[1] |= 0x2;
      }
    }
  }

  z_mem[0x32] = OBEYS_SPEC_MAJOR_REVISION_NUMER;
  z_mem[0x33] = OBEYS_SPEC_MINOR_REVISION_NUMER;
}


void fizmo_register_screen_interface(struct z_screen_interface
    *screen_interface)
{
  if (active_interface == NULL)
  {
    active_interface = screen_interface;
    register_i18n_stream_output_function(streams_z_ucs_output);
    register_i18n_abort_function(abort_interpreter);
  }
}


void fizmo_register_sound_interface(
    struct z_sound_interface *sound_interface)
{
  TRACE_LOG("Registered sound interface at %p.\n", sound_interface);
  active_sound_interface = sound_interface;
}


#ifndef DISABLE_CONFIGFILES
static int parse_fizmo_config_file(char *filename)
{
  char key[MAX_CONFIG_OPTION_LENGTH];
  char value[MAX_CONFIG_OPTION_LENGTH];
  int c, i;
  z_file *config_file;

  TRACE_LOG("Parsing config file \"%s\".\n", filename);

  if ((config_file = fsi->openfile(filename, FILETYPE_DATA, FILEACCESS_READ))
      == NULL)
    return 0;

  while ((c = fsi->getchar(config_file)) != EOF)
  {
    i = 0;
    while ( (c != '=') && (c != '\n') && (c != EOF) )
    {
      if ( (i < MAX_CONFIG_OPTION_LENGTH - 1) && ( ! ((i == 0) && (c == ' '))) )
        key[i++] = (char)c;
      c = fsi->getchar(config_file);
    }

    if ( (c == '=') || (c == '\n') )
    {
      while ( (i > 0) && (key[i-1] == ' ') )
        i--;
      key[i] = '\0';
      i = 0;

      if (c == '=')
      {
        do
          c = fsi->getchar(config_file);
        while (c == ' ');
        while ( (c != '\n') && (c != EOF) && (i < MAX_CONFIG_OPTION_LENGTH-1) )
        {
          value[i++] = (char)c;
          c = fsi->getchar(config_file);
        }

        while ( (i > 0) && (value[i-1] == ' ') )
          i--;
      }

      if (c == '\n')
      {
        value[i] = '\0';

        if ( (key[0] != 0) && (key[0] != '#') )
        {
          TRACE_LOG("Incoming config key: \"%s\".\n", key);

          // Don't set and overwrite path, append instead.
          if (
              (strcmp(key, "z-code-path") == 0)
              ||
              (strcmp(key, "z-code-root-path") == 0)
             )
          {
            append_path_value(key, value);
          }
          else
          {
            set_configuration_value(key, value);
          }
        }
      }
    }
  }

  fsi->closefile(config_file);
  return 0;
}
#endif // DISABLE_CONFIGFILES


#ifndef DISABLE_CONFIGFILES
int parse_fizmo_config_files()
{
  char *config_dirs, *dir;
  char *string_to_split, *split_index;
  char *filename = NULL;
  int filename_size = 0;

  if (bool_equal(config_files_were_parsed, true))
    return 0;

  parse_fizmo_config_file(SYSTEM_FIZMO_CONF_FILE_NAME);

  config_dirs = getenv("XDG_CONFIG_DIRS");
  if ( (config_dirs == NULL) || (strlen(config_dirs) == 0) )
    config_dirs = default_xdg_config_dirs;
  string_to_split = fizmo_strdup(config_dirs);
  for(;;)
  {
    split_index = strrchr(string_to_split, ':');
    if (split_index == NULL)
      dir = string_to_split;
    else
    {
      *split_index = '\0';
      dir = split_index + 1;
    }
    ensure_mem_size(&filename, &filename_size,
        strlen(dir) + strlen(FIZMO_CONFIG_DIR_NAME)
        + strlen(FIZMO_CONFIG_FILE_NAME) + 3);
    sprintf(filename, "%s/%s/%s",
        dir, FIZMO_CONFIG_DIR_NAME, FIZMO_CONFIG_FILE_NAME);
    parse_fizmo_config_file(filename);

    if (split_index == NULL)
      break;
  }

  dir = get_fizmo_config_dir_name();

  if (dir != NULL)
  {
    ensure_mem_size(&filename, &filename_size,
        strlen(dir) + strlen(FIZMO_CONFIG_FILE_NAME) + 2);
    sprintf(filename, "%s/%s", dir, FIZMO_CONFIG_FILE_NAME);
    parse_fizmo_config_file(filename);
  }

  config_files_were_parsed = true;

  if (filename != NULL)
    free(filename);

  return 0;
}
#endif // DISABLE_CONFIGFILES


// This will quote all newlines, TABs and backslashes which must not appear
// in a config-file. Returns a new malloced string which should be freed after
// use.
char *quote_special_chars(char *s)
{
  char *ptr, *result;
  size_t len;

  if (s == NULL)
    return NULL;

  len = strlen(s) + 1;
  ptr = s;
  while (*ptr != '\0')
  {
    if (
        (*ptr == '\n')
        ||
        (*ptr == '\t')
        ||
        (*ptr == '\\')
       )
      len++;
    ptr++;
  }

  result = (char*)fizmo_malloc(len);
  ptr = result;

  while (*s != '\0')
  {
    if (*s == '\n')
    {
      *ptr = '\\';
      ptr++;
      *ptr = 'n';
    }
    else if (*s == '\t')
    {
      *ptr = '\\';
      ptr++;
      *ptr = 't';
    }
    else if (*s == '\\')
    {
      *ptr = '\\';
      ptr++;
      *ptr = '\\';
    }
    else
    {
      *ptr = *s;
    }
    s++;
    ptr++;
  }
  *ptr = '\0';

  return result;
}


char *unquote_special_chars(char *s)
{
  char *ptr, *result;
  size_t len;

  if (s == NULL)
    return NULL;

  len = strlen(s) + 1;
  ptr = s;
  while (*ptr != '\0')
  {
    if (
        (*ptr == '\n')
        ||
        (*ptr == '\t')
        ||
        (*ptr == '\\')
       )
      len--;
    ptr++;
  }

  result = (char*)fizmo_malloc(len);
  ptr = result;

  while (*s != '\0')
  {
    if (*s == '\\')
    {
      s++;
      if (*s == '\\')
        *ptr = '\\';
      else if (*s == 'n')
        *ptr = '\n';
      else if (*s == 't')
        *ptr = '\t';
      else
      {
        fprintf(stderr, "Invalid input: \"\\%c\".", *s);
      }
    }
    else
    {
      *ptr = *s;
    }
    ptr++;
    s++;
  }
  *ptr = '\0';

  return result;
}


void fizmo_start(char* input_filename, char *blorb_filename,
    char *restore_on_start_filename)
{
  z_file *in = NULL;
  char *story_file_to_load = NULL;
  char *value;
  bool evaluate_result;
  char *assumed_filename = NULL;
  uint8_t flags2;
  char *str;
#ifndef DISABLE_FILELIST
  struct z_story_list *story_list;
  int i;
#endif // DISABLE_FILELIST

  TRACE_LOG("Startup for input filename \"%s\".\n", input_filename);

  if (active_interface == NULL)
    return;

  register_i18n_stream_output_function(
      streams_z_ucs_output);

  register_i18n_abort_function(
      abort_interpreter);

#ifndef DISABLE_CONFIGFILES
  ensure_dot_fizmo_dir_exists();
#endif // DISABLE_CONFIGFILES

  if ((str = getenv("ZCODE_PATH")) == NULL)
    str = getenv("INFOCOM_PATH");
  if (str != NULL)
    append_path_value("z-code-path", str);

  if ((str = getenv("ZCODE_ROOT_PATH")) != NULL)
    append_path_value("z-code-root-path", str);

#ifndef DISABLE_CONFIGFILES
  parse_fizmo_config_files();
#endif // DISABLE_CONFIGFILES

  if (get_configuration_value("random-mode") == NULL)
    set_configuration_value("random-mode", "random");

  //set_configuration_value("disable-external-streams", "true");

  open_streams();
  init_signal_handlers();

#ifndef DISABLE_FILELIST
  // We can only restore saved games directly from the save file in
  // case the filelist functionality exists.
  if (
      (restore_on_start_filename == NULL)
      &&
      (detect_saved_game(input_filename, &story_file_to_load) == true)
     )
  {
    if (
        (story_file_to_load == NULL)
        ||
        ((in = fsi->openfile(story_file_to_load, FILETYPE_DATA,
                             FILEACCESS_READ)) == NULL)
       )
    {
      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_FATAL_ERROR_READING_STORY_FILE,
          -0x0101);
    }
    if (in != NULL)
      fsi->closefile(in);

    // File given on the command line is a quetzal save game.
    restore_on_start_filename = input_filename;
  }
  else
#endif // DISABLE_FILELIST
    story_file_to_load = input_filename;

  if ((in = fsi->openfile(
          story_file_to_load, FILETYPE_DATA, FILEACCESS_READ)) != NULL)
  {
    // File exists, load.
    fsi->closefile(in);
  }
  else
  {
#ifndef DISABLE_FILELIST
    // File does not exist. We'll ask the story list to locate a suitable
    // entry.
    story_list = get_z_story_list();
    for (i=0; i<story_list->nof_entries; i++)
    {
      if (strcasecmp(story_list->entries[i]->title, story_file_to_load) == 0)
      {
        assumed_filename = fizmo_strdup(story_list->entries[i]->filename);
        free_z_story_list(story_list);
        break;
      }
    }
#endif // DISABLE_FILELIST

    if (assumed_filename == NULL)
    {
#ifndef DISABLE_FILELIST
      free_z_story_list(story_list);
#endif // DISABLE_FILELIST

      i18n_translate_and_exit(
          libfizmo_module_name,
          i18n_libfizmo_COULD_NOT_OPEN_FILE_NAMED_P0S,
          -0x0101,
          story_file_to_load);
    }

    story_file_to_load = assumed_filename;
  }

  TRACE_LOG("Story file to load: \"%s\".\n", story_file_to_load);

  active_z_story = load_z_story(story_file_to_load, blorb_filename);

  if (
      (active_z_story->release_code == 2)
      &&
      (strcmp(active_z_story->serial_code, "AS000C") == 0)
     )
    skip_active_routines_stack_check_warning = true;

  z_mem = active_z_story->memory;
  TRACE_LOG("Z-Mem at %p.\n", z_mem);

  ver = active_z_story->version;

  init_opcode_functions();
  init_streams(DEFAULT_SAVEGAME_FILENAME);

  (void)latin1_string_to_zucs_string(
      last_savegame_filename,
      DEFAULT_SAVEGAME_FILENAME,
      MAXIMUM_SAVEGAME_NAME_LENGTH + 1);

  TRACE_LOG("Converted savegame default filename: '");
  TRACE_LOG_Z_UCS(last_savegame_filename);
  TRACE_LOG("'.\n");

  active_interface->link_interface_to_story(active_z_story);

  TRACE_LOG("sound-strcmp: %d.\n",
      strcmp(get_configuration_value("disable-sound"), "true"));

  if (
      (active_sound_interface != NULL)
      &&
      (strcmp(get_configuration_value("disable-sound"), "true") != 0)
     )
  {
    TRACE_LOG("Activating sound.\n");
    active_sound_interface->init_sound();
  }

  write_interpreter_info_into_header();

  // REVISIT: Implement general initalization for restore / restart etc.
  active_window_number = 0;
  //previous_z_font = Z_FONT_NORMAL;
  current_font = Z_FONT_NORMAL;

#ifndef DISABLE_OUTPUT_HISTORY
  outputhistory[0]
    = create_outputhistory(
        0,
        Z_HISTORY_MAXIMUM_SIZE,
        Z_HISTORY_INCREMENT_SIZE,
        //Z_HISTORY_V5_WIN0_METADATA_INCREMENT_SIZE,
        active_interface->get_default_foreground_colour(),
        active_interface->get_default_background_colour(),
        Z_FONT_NORMAL,
        Z_STYLE_ROMAN);
#endif /* DISABLE_OUTPUT_HISTORY */

  if ( (ver <= 8) && (ver != 6) )
  {
    while (terminate_interpreter == INTERPRETER_QUIT_NONE)
    {
      if (restore_on_start_filename != NULL)
      {
        value = get_configuration_value(
            "restore-after-save-and-quit-file-before-read");

        if ( (value != NULL) && (strcmp(value, "true") == 0) )
          evaluate_result = false;
        else
          evaluate_result = true;

        if (restore_game(
              0,
              (uint16_t)(active_z_story->dynamic_memory_end - z_mem + 1),
              restore_on_start_filename,
              true,
              evaluate_result,
              NULL) == 2)
        {
          TRACE_LOG("Redrawing screen from history.\n");
          active_interface->game_was_restored_and_history_modified();

          interpret_resume();
        }

        restore_on_start_filename = NULL;
      }
      else
        interpret_from_address(load_word(z_mem + 0x6));

      if (terminate_interpreter == INTERPRETER_QUIT_RESTART)
      {
        TRACE_LOG("Initiating restart.\n");

        // Restart the game. (Any "Are you sure?" question must be asked by the
        // game, not the interpreter.) The only pieces of information surviving
        // from the previous state are the "transcribing to printer" bit (bit 0
        // of 'Flags 2' in the header, at address $10) and the "use fixed pitch
        // font" bit (bit 1 of 'Flags 2').

        flags2 = z_mem[0x11] & 0x3;

        drop_z_stack_words(z_stack_index - z_stack);
        number_of_stack_frames = 0;
        stack_words_from_active_routine = 0;
        number_of_locals_active = 0;
        store_first_stack_frame(true, 0, 0, 0);

        if ((fsi->setfilepos(
                active_z_story->z_story_file,
                active_z_story->story_file_exec_offset,
                SEEK_SET))
            != 0)
          i18n_translate_and_exit(
              libfizmo_module_name,
              i18n_libfizmo_FUNCTION_CALL_P0S_RETURNED_ERROR_CODE_P1D,
              -0x0101,
              "setfilepos",
              errno);

        TRACE_LOG("Loading %ld bytes.\n",
            (long int)(active_z_story->high_memory_end + 1 - z_mem));

        if (fsi->getchars(
              z_mem,
              (size_t)(active_z_story->high_memory_end + 1 - z_mem),
              active_z_story->z_story_file)
            != (size_t)(active_z_story->high_memory_end + 1 - z_mem))
        {
          i18n_translate_and_exit(
              libfizmo_module_name,
              i18n_libfizmo_FATAL_ERROR_READING_STORY_FILE,
              -1);
        }

        active_interface->reset_interface();

        write_interpreter_info_into_header();

        z_mem[0x11] &= 0xfc;
        z_mem[0x11] |= flags2;

        terminate_interpreter = INTERPRETER_QUIT_NONE;
      }
    }
  }
  else
    i18n_translate_and_exit(
        libfizmo_module_name,
        i18n_libfizmo_NOT_YET_IMPLEMENTED,
        -0x010d);

  if (active_sound_interface != NULL)
    active_sound_interface->close_sound();

  // Close all streams, this will also close the active interface.
  close_streams(NULL);

  if (assumed_filename != NULL)
    free(assumed_filename);

  // TODO: Free memory.
}

#endif /* fizmo_c_INCLUDED */

