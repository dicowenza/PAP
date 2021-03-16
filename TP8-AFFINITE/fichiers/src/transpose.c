
#include "compute.h"
#include "debug.h"
#include "global.h"
#include "graphics.h"
#include "ocl.h"
#include "scheduler.h"

#include <stdbool.h>

///////////////////////////// Version séquentielle simple (seq)

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned transpose_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    for (int i = 0; i < DIM; i++)
      for (int j = 0; j < DIM; j++)
        next_img (i, j) = cur_img (j, i);

    swap_images ();
  }

  return 0;
}

///////////////////////////// Version séquentielle tuilée (tiled)

unsigned transpose_compute_tiled (unsigned nb_iter)
{
  // TODO

  return 0;
}
