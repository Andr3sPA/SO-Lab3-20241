/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 *
 * @param[in] <-p> {vector size}
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create}
 * @param[in] <-i> {maximum itertions}
 *
 * @author     Danny Munera
 * @date       2020
 */
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <getopt.h>
int limJ = 0;
float limH = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct
{
	double *X;
	double a;
	double *Y;
	double *Y_avgs;
	int max_iters;
	int p;
	int n_threads;
	// int i_thread;
} myarg_t;

int i_thread = 0;

void *mythread(void *arg)
{
	myarg_t *args = (myarg_t *)arg;
	int n_threads = args->n_threads;
	double *X = args->X;
	double *Y = args->Y;
	double a = args->a;
	double *Y_avgs = args->Y_avgs;
	int max_iters = args->max_iters;
	int p = args->p;
	pthread_mutex_lock(&mutex);
	int threadIdx = i_thread;
	i_thread++;
	pthread_mutex_unlock(&mutex);

	// limJ += (max_iters / n_threads);
	float partitionSize = (p * 1.0 / n_threads);
	int startH = threadIdx * partitionSize;
	int limH = (threadIdx + 1) * partitionSize;

	int jj, hh;
	double jjAvg = 0;
	for (jj = 0; jj < max_iters; jj++)
	{
		for (hh = startH; hh < limH; hh++)
		{
			Y[hh] = Y[hh] + a * X[hh];
			jjAvg += Y[hh];
		}
		pthread_mutex_lock(&mutex);
		Y_avgs[jj] += jjAvg;
		pthread_mutex_unlock(&mutex);
		jjAvg = 0;
	}

	pthread_mutex_lock(&mutex);
	printf("\x1B[34mThread %d: (%d - %d), limH = %d, it = %d\n", threadIdx, startH, hh, limH, jj);
	printf("Y[%d] = %f\n", hh - 1, Y[hh - 1]);
	printf("Y_avgs[%d] = %f\x1B[0m\n", jj - 1,  Y_avgs[jj - 1]);
	pthread_mutex_unlock(&mutex);

	return NULL;
}

int main(int argc, char *argv[])
{
	// Variables to obtain command line parameters
	unsigned int seed = 1;
	int p = 10000000;
	int n_threads = 2;
	int max_iters = 1000;
	// Variables to perform SAXPY operation
	double *X;
	double a;
	double *Y;
	double *Y_avgs;
	int i;
	// Variables to get execution time
	struct timeval t_start, t_end;
	double exec_time;

	// Getting input values
	int opt;
	while ((opt = getopt(argc, argv, ":p:s:n:i:")) != -1)
	{
		switch (opt)
		{
		case 'p':
			printf("vector size: %s\n", optarg);
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647);
			break;
		case 's':
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
		case 'n':
			printf("threads number: %s\n", optarg);
			n_threads = strtol(optarg, NULL, 10);
			break;
		case 'i':
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;
		case ':':
			printf("option -%c needs a value\n", optopt);
			break;
		case '?':
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum itertions>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	srand(seed);

	printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n",
		   p, seed, n_threads, max_iters);

	// initializing data
	X = (double *)malloc(sizeof(double) * p);
	Y = (double *)malloc(sizeof(double) * p);
	Y_avgs = (double *)malloc(sizeof(double) * max_iters);

	for (i = 0; i < p; i++)
	{
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	for (i = 0; i < max_iters; i++)
	{
		Y_avgs[i] = 0.0;
	}
	a = (double)rand() / RAND_MAX;

#ifdef DEBUG
	double *YRES = malloc(sizeof(double) * p);
	for (i = 0; i < p; i++)
	{
		YRES[i] = X[i] * a * max_iters + Y[i];
	}
	printf("\x1B[34mvector X= [ ");
	for (i = 0; i < p - 1; i++)
	{
		printf("%f, ", X[i]);
	}
	printf("%f ]\n", X[p - 1]);

	printf("\x1B[32mvector Y= [ ");
	for (i = 0; i < p - 1; i++)
	{
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p - 1]);

	printf("\x1B[31ma= %f \x1B[0m\n", a);
    free(YRES);
#endif

	/*
	 *	Function to parallelize
	 */
	gettimeofday(&t_start, NULL);
	// SAXPY iterative SAXPY mfunction
	pthread_t hilos[n_threads];
	for (int i = 0; i < n_threads; i++)
	{
		myarg_t args = {X, a, Y, Y_avgs, max_iters, p, n_threads};
		pthread_create(&hilos[i], NULL, mythread, &args);
	}

	for (int i = 0; i < n_threads; i++)
	{
		pthread_join(hilos[i], NULL);
	}

	for (int j = 0; j < max_iters; j++)
	{
		Y_avgs[j] = Y_avgs[j] / p;
	}

	gettimeofday(&t_end, NULL);

#ifdef DEBUG
	printf("\x1B[32mRES: final vector Y= [ ");
	for (i = 0; i < p - 1; i++)
	{
		printf("%f, ", Y[i]);
	}
	printf("%f ]\x1B[0m\n", Y[p - 1]);
	printf("\x1B[36mFinal RES should be Y = [");
	for (i = 0; i < p - 1; i++)
	{
		printf("%f, ", YRES[i]);
	}
	printf("%f ]\x1B[0m\n", YRES[p - 1]);
#endif

	// Computing execution time
	exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;	 // sec to ms
	exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
	printf("Execution time: %f ms \n", exec_time);
	printf("Last 3 values of Y: %f, %f, %f \n", Y[p - 3], Y[p - 2], Y[p - 1]);
	printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters - 3], Y_avgs[max_iters - 2], Y_avgs[max_iters - 1]);
    free(X);
    free(Y);
    free(Y_avgs);
	return 0;
}