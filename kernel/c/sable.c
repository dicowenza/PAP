#include "easypap.h"

#include <stdbool.h>

static long unsigned int *TABLE = NULL;
static long unsigned int *COPY = NULL;

#define table(i, j) TABLE[(i)*DIM + (j)]
#define copy(i, j) COPY[(i)*DIM + (j)]

static int *myTABLE = NULL;
static int *myCOPY = NULL;

#define mytable(i, j) myTABLE[(i)*GRAIN + (j)]
#define mycopy(i, j) myCOPY[(i)*GRAIN + (j)]



static volatile int changement;

static unsigned long int max_grains;


#define RGB(r, v, b) (((r) << 24 | (v) << 16 | (b) << 8) | 255)

void sable_init()
{
  TABLE = calloc(DIM * DIM, sizeof(long unsigned int));
  COPY = calloc(DIM * DIM, sizeof(long unsigned int));
  myTABLE = calloc(GRAIN * GRAIN, sizeof(int));
  myCOPY = calloc(GRAIN * GRAIN, sizeof(int));
  ////////////////////////////////////////////
  for (int i = 0; i < GRAIN; i++)
    for (int j = 0; j < GRAIN; j++)
      mytable(i, j) = 1;
}

void sable_finalize()
{
  free(TABLE);
  free(COPY);
  free(myCOPY);
  free(myTABLE);
}

///////////////////////////// Production d'une image
void sable_refresh_img()
{
  unsigned long int max = 0;
  for (int i = 1; i < DIM - 1; i++)
    for (int j = 1; j < DIM - 1; j++)
    {
      int g = table(i, j);
      int r, v, b;
      r = v = b = 0;
      if (g == 1)
        v = 255;
      else if (g == 2)
        b = 255;
      else if (g == 3)
        r = 255;
      else if (g == 4)
        r = v = b = 255;
      else if (g > 4)
        r = b = 255 - (240 * ((double)g) / (double)max_grains);

      cur_img(i, j) = RGB(r, v, b);
      if (g > max)
        max = g;
    }
  max_grains = max;
}

///////////////////////////// Configurations initiales

static void sable_draw_4partout(void);

void sable_draw(char *param)
{
  // Call function ${kernel}_draw_${param}, or default function (second
  // parameter) if symbol not found
  hooks_draw_helper(param, sable_draw_4partout);
}

void sable_draw_4partout(void)
{
  max_grains = 8;
  for (int i = 1; i < DIM - 1; i++)
    for (int j = 1; j < DIM - 1; j++)
      table(i, j) = 4;
}

void sable_draw_DIM(void)
{
  max_grains = DIM;
  for (int i = DIM / 4; i < DIM - 1; i += DIM / 4)
    for (int j = DIM / 4; j < DIM - 1; j += DIM / 4)
      table(i, j) = i * j / 4;
}

void sable_draw_alea(void)
{
  max_grains = 5000;
  for (int i = 0; i<DIM>> 3; i++)
  {
    table(1 + random() % (DIM - 2), 1 + random() % (DIM - 2)) =
        1000 + (random() % (4000));
  }
}

///////////////////////////// Version séquentielle simple (seq)

static inline void compute_new_state(int y, int x)
{
  if (table(y, x) >= 4)
  {
    unsigned long int div4 = table(y, x) / 4;
    table(y, x - 1) += div4;
    table(y, x + 1) += div4;
    table(y - 1, x) += div4;
    table(y + 1, x) += div4;
    table(y, x) %= 4;
    changement = 1;
  }
}

static void do_tile(int x, int y, int width, int height, int who)
{
  PRINT_DEBUG('c', "tuile [%d-%d][%d-%d] traitée\n", x, x + width - 1, y,
              y + height - 1);

  monitoring_start_tile(who);
#pragma omp parallel for schedule(runtime)
  for (int i = y; i < y + height; i++)
    for (int j = x; j < x + width; j++)
    {
      compute_new_state(i, j);
    }
  monitoring_end_tile(x, y, width, height, who);
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned sable_compute_seq(unsigned nb_iter)
{

  for (unsigned it = 1; it <= nb_iter; it++)
  {
    changement = 0;
    // On traite toute l'image en un coup (oui, c'est une grosse tuile)
    do_tile(1, 1, DIM - 2, DIM - 2, 0);
    if (changement == 0)
      return it;
  }
  return 0;
}

///////////////////////////// Version séquentielle tuilée (tiled)

unsigned sable_compute_tiled(unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++)
  {
    changement = 0;

    for (int y = 0; y < DIM; y += TILE_SIZE)
      for (int x = 0; x < DIM; x += TILE_SIZE)
        do_tile(x + (x == 0), y + (y == 0),
                TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                0 /* CPU id */);
    if (changement == 0)
      return it;
  }

  return 0;
}
///////////////////////////////// version seq base omp
//////////////////////////////// paralléllisation de la version sequentielle de base
unsigned sable_compute_seq_omp(unsigned nb_iter)
{

  for (unsigned it = 1; it <= nb_iter; it++)
  {
    changement = 0;
    // On traite toute l'image en un coup (oui, c'est une grosse tuile)
    do_tile(1, 1, DIM - 2, DIM - 2, 0);
    if (changement == 0)
      return it;
  }
  return 0;
}

//////////////////////////////////version tiled base omp
//////////////////////////////// paralléllisation de la version tuilée de base 
unsigned sable_compute_tiled_omp(unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++)
  {
    changement = 0;
#pragma omp parallel for collapse(2) schedule(runtime)
    for (int y = 0; y < DIM; y += TILE_SIZE)
      for (int x = 0; x < DIM; x += TILE_SIZE)
        do_tile(x + (x == 0), y + (y == 0),
                TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                 omp_get_thread_num());
    if (changement == 0)
      return it;
  }

  return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// ajout////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
static inline int my_compute_new_state(int y, int x)
{
  if (table(y, x) >= 4)
  {
    unsigned long int div4 = table(y, x) / 4;
    table(y, x - 1) += div4;
    table(y, x + 1) += div4;
    table(y - 1, x) += div4;
    table(y + 1, x) += div4;
    table(y, x) %= 4;
    return 1;
  }
  return 0;
}

static int my_do_tile(int x, int y, int width, int height, int who)
{
  int counter = 0;
  PRINT_DEBUG('c', "tuile [%d-%d][%d-%d] traitée\n", x, x + width - 1, y,
              y + height - 1);

  monitoring_start_tile(who);

#pragma omp parallel for collapse(2) schedule(runtime)
  for (int i = y; i < y + height; i++)
    for (int j = x; j < x + width; j++)
    {
      counter += my_compute_new_state(i, j);
    }
  monitoring_end_tile(x, y, width, height, who);
  return counter;
}

////////////////////////////////Ma version qui calculent a plusieurs endroits//////////

unsigned sable_compute_several_places(unsigned nb_iter)
{
  int change;
  for (unsigned it = 1; it <= nb_iter; it++)
  {
    change = 0;
#pragma omp parallel 
    {
#pragma omp for schedule(runtime)
      for (int y = 0; y < DIM; y += (TILE_SIZE)*2)
        for (int x = 0; x < DIM; x += (TILE_SIZE)*2)
        {
          int changement = my_do_tile(x + (x == 0), y + (y == 0),
                                      TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                                      TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                                      omp_get_thread_num());
          if (changement)
            change += changement;
        }
#pragma omp for schedule(runtime)
      for (int y = TILE_SIZE; y < DIM; y += (TILE_SIZE)*2)
        for (int x = 0; x < DIM; x += (TILE_SIZE)*2)
        {

          int changement = my_do_tile(x + (x == 0), y + (y == 0),
                                      TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                                      TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                                      omp_get_thread_num());
          if (changement)
            change += changement;
        }
#pragma omp for schedule(runtime)
      for (int y = TILE_SIZE; y < DIM; y += (TILE_SIZE)*2)
        for (int x = TILE_SIZE; x < DIM; x += (TILE_SIZE)*2)
        {

          int changement = my_do_tile(x + (x == 0), y + (y == 0),
                                      TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                                      TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                                      omp_get_thread_num());
          if (changement)
            change += changement;
        }
#pragma omp for schedule(runtime)
      for (int y = 0; y < DIM; y += (TILE_SIZE)*2)
        for (int x = TILE_SIZE; x < DIM; x += (TILE_SIZE)*2)
        {

          int changement = my_do_tile(x + (x == 0), y + (y == 0),
                                      TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                                      TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                                      omp_get_thread_num());
          if (changement)
            change += changement;
        }
    }
    if (change == 0)
      return it;
  }
  return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////




/////////////////Version synchrone ////////////////
void echanger_table()
{
  long unsigned int *tmp = TABLE;
  TABLE = COPY;
  COPY = tmp;
}
static inline int compute_new_state_bis(int y, int x)
{
  long unsigned int center = table(y, x);
  long unsigned int east = table(y + 1, x);
  long unsigned int west = table(y - 1, x);
  long unsigned int north = table(y, x - 1);
  long unsigned int south = table(y, x + 1);
  copy(y, x) = center % 4 + east/4+west/4+north/4+south/ 4;
  if (copy(y, x) != center)
    return 1;
  return 0;
}
static int do_tile_bis(int x, int y, int width, int height, int who)
{
  int counter = 0;
  monitoring_start_tile(who);
  for (int i = y; i < y + height; i++)
    for (int j = x; j < x + width; j++)
    {
      counter += compute_new_state_bis(i, j);
    }
  monitoring_end_tile(x, y, width, height, who);
  return counter;
}
static int do_tile_bis_optimized(int x, int y, int width, int height, int who)
{
  int counter = 0;
  monitoring_start_tile(who);
#pragma omp for schedule(runtime)
  for (int i = y; i < y + height; i++)
    for (int j = x; j < x + width; j++)
    {
      counter += compute_new_state_bis(i, j);
    }
  monitoring_end_tile(x, y, width, height, who);
  return counter;
}
unsigned sable_compute_myseq(unsigned nb_iter)
{
  int change;
  for (unsigned it = 1; it <= nb_iter; it++)
  {
    change = 0;
    change = do_tile_bis_optimized(1, 1, DIM - 2, DIM - 2, omp_get_thread_num());
    echanger_table();
    if (change == 0)
      return it;
  }
  return 0;
}
unsigned sable_compute_mytiled(unsigned nb_iter)
{
  int change;
  for (unsigned it = 1; it <= nb_iter; it++)
  {
    change = 0;
#pragma omp parallel for schedule(runtime)
    for (int y = 0; y < DIM; y += TILE_SIZE)
    {
      for (int x = 0; x < DIM; x += TILE_SIZE)
      {
        int changement = do_tile_bis(x + (x == 0), y + (y == 0),
                                TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                                TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                                omp_get_thread_num());
        if (changement)
          change += changement;
      }
    }
    echanger_table();
    if (change == 0)
      return it;
  }
  return 0;
}
///////////Version booleenne synchrone //////////////
void swap_bool()
{
  long unsigned int *tmp = TABLE;
  TABLE = COPY;
  COPY = tmp;
  int *boolean = myTABLE;
  myTABLE = myCOPY;
  myCOPY = boolean;
}
static int nb_neighbors(int x, int y)
{
  int center = mytable(y, x);
  int west = mytable(y + 1, x);
  int east = mytable(y - 1, x);
  int north = mytable(y, x - 1);
  int south = mytable(y, x + 1);
  int all = center + north + east+ west + south;
  return all;
}
static int has_neighbor(int x, int y)
{
  int east = 0;
  int west = 0;
  int north = 0;
  int south = 0;
  int center = mytable(y, x);
  if (y != GRAIN - 1)
    west = mytable(y + 1, x);
  if (y != 0)
    east = mytable(y - 1, x);
  if (x != 0)
    north = mytable(y, x - 1);
  if (x != GRAIN - 1)
    south = mytable(y, x + 1);
  int result =  east + west + south + north + center;
  return result;
}
static int compute_new_state_bool(int y, int x)
{
  long unsigned int center = table(y, x);
  long unsigned int east = table(y + 1, x);
  long unsigned int west = table(y - 1, x);
  long unsigned int north = table(y, x - 1);
  long unsigned int south = table(y, x + 1);
  copy(y, x) = center % 4 +east/4+west/4+north/4+south/4;
  
  if (copy(y, x) != center)
    return 1;
  
  return 0;
}
static int do_tile_bool(int x, int y, int width, int height, int who)
{
  PRINT_DEBUG('c', "tuile [%d-%d][%d-%d] traitée\n", x, x + width - 1, y, y + height - 1);
  monitoring_start_tile(who);
  int counter = 0;
  for (int i = y; i < y + height; i++)
    for (int j = x; j < x + width; j++)
      counter += compute_new_state_bool(i, j);
  monitoring_end_tile(x, y, width, height, who);
  return counter;
}
unsigned sable_compute_bool_tiled(unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++)
  {
    int change = 0;
#pragma omp parallel for schedule(runtime)
    for (int y = 0; y < DIM; y += TILE_SIZE)
    {
      for (int x = 0; x < DIM; x += TILE_SIZE)
      {
        int modification = 0;
        int X = x / TILE_SIZE;
        int Y = y / TILE_SIZE;
        if (X == 0 || Y == 0 || X == GRAIN - 1 || Y == GRAIN - 1)
          modification = has_neighbor(X, Y);
        else
          modification = nb_neighbors(X, Y);
        if (modification != 0)
        {
          int changement = do_tile_bool(x + (x == 0), y + (y == 0),
                                       TILE_SIZE - ((x + TILE_SIZE == DIM) + (x == 0)),
                                       TILE_SIZE - ((y + TILE_SIZE == DIM) + (y == 0)),
                                       omp_get_thread_num());
          mycopy(Y, X) = changement;
          change += changement;
        }
        else
        {
          mycopy(Y, X) = 0;
        }
      }
    }
    swap_bool();
    if (change == 0)
      return it;
  }
  return 0;
}
