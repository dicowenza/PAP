
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

unsigned version = 0;

void first_touch_v1(void);
void first_touch_v2(void);

unsigned compute_v0(unsigned nb_iter);
unsigned compute_v1(unsigned nb_iter);
unsigned compute_v2(unsigned nb_iter);
unsigned compute_v3(unsigned nb_iter);
unsigned compute_v0_openmpfor(unsigned nb_iter);
unsigned compute_v1_openmpfor(unsigned nb_iter);
unsigned compute_v2_openmpfor(unsigned nb_iter);
unsigned compute_v1_openmptask(unsigned nb_iter);
unsigned compute_v2_openmptask(unsigned nb_iter);

void_func_t first_touch [] = {
  NULL,
  NULL,//first_touch_v1,
  NULL,//first_touch_v2,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v0_openmpfor,
  compute_v1_openmpfor,
  compute_v2_openmpfor,
  compute_v1_openmptask,
  compute_v2_openmptask,
  compute_v3,
};

char *version_name [] = {
  "Séquentielle de base",
  "Séquentielle tuilée",
  "Séquentielle tuilée (optimisée)",
  "OpenMP for de base",
  "OpenMP for tuilée",
  "OpenMP for tuilée (optimisée)",
  "OpenMP task tuilée",
  "OpenMP task tuilée (optimisée)",
  "OpenCL de base"
};//  "OpenMP",
//  "OpenMP zone",
//  "OpenCL",


unsigned opencl_used [] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1
};

///////////////////////////// Version séquentielle simple


unsigned const colorViv = 0xFFFF00FF;
unsigned const colorMort = 0x0;

int countAlive(int x, int y);
void majImg(int i, int j, int nbVoisinsVivants);
static void majTab();
void initTabsTiles();
void freeTabsTiles();
void task_v1(int i);
void task_v2(int i);
static void majTabOmpFor();
static void majTabOmpTask();
void task_omp_task(int x, int y);

unsigned compute_v0 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++){
    for(unsigned i = 0; i < DIM; i++){
      for(unsigned j = 0; j < DIM; j++){
	majImg(i, j, countAlive(i, j));
      }
    }
    swap_images();
  }
  return 0;
}


///////////////////////////// Version OpenMP de base

void first_touch_v1 ()
{
  int i,j ;

#pragma omp parallel for
  for(i=0; i<DIM ; i++) {
    for(j=0; j < DIM ; j += 512)
      next_img (i, j) = cur_img (i, j) = 0 ;
  }
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned sizeTiles = 32;
#define NBTILES  DIM / sizeTiles

unsigned compute_v1(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	for(int x = i; x < i + sizeTiles; x++){
	  for(int y = j; y < j + sizeTiles; y++){
	    majImg(x, y, countAlive(x, y));
	  }
	}
      }
    }
    swap_images();
  }
  return 0;
}

unsigned compute_v1_openmpfor(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp parallel for collapse(2)
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	for(int x = i; x < i + sizeTiles; x++){
	  for(int y = j; y < j + sizeTiles; y++){
	    majImg(x, y, countAlive(x, y));
	  }
	}
      }
    }
    swap_images();
  }
  return 0;
}

unsigned compute_v1_openmptask(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++){
#pragma omp parallel
#pragma omp single
    for(int i = 0; i < DIM; i+=sizeTiles){
#pragma omp task
      task_v1(i);
    }
    swap_images();
  }
  return 0;
}

void task_v1(int i){
  for(int j = 0; j < DIM; j+=sizeTiles){
    for(int x = i; x < i + sizeTiles; x++){
      for(int y = j; y < j + sizeTiles; y++){
	majImg(x, y, countAlive(x, y));
      }
    }
  }
}


///////////////////////////// Version OpenMP optimisée

void first_touch_v2 ()
{

}


int** tabTilesToMove;
int** tabTilesHaveMoved;
// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v2(unsigned nb_iter){
  static bool toto = true;
  if(toto){
    initTabsTiles();
    toto=false;
  }
  for (unsigned it = 1; it <= nb_iter; it++) {
    
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	if(tabTilesToMove[i/sizeTiles][j/sizeTiles]){
	  tabTilesHaveMoved[i/sizeTiles][j/sizeTiles] = 0;
	  for(int x = i; x < i + sizeTiles; x++){
	    for(int y = j; y < j + sizeTiles; y++){
	      majImg(x, y, countAlive(x, y));
	      if(cur_img(x, y) != next_img(x, y)){
		tabTilesHaveMoved[i/sizeTiles][j/sizeTiles] = 1;
	      }
	    }
	  }
			
	}
      }
    }
    majTab();
    swap_images();
  }
  
  //freeTabsTiles();
  return 0; // on ne s'arrête jamais
}

unsigned compute_v2_openmpfor(unsigned nb_iter){
  static bool toto = true;
  if(toto){
    initTabsTiles();
    toto=false;
  }
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp parallel for     
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	if(tabTilesToMove[i/sizeTiles][j/sizeTiles]){
	  tabTilesHaveMoved[i/sizeTiles][j/sizeTiles] = 0;
	  for(int x = i; x < i + sizeTiles; x++){
	    for(int y = j; y < j + sizeTiles; y++){
	      majImg(x, y, countAlive(x, y));
	      if(cur_img(x, y) != next_img(x, y)){
		tabTilesHaveMoved[i/sizeTiles][j/sizeTiles] = 1;
	      }
	    }
	  }
			
	}
      }
    }
    majTab();
    swap_images();
  }
  
  //freeTabsTiles();
  return 0; // on ne s'arrête jamais
}

unsigned compute_v2_openmptask(unsigned nb_iter){
  static bool toto = true;
  if(toto){
    initTabsTiles();
    toto=false;
  }
  for (unsigned it = 1; it <= nb_iter; it++){    
#pragma omp parallel
#pragma omp single
    for(int i = 0; i < DIM; i+=sizeTiles){
#pragma omp task
      task_v2(i);
    }
    //majTabOmpTask();
    majTab();
    swap_images();
  }
  return 0; // on ne s'arrête jamais
}

void task_v2(int i){
  for(int j = 0; j < DIM; j+=sizeTiles){
    if(tabTilesToMove[i/sizeTiles][j/sizeTiles]){
      tabTilesHaveMoved[i/sizeTiles][j/sizeTiles] = 0;
      for(int x = i; x < i + sizeTiles; x++){
	for(int y = j; y < j + sizeTiles; y++){
	  majImg(x, y, countAlive(x, y));
	  if(cur_img(x, y) != next_img(x, y)){
	    tabTilesHaveMoved[i/sizeTiles][j/sizeTiles] = 1;
	  }
	}
      }
    }
  }
}

///////////////////////////// Version OpenCL

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v3 (unsigned nb_iter)
{
  return ocl_compute(nb_iter);
}

unsigned compute_v0_openmpfor(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp parallel for
    for(unsigned i = 0; i < DIM; i++){
      for(unsigned j = 0; j < DIM; j++){
	majImg(i, j, countAlive(i, j));
      }
    }
    swap_images();
  }
  return 0;
}

void initTabsTiles(){
  tabTilesToMove = malloc(sizeof(int*) * NBTILES);
  tabTilesHaveMoved = malloc(sizeof(int*) * NBTILES);
  for(int i = 0; i < NBTILES; i++){
    tabTilesToMove[i] = malloc(sizeof(int) * NBTILES);
    tabTilesHaveMoved[i] = malloc(sizeof(int) * NBTILES);
    for(int j = 0; j < NBTILES; j++){
      tabTilesToMove[i][j] = 1;
      //tabTilesHaveMoved[i][j] = 0;
    }
  }
}

void freeTabsTiles(){
  for(int i = 0; i < NBTILES; i++){
    free(tabTilesToMove[i]);
    free(tabTilesHaveMoved[i]);
  }
  free(tabTilesToMove);
  free(tabTilesHaveMoved);
}
  
int countAlive(int x, int y){
  int cpt = 0;
  for(int i = x-1; i <= x+1; i++){
    for(int j = y-1; j <= y+1; j++){
      if(i >= 0 && j>= 0){
	if((i!=x || j!=y) && cur_img(i, j) == colorViv){
	  cpt++;
	}
      }
    }
  }
  return cpt;
}

//retourne 0 si l'image n'évoluera pas, 1 sinon
void majImg(int i, int j, int nbVoisinsVivants){
  if(cur_img(i, j) == colorViv){
    if(nbVoisinsVivants == 2 || nbVoisinsVivants == 3){
      next_img(i, j) = colorViv;
    }
    else{
      next_img(i, j) = colorMort;
    }
  }
  else{
    if(nbVoisinsVivants == 3){
      next_img(i, j) = colorViv;
    }
    else{
      next_img(i, j) = colorMort;
    }
  }  
}

int max(int a, int b){
  if(a > b){
    return a;
  }
  return b;
}


int min(int a, int b){
  if(a < b){
    return a;
  }
  return b;
}

static void majTab(){
  for(int x =0;x<NBTILES;x++){
    for (int y = 0; y<NBTILES;y++){
      tabTilesToMove[x][y]=0;
      for(int i = x-1; i <= x+1; i++){
	if(i != -1 && i != NBTILES){
	  for(int j = y-1; j <= y+1; j++){
	    if(j != -1 && j != NBTILES){
	      if (tabTilesHaveMoved[i][j]==1) {
		tabTilesToMove[x][y]=1;
	      }
	    }
	  }
	}
      }
    }
  }
}

static void majTabOmpFor(){
#pragma omp parallel for
  for(int x =0;x<NBTILES;x++){
    for (int y = 0; y<NBTILES;y++){
      tabTilesToMove[x][y]=0;
      for(int i = x-1; i <= x+1; i++){
	if(i != -1 && i != NBTILES){
	  for(int j = y-1; j <= y+1; j++){
	    if(j != -1 && j != NBTILES){
	      if (tabTilesHaveMoved[i][j] == 1) {
		tabTilesToMove[x][y]=1;
	      }
	    }
	  }
	}
      }
    }
  }
}

static void majTabOmpTask(){
#pragma omp parallel
#pragma omp single
  for(int x =0;x<NBTILES;x++){
    for (int y = 0; y<NBTILES;y++){
#pragma omp task
      task_omp_task(x, y);
    }
  }
}

void task_omp_task(int x, int y){
  tabTilesToMove[x][y]=0;
  for(int i=x-1; i <= x+1; i++){
    if(i != -1 && i != NBTILES){
      for(int j=y-1; j <= y+1; j++){
	if(j != -1 && j != NBTILES){
	  if (tabTilesHaveMoved[i][j] == 1) {
	    tabTilesToMove[x][y] = 1;
	  }
	}
      }
    }
  }
}
