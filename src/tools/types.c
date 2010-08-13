
#ifndef types_c_INCLUDED
#define types_c_INCLUDED

#include "types.h"

bool is_regular_z_colour(z_colour colour)
{
  if (
      (colour == Z_COLOUR_BLACK)
      ||
      (colour == Z_COLOUR_RED)
      ||
      (colour == Z_COLOUR_GREEN)
      ||
      (colour == Z_COLOUR_YELLOW)
      ||
      (colour == Z_COLOUR_BLUE)
      ||
      (colour == Z_COLOUR_MAGENTA)
      ||
      (colour == Z_COLOUR_CYAN)
      ||
      (colour == Z_COLOUR_WHITE)
      ||
      (colour == Z_COLOUR_MSDOS_DARKISH_GREY)
      ||
      (colour == Z_COLOUR_AMIGA_LIGHT_GREY)
      ||
      (colour == Z_COLOUR_MEDIUM_GREY)
      ||
      (colour == Z_COLOUR_DARK_GREY)
     )
    return true;
  else
    return false;
}

#endif /* types_c_INCLUDED */

