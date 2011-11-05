
/* debugger.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2011 Christoph Ender.
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


#ifndef debugger_c_INCLUDED 
#define debugger_c_INCLUDED

#define DEBUGGER_INPUT_BUFFER_SIZE 1024

/*
#include <stdlib.h>
#include <string.h>
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "../tools/tracelog.h"
#include "../tools/list.h"
#include "zpu.h"
#include "streams.h"
#include "text.h"
#include "fizmo.h"
#include "debugger.h"
#include "string.h"
#include "variable.h"
/*
#include "../tools/i18n.h"
#include "debugger.h"
#include "config.h"
#include "routine.h"
#include "../locales/libfizmo_locales.h"
*/

// Pure PCs are saved until the story is available, meaning until the
// function "debugger_story_has_been_loaded" has been called. From then on,
// breakspoints are stored as pointers relative to z_mem in order to
// speed up searching.

list *pcs = NULL;
list *breakpoints = NULL;
bool story_has_been_loaded = false;
int sockfd;
struct sockaddr_in serv_addr;


static void debugger_output(int socked_fd, char *text)
{
  write(socked_fd, text, strlen(text));
}


void add_breakpoint(uint32_t breakpoint_pc)
{
  static uint32_t *new_element;

  if (story_has_been_loaded == false)
  {
    if (pcs == NULL)
      pcs = create_list();
    new_element = malloc(sizeof(uint32_t));
    *new_element = breakpoint_pc;
    add_list_element(pcs, new_element);
  }
  else
  {
    if (breakpoints == NULL)
      breakpoints = create_list();
    add_list_element(breakpoints, z_mem + breakpoint_pc);
  }
}


void debugger_story_has_been_loaded()
{
  size_t index, len;
  uint32_t *element;

  //add_breakpoint(0x200d0);
  story_has_been_loaded = true;

  if (pcs != NULL)
  {
    len = get_list_size(pcs);
    breakpoints = create_list();
    for (index=0; index<len; index++)
    {
      element = (uint32_t*)get_list_element(pcs, index);
      // TODO: Verify breakpoints.
      add_list_element(breakpoints, z_mem + *element);
      free(element);
    }
    delete_list(pcs);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    exit(-1);
  bzero((char *) &serv_addr, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(2048);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0) 
    exit(-1);
  listen(sockfd,5);
}


void do_breakpoint_actions()
{
  if (breakpoints == NULL)
    return;

  if (list_contains_element(breakpoints, pc) == false)
    return;

  //debugger_output("\nReached breakpoint.\n");
  debugger();
}


void debugger()
{
  int newsockfd = -1;
  char buffer[256];
  int n;
  struct sockaddr_in cli_addr;
  socklen_t clilen;
  int i;

  clilen = sizeof(cli_addr);
  while (newsockfd < 0) 
  {
    newsockfd = accept(sockfd, 
        (struct sockaddr *) &cli_addr, 
        &clilen);
    if (errno != EINTR) 
    {
      perror("accept");
      return;
    }
  }

  debugger_output(newsockfd, "\nEntering debugger.\n");

  for(;;)
  {
    sprintf(buffer, "\nPC: %lx\n", pc - z_mem);
    debugger_output(newsockfd, buffer);
    for (i=0; i<number_of_locals_active; i++)
    {
      if (i != 0)
        debugger_output(newsockfd, " ");
      sprintf(buffer, "L%02d:%x", i, local_variable_storage_index[i]);
      debugger_output(newsockfd, buffer);
    }
    debugger_output(newsockfd, "\n# ");

    n = read(newsockfd,buffer,255);
    if (n < 0)
      return;
    buffer[n-2]=0;

    debugger_output(newsockfd, "\n");
    if ( (strcmp(buffer, "exit") == 0) || (strcmp(buffer, "quit") == 0) )
    {
      break;
    }
    else
    {
      debugger_output(newsockfd, "Unknown command \"");
      debugger_output(newsockfd, buffer);
      debugger_output(newsockfd, "\".\n");
    }
  }
 
  debugger_output(newsockfd, "\nLeaving debugger.\n");

  close(newsockfd);
  return; 
}


void debugger_interpreter_stopped()
{
  close(sockfd);
}

#endif /* debugger_c_INCLUDED */

