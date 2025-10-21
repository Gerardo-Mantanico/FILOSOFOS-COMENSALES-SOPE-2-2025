#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_FILOSOFOS 3
#define TIEMPO_PENSAR_MIN 50000   // 50ms
#define TIEMPO_PENSAR_MAX 100000  // 100ms
#define TIEMPO_COMER_MIN 100000   // 100ms
#define TIEMPO_COMER_MAX 300000   // 300ms
#define TIEMPO_SIMULACION 30

// Variables Globales
sem_t tenedores[NUM_FILOSOFOS];
sem_t comedor;
pthread_mutex_t print_mutex;
volatile int keep_running = 1;
int total_comidas[NUM_FILOSOFOS] = {0};

unsigned int rand_range(unsigned int min, unsigned int max) {
    return min + rand() % (max - min + 1);
}

void *filosofo(void *arg) {
    int id = *(int*)arg;
    int tenedor_izq = id;
    int tenedor_der = (id + 1) % NUM_FILOSOFOS;
    srand(time(NULL) + id);

    while (keep_running) {
        // Pensar
        usleep(rand_range(TIEMPO_PENSAR_MIN, TIEMPO_PENSAR_MAX));
        sem_wait(&comedor);

        // Tomar ambos tenedores
        sem_wait(&tenedores[tenedor_izq]);
        sem_wait(&tenedores[tenedor_der]);

        // Comer
        pthread_mutex_lock(&print_mutex);

        total_comidas[id]++;
        printf("Filósofo %d comiendo (total: %d)\n", id, total_comidas[id]);
        pthread_mutex_unlock(&print_mutex);
        usleep(rand_range(TIEMPO_COMER_MIN, TIEMPO_COMER_MAX));

        // Soltar tenedores
        sem_post(&tenedores[tenedor_izq]);
        sem_post(&tenedores[tenedor_der]);
        
        sem_post(&comedor);
    }
    free(arg);
    return NULL;
}

int main() {
    pthread_t filosofos[NUM_FILOSOFOS];
    int i;

    pthread_mutex_init(&print_mutex, NULL);

    for (i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1);
    }


    sem_init(&comedor, 0, NUM_FILOSOFOS - 1);

    printf("--- INICIO: Análisis de Robustez con alta concurrencia (30 segundos) ---\n");
    
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&filosofos[i], NULL, filosofo, id);
    }

    sleep(TIEMPO_SIMULACION);
    keep_running = 0;
    
    printf("\n--- SIMULACIÓN TERMINADA ---\n");
    sleep(2);

    printf("      RECUENTO FINAL DE COMIDAS\n");
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filósofo %d comió un total de: %d veces.\n", i, total_comidas[i]);
    }
    
    // Limpieza de recursos
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_cancel(filosofos[i]);
    }
    pthread_mutex_destroy(&print_mutex);
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        sem_destroy(&tenedores[i]);
    }
    sem_destroy(&comedor);
    return 0;
}