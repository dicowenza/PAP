
#include "compute.h"
#include "debug.h"
#include "global.h"
#include "graphics.h"
#include "monitoring.h"
#include "ocl.h"
#include "pthread_barrier.h"
#include "pthread_distrib.h"
#include "scheduler.h"

#include <stdbool.h>
#include <omp.h>

#ifdef ENABLE_VECTO
#include <immintrin.h>
#endif

#define MAX_ITERATIONS 4096
#define ZOOM_SPEED -0.01

static unsigned iteration_to_color (unsigned iter)
{
  unsigned r = 0, g = 0, b = 0;

  if (iter < MAX_ITERATIONS) {
    if (iter < 64) {
      r = iter * 2; /* 0x0000 to 0x007E */
    } else if (iter < 128) {
      r = (((iter - 64) * 128) / 126) + 128; /* 0x0080 to 0x00C0 */
    } else if (iter < 256) {
      r = (((iter - 128) * 62) / 127) + 193; /* 0x00C1 to 0x00FF */
    } else if (iter < 512) {
      r = 255;
      g = (((iter - 256) * 62) / 255) + 1; /* 0x01FF to 0x3FFF */
    } else if (iter < 1024) {
      r = 255;
      g = (((iter - 512) * 63) / 511) + 64; /* 0x40FF to 0x7FFF */
    } else if (iter < 2048) {
      r = 255;
      g = (((iter - 1024) * 63) / 1023) + 128; /* 0x80FF to 0xBFFF */
    } else {
      r = 255;
      g = (((iter - 2048) * 63) / 2047) + 192; /* 0xC0FF to 0xFFFF */
    }
  }
  return (r << 24) | (g << 16) | (b << 8) | 255 /* alpha */;
}

// Cadre initial
#if 0
// Config 1
static float leftX = -0.744;
static float rightX = -0.7439;
static float topY = .146;
static float bottomY = .1459;
#endif

#if 1
// Config 2
static float leftX   = -0.2395;
static float rightX  = -0.2275;
static float topY    = .660;
static float bottomY = .648;
#endif

#if 0
// Config 3
static float leftX = -0.13749;
static float rightX = -0.13715;
static float topY = .64975;
static float bottomY = .64941;
#endif

static float xstep;
static float ystep;

static void zoom (void)
{
  float xrange = (rightX - leftX);
  float yrange = (topY - bottomY);

  leftX += ZOOM_SPEED * xrange;
  rightX -= ZOOM_SPEED * xrange;
  topY -= ZOOM_SPEED * yrange;
  bottomY += ZOOM_SPEED * yrange;

  xstep = (rightX - leftX) / DIM;
  ystep = (topY - bottomY) / DIM;
}

void mandel_init ()
{
  xstep = (rightX - leftX) / DIM;
  ystep = (topY - bottomY) / DIM;
}

static unsigned compute_one_pixel (int i, int j)
{
  float xc = leftX + xstep * j;
  float yc = topY - ystep * i;
  float x = 0.0, y = 0.0; /* Z = X+I*Y */

  int iter;

  // Pour chaque pixel, on calcule les termes d'une suite, et on
  // s'arrête lorsque |Z| > 2 ou lorsqu'on atteint MAX_ITERATIONS
  for (iter = 0; iter < MAX_ITERATIONS; iter++) {
    float x2 = x * x;
    float y2 = y * y;

    /* Stop iterations when |Z| > 2 */
    if (x2 + y2 > 4.0)
      break;

    float twoxy = (float)2.0 * x * y;
    /* Z = Z^2 + C */
    x = x2 - y2 + xc;
    y = twoxy + yc;
  }

  return iter;
}


///////////////////////////// Version séquentielle simple (seq)

static void traiter_tuile (int i_d, int j_d, int i_f, int j_f)
{
  PRINT_DEBUG ('c', "tuile [%d-%d][%d-%d] traitée\n", i_d, i_f, j_d, j_f);

  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      cur_img (i, j) = iteration_to_color (compute_one_pixel (i, j));
}

unsigned mandel_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    // On traite toute l'image en une seule fois
    traiter_tuile (0, 0, DIM - 1, DIM - 1);
    
    zoom ();
  }

  return 0;
}


///////////////////////////// Version séquentielle tuilée (tiled)

static unsigned tranche = 0;

unsigned mandel_compute_tiled (unsigned nb_iter)
{
  tranche = DIM / GRAIN;

  for (unsigned it = 1; it <= nb_iter; it++) {

    // On itére sur les coordonnées des tuiles
    for (int i = 0; i < GRAIN; i++)
      for (int j = 0; j < GRAIN; j++)
        traiter_tuile (i * tranche /* i debut */, j * tranche /* j debut */,
		       (i + 1) * tranche - 1 /* i fin */,
		       (j + 1) * tranche - 1 /* j fin */);

    zoom ();
  }

  return 0;
}


/////////////////////////////////////////////// Ajout de Code///////////////////////////////////////////////////////////////////////////////////////////



unsigned mandel_compute_ompb (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    // On traite toute l'image en une seule fois
    //traiter_tuile (0, 0, DIM - 1, DIM - 1);

  #pragma omp parallel for schedule(static)
    for (int i = 0; i <= DIM-1; i++)
    for (int j = 0; j <= DIM-1; j++)
      cur_img (i, j) = iteration_to_color (compute_one_pixel (i, j));
    
    zoom ();
  }

  return 0;
}

// 3 est le meilleur


unsigned mandel_compute_ompc_k (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {
    // On traite toute l'image en une seule fois
    #pragma omp parallel for schedule(static,3)
    for (int i = 0; i <= DIM-1; i++)
    for (int j = 0; j <= DIM-1; j++)
      cur_img (i, j) = iteration_to_color (compute_one_pixel (i, j));
    zoom ();
  }
  return 0;
}

// 1 est le meilleur

unsigned mandel_compute_ompd_k (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {
    #pragma omp parallel for schedule(dynamic,1)
    for (int i = 0; i <= DIM-1; i++)
    for (int j = 0; j <= DIM-1; j++)
      cur_img (i, j) = iteration_to_color (compute_one_pixel (i, j));
    zoom ();
  }
  return 0;
}

// 2 est le meilleur paramètres //

unsigned mandel_compute_omptiled(unsigned nb_iter)
{
  tranche = DIM / GRAIN;
  for (unsigned it = 1; it <= nb_iter; it++) {
    // On itére sur les coordonnées des tuiles
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < GRAIN; i++)
      for (int j = 0; j < GRAIN; j++)
        traiter_tuile (i * tranche /* i debut */, j * tranche /* j debut */,
		       (i + 1) * tranche - 1 /* i fin */,
		       (j + 1) * tranche - 1 /* j fin */);
    zoom ();
  }
  return 0;
}

