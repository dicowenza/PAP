
#include "compute.h"
#include "debug.h"
#include "global.h"
#include "graphics.h"
#include "ocl.h"
#include "scheduler.h"
#include "monitoring.h"

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

static unsigned tranche = 0;

static void traiter_tuile (int i_d, int j_d, int i_f, int j_f)
{
  PRINT_DEBUG ('c', "tuile [%d-%d][%d-%d] traitée\n", i_d, i_f, j_d, j_f);

  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      next_img (i, j) = cur_img (j, i);
}

unsigned transpose_compute_tiled (unsigned nb_iter)
{
  tranche = DIM / GRAIN;

  for (unsigned it = 1; it <= nb_iter; it++) {

    // On itére sur les coordonnées des tuiles
    for (int i = 0; i < GRAIN; i++)
      for (int j = 0; j < GRAIN; j++)
        traiter_tuile (i * tranche /* i debut */, j * tranche /* j debut */,
                           (i + 1) * tranche - 1 /* i fin */,
                           (j + 1) * tranche - 1 /* j fin */);

    swap_images ();
  }

  return 0;
}

///////////////////////////// Version OpenMP avec omp for (omp)

void transpose_ft_omp ()
{
  int i, j;

#pragma omp parallel for
  for (i = 0; i < DIM; i++) {
    for (j = 0; j < DIM; j += 512)
      next_img (i, j) = cur_img (i, j) = 0;
  }
}

unsigned transpose_compute_omp (unsigned nb_iter)
{
  tranche = DIM / GRAIN;
#pragma omp parallel
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp for
    for (int i = 0; i < DIM; i++) {
      for (int j = 0; j < DIM; j++)
        next_img (i, j) = cur_img (j, i);
    }
#pragma omp single
    swap_images ();
  }

  return 0;
}

// tuile omp

static void touche_tuile (int i_d, int j_d, int i_f, int j_f)
{

  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      next_img (i, j) = cur_img (j, i) = 1;
}

unsigned transpose_ft_omp_tiled (unsigned nb_iter)
{
  tranche = DIM / GRAIN;

#pragma omp parallel
#pragma omp for collapse(2)
  for (int i = 0; i < GRAIN; i++)
    for (int j = 0; j < GRAIN; j++)
      touche_tuile (i * tranche /* i debut */, j * tranche /* j debut */,
                    (i + 1) * tranche - 1 /* i fin */,
                    (j + 1) * tranche - 1 /* j fin */);

  return 0;
}

unsigned transpose_compute_omp_tiled (unsigned nb_iter)
{
  tranche = DIM / GRAIN;

#pragma omp parallel
  for (unsigned it = 1; it <= nb_iter; it++) {
    // On itére sur les coordonnées des tuiles
#pragma omp for collapse(2) schedule(static,1)
    for (int i = 0; i < GRAIN; i++)
      for (int j = 0; j < GRAIN; j++) {
        traiter_tuile (i * tranche /* i debut */, j * tranche /* j debut */,
                           (i + 1) * tranche - 1 /* i fin */,
                           (j + 1) * tranche - 1 /* j fin */);
#ifdef ENABLE_MONITORING
        monitoring_add_tile (j * tranche, i * tranche, tranche, tranche,
                             omp_get_thread_num ());
#endif
      }
#pragma omp single
    swap_images ();
  }

  return 0;
}

///////////////////////////// Version utilisant un ordonnanceur maison (sched)

unsigned P;

void transpose_init_sched ()
{
  P = scheduler_init (-1);
}

void transpose_finalize_sched ()
{
  scheduler_finalize ();
}

static inline void *pack (int i, int j)
{
  uint64_t x = (uint64_t)i << 32 | j;
  return (void *)x;
}

static inline void unpack (void *a, int *i, int *j)
{
  *i = (uint64_t)a >> 32;
  *j = (uint64_t)a & 0xFFFFFFFF;
}

static unsigned triangle (int i, int j)
{
  int x = i, y = j;
  if (i < j) {
    x = j;
    y = i;
  }
  return ((x) * (x + 1)) / 2 + y;
}

static inline unsigned cpu (int i, int j)
{
  return triangle (i, j) % P;
}

static inline void create_task (task_func_t t, int i, int j)
{
  scheduler_create_task (t, pack (i, j), cpu (i, j));
}

//////// First Touch

static void zero_seq (int i_d, int j_d, int i_f, int j_f)
{

  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      next_img (i, j) = cur_img (i, j) = 0;
}

static void first_touch_task (void *p, unsigned proc)
{
  int i, j;

  unpack (p, &i, &j);

  // PRINT_DEBUG ('s', "First-touch Task is running on tile (%d, %d) over cpu
  // #%d\n", i, j, proc);
  zero_seq (i * tranche, j * tranche, (i + 1) * tranche - 1,
            (j + 1) * tranche - 1);
}

void transpose_ft_sched (void)
{
  tranche = DIM / GRAIN;

  for (int i = 0; i < GRAIN; i++)
    for (int j = 0; j < GRAIN; j++)
      create_task (first_touch_task, i, j);

  scheduler_task_wait ();
}

//////// Compute

static void compute_task (void *p, unsigned proc)
{
  int i, j;

  unpack (p, &i, &j);

  PRINT_DEBUG ('s', "Compute Task is running on tile (%d, %d) over cpu #%d\n", i, j, proc);
  traiter_tuile (i * tranche, j * tranche, (i + 1) * tranche - 1,
                     (j + 1) * tranche - 1);
}

unsigned transpose_compute_sched (unsigned nb_iter)
{
  tranche = DIM / GRAIN;

  for (unsigned it = 1; it <= nb_iter; it++) {

    for (int i = 0; i < GRAIN; i++)
      for (int j = 0; j < GRAIN; j++)
        create_task (compute_task, i, j);

    scheduler_task_wait ();

    swap_images ();
  }

  return 0;
}
