// From https://github.com/NiVZ78/Edge_Ticks_4
// Edge_Ticks_4 version 1.0.0 by Paul Niven

#include <pebble.h>

#define MAJOR_TICK_LENGTH   PBL_IF_ROUND_ELSE(28, 23)
#define MAJOR_TICK_WIDTH    3
#define MINOR_TICK_LENGTH   PBL_IF_ROUND_ELSE(10, 6)
#define MINOR_TICK_WIDTH    1



// courtesy of @robisodd
int32_t abs32(int32_t a) {return (a^(a>>31)) - (a>>31);}     // returns absolute value of A (only works on 32bit signed)

// courtesy of @robisodd
GPoint getPointOnRect(GRect r, int angle) {
  int32_t sin = sin_lookup(angle), cos = cos_lookup(angle);  // Calculate once and store, to make quicker and cleaner
  int32_t dy = sin>0 ? (r.size.h/2) : (0-r.size.h)/2;        // Distance to top or bottom edge (from center)
  int32_t dx = cos>0 ? (r.size.w/2) : (0-r.size.w)/2;        // Distance to left or right edge (from center)
  if(abs32(dx*sin) < abs32(dy*cos)) {                        // if (distance to vertical line) < (distance to horizontal line)
    dy = (dx * sin) / cos;                                   // calculate distance to vertical line
  } else {                                                   // else: (distance to top or bottom edge) < (distance to left or right edge)
    dx = (dy * cos) / sin;                                   // move to top or bottom line
  }
  return GPoint(dx+r.origin.x+(r.size.w/2), dy+r.origin.y+(r.size.h/2));  // Return point on rectangle
}

