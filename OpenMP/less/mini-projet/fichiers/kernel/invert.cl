#include "kernel/common.cl"


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// scrollup
////////////////////////////////////////////////////////////////////////////////

__kernel void invert (__global unsigned *in, __global unsigned *out)
{
  int y = get_global_id (1);
  int x = get_global_id (0);
  unsigned couleur;

  couleur = in [y * DIM + x];
  couleur |= 0xFF000000;



  out [y * DIM + x] = couleur;
}
