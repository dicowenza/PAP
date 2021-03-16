#include "kernel/common.cl"


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// pavés
////////////////////////////////////////////////////////////////////////////////

#ifdef PARAM
#define PIX_BLOC PARAM
#else
#define PIX_BLOC 16
#endif

__kernel void tiles (__global unsigned *in, __global unsigned *out)
{
  __local unsigned couleur [TILEY / PIX_BLOC][TILEX / PIX_BLOC];
  int x = get_global_id (0);
  int y = get_global_id (1);
  int xloc = get_local_id (0);
  int yloc = get_local_id (1);

  if (xloc % PIX_BLOC == 0 && yloc % PIX_BLOC == 0)
    couleur [yloc / PIX_BLOC][xloc / PIX_BLOC] = in [y * DIM + x];

  barrier (CLK_LOCAL_MEM_FENCE);

  out [y * DIM + x] = couleur [yloc / PIX_BLOC][xloc / PIX_BLOC];
}
