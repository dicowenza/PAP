#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

#define TIME_DIFF(t1, t2) \
		((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

#define MESURES 3
#define N 1024*16
int main(void) {
	long int sum0, sum1, sum2;
	unsigned char *c;
	int i, j, mesure;
	struct timeval tv1,tv2;

	c = malloc(N*sizeof(*c));
	for (i=0; i<N; i++)
		c[i] = random()%3;

	for (mesure=0; mesure<MESURES; mesure++) {
		gettimeofday(&tv1,NULL);
		for (j=0; j<200000; j++) {
			sum0 = 0;
			sum1 = 0;
			sum2 = 0;
			for (i=0; i<N; i++) {
				if (c[i] == 0)
					sum0++;
				else if (c[i] == 1)
					sum1++;
			}
			sum2 = N-sum0-sum1;
		}
		gettimeofday(&tv2,NULL);
		printf(" version 0 sum0 %ld sum1 %ld sum2 %ld %.3f sec\n", sum0, sum1, sum2, ((float)(TIME_DIFF(tv1,tv2)))/1000000);
	}

#if 1
	for (mesure=0; mesure<MESURES; mesure++) {
		gettimeofday(&tv1,NULL);
		for (j=0; j<200000; j++) {
			sum0 = 0;
			sum1 = 0;
			sum2 = 0;
			for (i=0; i<N; i++) {
				sum1 += c[i] & 1;
				sum2 += c[i] & 2;
			}
		}
		sum2 = sum2 / 2;
		sum0 = N-sum2-sum1;
		gettimeofday(&tv2,NULL);
		printf("version & sum0 %ld sum1 %ld sum2 %ld %.3f sec\n", sum0, sum1, sum2, ((float)(TIME_DIFF(tv1,tv2)))/1000000);
	}
#endif
#if 1
	for (mesure=0; mesure<MESURES; mesure++) {
		gettimeofday(&tv1,NULL);
		for (j=0; j<200000; j++) {
			sum0 = 0;
			sum1 = 0;
			sum2 = 0;
			for (i=0; i<N; i++) {
				sum1 += c[i] == 1;
				sum2 += c[i] == 2;
			}
		}
		sum2 = sum2 ;
		sum0 = N-sum2-sum1;
		gettimeofday(&tv2,NULL);
		printf(" version == sum0 %ld sum1 %ld sum2 %ld %.3f sec\n", sum0, sum1, sum2, ((float)(TIME_DIFF(tv1,tv2)))/1000000);
	}
#endif

#if 1
	long int sum[3];
	for (mesure=0; mesure<MESURES; mesure++) {
		gettimeofday(&tv1,NULL);
		for (j=0; j<200000; j++) {
			sum[0]=sum[1]=sum[2]=0;
			for (i=0; i<N; i++) {
				sum[c[i]]++;
			}
		}
		gettimeofday(&tv2,NULL);
		printf("version tab sum0 %ld sum1 %ld sum2 %ld %.3f sec\n", sum[0], sum[1], sum[2], ((float)(TIME_DIFF(tv1,tv2)))/1000000);
	}
#endif

	return 0;
}
