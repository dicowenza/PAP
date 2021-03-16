
#include "compute.h"
#include "constants.h"
#include "debug.h"
#include "error.h"
#include "global.h"
#include "graphics.h"
#include "ocl.h"
#include "scheduler.h"

#include <stdbool.h>

///////////////////////////// Version séquentielle simple (seq)

int descendre_max_seq (int i_d, int j_d, int i_f, int j_f)
{
  int changement = 0;

  if (i_d == 0)
    i_d++;
  if (j_d == 0)
    j_d++;
  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      if (cur_img (i, j)) {
        Uint32 m = MAX (cur_img (i - 1, j), cur_img (i, j - 1));
        if (m > cur_img (i, j)) {
          changement     = 1;
          cur_img (i, j) = m;
        }
      }

  return changement;
}

int monter_max_seq (int i_d, int j_d, int i_f, int j_f)
{
  int changement = 0;

  if (i_f == DIM - 1)
    i_f--;
  if (j_f == DIM - 1)
    j_f--;
  for (int i = i_f; i >= i_d; i--)
    for (int j = j_f; j >= j_d; j--)
      if (cur_img (i, j)) {
        Uint32 m = MAX (cur_img (i + 1, j), cur_img (i, j + 1));
        if (m > cur_img (i, j)) {
          changement     = 1;
          cur_img (i, j) = m;
        }
      }

  return changement;
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned max_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    if ((descendre_max_seq (0, 0, DIM - 1, DIM - 1) |
         monter_max_seq (0, 0, DIM - 1, DIM - 1)) == 0)
      return it;
  }

  return 0;
}


///////////////////////////// Configuration initiale





/******************************************************************************************************/

int descendre_max_omp (int i_d, int j_d, int i_f, int j_f)
{
  int changement = 0;

  if (i_d == 0)
    i_d++;
  if (j_d == 0)
    j_d++;
//#pragma omp task firstprivate(i,j) depend(out:A[i][j])  depend(in: A[i-1][j],A[i][j-1])
  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      if (cur_img (i, j)) {
        Uint32 m = MAX (cur_img (i - 1, j), cur_img (i, j - 1));
        if (m > cur_img (i, j)) {
          changement     = 1;
          cur_img (i, j) = m;
        }
      }

  return changement;
}

int monter_max_omp (int i_d, int j_d, int i_f, int j_f)
{
  int changement = 0;

  if (i_f == DIM - 1)
    i_f--;
  if (j_f == DIM - 1)
    j_f--;
//#pragma omp task firstprivate(i,j) depend(in:A[i][j])  depend(out: A[i-1][j],A[i][j-1])
  for (int i = i_f; i >= i_d; i--)
    for (int j = j_f; j >= j_d; j--)
      if (cur_img (i, j)) {
        Uint32 m = MAX (cur_img (i + 1, j), cur_img (i, j + 1));
        if (m > cur_img (i, j)) {
          changement     = 1;
          cur_img (i, j) = m;
        }
      }

  return changement;
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned max_compute_omp (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    if ((descendre_max_seq (0, 0, DIM - 1, DIM - 1) |
         monter_max_seq (0, 0, DIM - 1, DIM - 1)) == 0)
      return it;
  }

  return 0;
}

/*****************************************************************************************************/
void spiral (unsigned twists);
void recolor (void);

void max_draw (char *param)
{
  unsigned n;

  if (param != NULL) {
    n = atoi (param);
    if (n > 0)
      spiral (n);
  }

  recolor ();
}

void recolor (void)
{
  unsigned nbits = 0;
  unsigned rb, bb, gb;
  unsigned r_lsb, g_lsb, b_lsb;
  unsigned r_shift, g_shift, b_shift;
  Uint8 r_mask, g_mask, b_mask;
  Uint8 red = 0, blue = 0, green = 0;

  // Calcul du nombre de bits nécessaires pour mémoriser une valeur
  // différente pour chaque pixel de l'image
  for (int i = DIM - 1; i; i >>= 1)
    nbits++; // log2(DIM-1)
  nbits = nbits * 2;

  if (nbits > 24)
    exit_with_error ("DIM of %d is too large (suggested max: 4096)\n", DIM);

  gb = nbits / 3;
  bb = gb;
  rb = nbits - 2 * bb;

  r_shift = 8 - rb;
  r_lsb   = (1 << r_shift) - 1;
  g_shift = 8 - gb;
  g_lsb   = (1 << g_shift) - 1;
  b_shift = 8 - bb;
  b_lsb   = (1 << b_shift) - 1;

  r_mask = (1 << rb) - 1;
  g_mask = (1 << gb) - 1;
  b_mask = (1 << bb) - 1;

  PRINT_DEBUG ('g', "nbits : %d (r: %d, g: %d, b: %d)\n", nbits, rb, gb, bb);
  PRINT_DEBUG ('g', "r_shift: %d, r_lst: %d, r_mask: %d\n", r_shift, r_lsb,
               r_mask);
  PRINT_DEBUG ('g', "g_shift: %d, g_lst: %d, g_mask: %d\n", g_shift, g_lsb,
               g_mask);
  PRINT_DEBUG ('g', "b_shift: %d, b_lst: %d, b_mask: %d\n", b_shift, b_lsb,
               b_mask);

  for (unsigned y = 0; y < DIM; y++) {
    for (unsigned x = 0; x < DIM; x++) {
      unsigned alpha = cur_img (y, x) & 255;

      if (alpha == 0 || x == 0 || x == DIM - 1 || y == 0 || y == DIM - 1)
        cur_img (y, x) = 0;
      else {
        unsigned c = 255; // alpha
        c |= ((blue << b_shift) | b_lsb) << 8;
        c |= ((green << g_shift) | g_lsb) << 16;
        c |= ((red << r_shift) | r_lsb) << 24;
        cur_img (y, x) = c;
      }

      red = (red + 1) & r_mask;
      if (red == 0) {
        green = (green + 1) & g_mask;
        if (green == 0)
          blue = (blue + 1) & b_mask;
      }
    }
  }
}

static unsigned couleur = 0xFFFF00FF; // Yellow

static void one_spiral (int x, int y, int pas, int nbtours)
{
  int i = x, j = y, tour;

  for (tour = 1; tour <= nbtours; tour++) {
    for (; i < x + tour * pas; i++)
      cur_img (i, j) = couleur;
    for (; j < y + tour * pas + 1; j++)
      cur_img (i, j) = couleur;
    for (; i > x - tour * pas - 1; i--)
      cur_img (i, j) = couleur;
    for (; j > y - tour * pas - 1; j--)
      cur_img (i, j) = couleur;
  }
}

static void many_spirals (int xdebut, int xfin, int ydebut, int yfin, int pas,
                          int nbtours)
{
  int i, j;
  int taille = nbtours * pas + 2;

  for (i = xdebut + taille; i < xfin - taille; i += 2 * taille)
    for (j = ydebut + taille; j < yfin - taille; j += 2 * taille)
      one_spiral (i, j, pas, nbtours);
}

void spiral (unsigned twists)
{
  many_spirals (1, DIM - 2, 1, DIM - 2, 2, twists);
}
