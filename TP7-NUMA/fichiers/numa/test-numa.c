/* ne *pas* mettre de -O, sinon gcc arrive à éliminer les latences (et puis il optimise mes opérations débiles */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <numa.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <signal.h>
#include <sched.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>

#define TIME_DIFF(t1, t2) \
	((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))


#define MAXMEMSHIFT (6+20)
#define STARTMEMSHIFT (10)
#define MEMSHIFT 1
#define MEMSIZE (1<<MAXMEMSHIFT)
#define STARTSIZE (1<<STARTMEMSHIFT)

#define NLOOPS 10
#define LOOPS 1000000



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

unsigned long readtime;


void generateTab(volatile char* t, int N) {
	int i;
	for(i=0;i<N;i++)
	  t[i]=i+1;
}

int sumTab(volatile char* a, int N, float gold) {
        int i,j;
	int masque=N-1; // 0..01..11
        j=0;

	for(i=0;i<LOOPS;i++){
	  j+=a[j]+gold;
	  j&=masque;
	}
	return j;
}



int main(int argc, char*argv[]) {
	int mems,m,cpu;
	long mem;
	volatile char *c;
	char sum=0;
	unsigned int gold;
	int node, lastnode;
	tick_t t1,t2;

	
	cpu_set_t cpusbind;



	if (argc<3) {
		printf("usage: %s cpu node\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	cpu = atoi(argv[1]);
	node = atoi(argv[2]);

	/*************************************/
	/* Vérifier le nombre de noeuds NUMA */
	/*************************************/
	/* Version linux */
	if (numa_available()<0) {
		fprintf(stderr,"numa not available\n");
		exit(EXIT_FAILURE);
	}
	numa_set_bind_policy(1);
	numa_set_strict(1);
	lastnode = numa_max_node();
	if (lastnode<node) {
	        fprintf(stderr,"numa node not available\n");
		exit(EXIT_FAILURE);
	}

	/***********************************/
	/* Migrer vers le processeur donné */
	/***********************************/

	/* Version linux */
	CPU_ZERO(&cpusbind);
	CPU_SET(cpu,&cpusbind);
	if (sched_setaffinity(0, sizeof(cpusbind), &cpusbind)) {
		perror("sched_setaffinity");
		exit(EXIT_FAILURE);
	}

	/***************************/
	/* Allouer la zone mémoire */
	/***************************/
	//	printf("allocating on node %d\n",node);

	/* Version linux */
	c = numa_alloc_onnode(MEMSIZE, node);

	/* Toucher vraiment la zone mémoire allouée */
	generateTab(c,MEMSIZE);
	//	memset((void*)c,0,MEMSIZE);
	//	memcpy((void*)c,(void*)c,MEMSIZE);

	/*******************************/
	/* boucle sur la taille buffer */
	/*******************************/

	for (mems=STARTMEMSHIFT,mem=1<<mems;mems<=MAXMEMSHIFT;mems+=MEMSHIFT,mem<<=MEMSHIFT) {
		gold = ((float)mem*((sqrt(5)-1.)/2.));
		sum+=sumTab(c,mem,gold); // on préchauffe
		GET_TICK(t1);
		for (m=0;m<NLOOPS;m++) {
		  sum+=sumTab(c,mem,gold);
		}
		GET_TICK(t2);
		readtime = (TICK_DIFF(t1,t2)/NLOOPS);
		fprintf(stderr,"%lu %d-%d %4lu\n", mem, cpu, node, readtime);
	}
	return sum;
}
