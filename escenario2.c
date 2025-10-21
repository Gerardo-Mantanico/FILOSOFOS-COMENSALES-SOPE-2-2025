#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_FILOSOFOS 5
#define TIEMPO_PENSAR_MIN 1000000 
#define TIEMPO_PENSAR_MAX 2000000 
#define TIEMPO_COMER_MIN 500000   
#define TIEMPO_COMER_MAX 1000000   
#define TIEMPO_SIMULACION 60


sem_t tenedores[NUM_FILOSOFOS];
sem_t comedor; // Semáforo que previene el interbloqueo
pthread_mutex_t print_mutex;
volatile int keep_running = 1;

// *** NUEVO: Array para contar las comidas de cada filósofo ***
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
        // --- PENSAR ---
        pthread_mutex_lock(&print_mutex);
        printf("[Filósofo %d] -> Pensando...\n", id);
        pthread_mutex_unlock(&print_mutex);
        usleep(rand_range(TIEMPO_PENSAR_MIN, TIEMPO_PENSAR_MAX));

        // --- INTENTAR ENTRAR AL COMEDOR ---
        pthread_mutex_lock(&print_mutex);
        printf("[Filósofo %d] Tiene hambre, quiere entrar al comedor.\n", id);
        pthread_mutex_unlock(&print_mutex);
        sem_wait(&comedor);

        // --- TOMAR TENEDORES ---
        sem_wait(&tenedores[tenedor_izq]);
        sem_wait(&tenedores[tenedor_der]);

        // --- COMER ---
        pthread_mutex_lock(&print_mutex);
        // *** ACCIÓN CLAVE: Incrementar el contador de comidas ***
        total_comidas[id]++;
        printf(">>>>>>>>>> [Filósofo %d] COMIENDO por %da vez. <<<<<<<<<<\n", id, total_comidas[id]);
        pthread_mutex_unlock(&print_mutex);
        
        usleep(rand_range(TIEMPO_COMER_MIN, TIEMPO_COMER_MAX));

        // --- DEJAR DE COMER ---
        sem_post(&tenedores[tenedor_izq]);
        sem_post(&tenedores[tenedor_der]);
        
        pthread_mutex_lock(&print_mutex);
        printf("[Filósofo %d] Terminó de comer. Sale del comedor.\n", id);
        pthread_mutex_unlock(&print_mutex);
        
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
    sem_init(&comedor, 0, NUM_FILOSOFOS - 1); // La solución al interbloqueo

    printf("--- INICIO: Análisis de Equidad (60 segundos) ---\n");
    printf("Contando el número de comidas por filósofo para detectar inanición.\n\n");
    
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&filosofos[i], NULL, filosofo, id);
    }

    sleep(TIEMPO_SIMULACION);
    keep_running = 0;
    
    printf("\n--- SIMULACIÓN TERMINADA ---\n");
    
    // Dar un par de segundos para que los hilos terminen su ciclo actual
    sleep(3);

    printf("      RESULTADOS FINALES DE COMIDAS\n");
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filósofo %d comió un total de: %d veces.\n", i, total_comidas[i]);
    }
    printf("Análisis: Si ningún filósofo tiene 0 comidas y los\n");
    printf("valores son razonablemente cercanos, la solución es equitativa.\n");


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