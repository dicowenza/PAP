
#include "compute.h"
#include "debug.h"
#include "global.h"
#include "graphics.h"
#include "ocl.h"
#include "pthread_barrier.h"
#include "scheduler.h"

#include <pthread.h>
#include <stdbool.h>

#ifdef ENABLE_VECTO
#include <immintrin.h>
#endif

static inline unsigned invert_pixel (unsigned c)
{
  return ((unsigned)0xFFFFFFFF - c) | 0xFF;
}


///////////////////////////// Version séquentielle simple (seq)

unsigned invert_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    for (int i = 0; i < DIM; i++)
      for (int j = 0; j < DIM; j++)
        cur_img (i, j) = invert_pixel (cur_img (i, j));
  }

  return 0;
}


///////////////////////////// Version OpenMP

unsigned invert_compute_omp (unsigned nb_iter)
{
  // TODO
  
  return 0;
}
