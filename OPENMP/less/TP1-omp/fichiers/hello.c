#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel 

{
  //single le premier arrivé execute le code nowait sans attente
  //master le code est executé par le thread principal 0
  #pragma omp master
   printf("Bonjour je suis le thread %d \n",omp_get_thread_num());

  // #pragma omp barrier

   printf("Au revoir je suis le thread  %d \n",omp_get_thread_num());
}
  return 0;
}
