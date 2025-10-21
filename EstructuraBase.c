#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#define NUM_FILOSOFOS 5
#define TIEMPO_PENSAR_MIN 1000000
#define TIEMPO_PENSAR_MAX 3000000
#define TIEMPO_COMER_MIN 500000
#define TIEMPO_COMER_MAX 1500000
#define TIEMPO_SIMULACION 30
// Variables globales
sem_t tenedores[NUM_FILOSOFOS];
sem_t comedor;
pthread_mutex_t mutex_print;
int filosofos_comiendo[NUM_FILOSOFOS];
int total_comidas[NUM_FILOSOFOS];   