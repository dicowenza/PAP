#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>

/*
 *
 * Mesure de temps à l'aide du compteur de cycles
 * (spécifique à x86 et x86_64)
 *
 */

typedef union {
	unsigned long long tick;
	struct {
		unsigned low;
		unsigned high;
	};
} tick_t;

#define GET_TICK(t) \
	   __asm__ volatile("rdtsc" : "=a" ((t).low), "=d" ((t).high))
#define TICK_DIFF(t1, t2) \
	   ((t2).tick - (t1).tick)

#define MESURES 10
#define M 100000 // taille totale du tableau.

unsigned char c[M];

int main(int argc, char *argv[]) {
	long int sum255 = 0, sum0 = 0;
	int i, j, k, mesure, n;
	struct timeval tv1,tv2;
	tick_t t1,t2;


	for (i=0; i<M; i++)
		c[i] =  (random()%256);


	for(int N = 2; N < (1 << 16); N*=2)
	for (mesure=0; mesure<MESURES; mesure++) {
	  GET_TICK(t1);
	 
		for (k=0; k<1000; k++) 
		 {
		   i = 0;
			n = random()%(M-N); 
			for (j=0; j<10000; j++) {
				// on parcourt 10 000 fois la portion M..M+N
				// pour N petit, le processeur arrive a se
				// souvenir quelle branche du if on va prendre
			  
				if (c[i+n] >= (unsigned char) 128)
				  {
				    sum255++;
				    sum0--;
					}
				else
				  {
				    sum0++;
				    sum255--;
				  }
				i = (i + 1) & (N-1); // i = (i + 1) % N lorsque N = 2^a
			  }

		}
		GET_TICK(t2);
		fprintf(stderr,"%d %lld\n", N, TICK_DIFF(t1,t2));
		//	gettimeofday(&tv2,NULL);
		printf(" %ld \n", sum0 + sum255);
	}
	return 0;
}
