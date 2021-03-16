#include "kernel/common.cl"

#define TILEX 16
#define TILEY 16
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// transpose
////////////////////////////////////////////////////////////////////////////////

__kernel void transpose (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);

  __local float tile[TILEX][TILEY];
  
  int xloc = get_global_id (0);
  int yloc = get_global_id (1);

  int X=get_group_id(0)*TILEX;/////x=x-xloc
  int Y=get_group_id(0)*TILEY;////y=y-yloc

  tile[xloc][yloc]=in[y*DIM+x];
  barrier(CLK_LOCAL_MEM_FENCE);
  out [X* DIM + Y+xloc+yloc*DIM] = tile [yloc][xloc];
}

