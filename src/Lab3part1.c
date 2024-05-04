#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

void *factorial(void *arg);

int main(int argc, char *argv[]) {
    pthread_t hilos[argc - 1]; 

    for (int i = 1; i < argc; i++) {
        int *num = (int *)malloc(sizeof(int));
        *num = atoi(argv[i]);
        pthread_create(&hilos[i - 1], NULL, factorial, (void *)num);
    }

    for (int i = 0; i < argc - 1; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Hilo principal\n");

    return 0;
}

void *factorial(void *arg) {
    int n = *((int *)arg);
    free(arg); 
    long long int resultado = 1;
    int num;
    for (num = 2; num <= n; num++) {
        resultado = resultado * num;
        printf("Factorial de %d, resultado parcial %lld\n", n, resultado);
        sleep(random() % 3);
    }
    printf("Hilo de num=%d terminado\n", n);
    return NULL;
}
