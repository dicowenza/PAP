#include <stdio.h>
#include <omp.h>

int main()
{
//#pragma omp parallel
#pragma omp critical
  { printf("Bonjour je suis le thread numero %i!\n",omp_get_thread_num);
//#pragma omp barrier
   printf("Au revoir je le thread numero! %i\n",omp_get_thread_num);}
   

  return 0;
}
