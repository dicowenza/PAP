
#include "compute.h"
#include "debug.h"
#include "global.h"
#include "graphics.h"
#include "ocl.h"
#include "scheduler.h"
#include "pthread_barrier.h"

#include <stdbool.h>
#include <pthread.h>

///////////////////////////// Version séquentielle simple (seq)

unsigned scrollup_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    for (int j = 0; j < DIM; j++)
      next_img (DIM - 1, j) = cur_img (0, j);

    for (int i = 0; i < DIM - 1; i++)
      for (int j = 0; j < DIM; j++)
        next_img (i, j) = cur_img (i + 1, j);

    swap_images ();
  }

  return 0;
}


///////////////////////////// Version OpenMP (omp)

unsigned scrollup_compute_omp (unsigned nb_iter)
{
  // TODO
  
  return 0;
}



