#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

/* --- Variables Globales Configurables --- */
int NUM_FILOSOFOS;
long TIEMPO_PENSAR_MIN_US;
long TIEMPO_PENSAR_MAX_US;
long TIEMPO_COMER_MIN_US;
long TIEMPO_COMER_MAX_US;

sem_t *tenedores;
// *** CLAVE: El semáforo "comedor" se ajustará a N-1 en el main. ***
sem_t comedor;
int *total_comidas;

pthread_mutex_t print_mutex;
volatile int keep_running = 1;

long rand_range(long min, long max) {
    return min + rand() % (max - min + 1);
}

void *filosofo(void *arg) {
    int id = *(int*)arg;
    int tenedor_izq = id;
    int tenedor_der = (id + 1) % NUM_FILOSOFOS;
    srand(time(NULL) + id);

    while (keep_running) {
        usleep(rand_range(TIEMPO_PENSAR_MIN_US, TIEMPO_PENSAR_MAX_US));
        sem_wait(&comedor);
        sem_wait(&tenedores[tenedor_izq]);
        sem_wait(&tenedores[tenedor_der]);
        
        pthread_mutex_lock(&print_mutex);
        total_comidas[id]++;
        pthread_mutex_unlock(&print_mutex);
        
        usleep(rand_range(TIEMPO_COMER_MIN_US, TIEMPO_COMER_MAX_US));
        sem_post(&tenedores[tenedor_izq]);
        sem_post(&tenedores[tenedor_der]);
        sem_post(&comedor);
    }
    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Uso: %s <num_filosofos> <pensar_min_ms> <pensar_max_ms> <comer_min_ms> <comer_max_ms> <tiempo_sim_seg>\n", argv[0]);
        return 1;
    }

    // *** CLAVE: Las variables se leen de la línea de comandos. ***
    NUM_FILOSOFOS = atoi(argv[1]);
    TIEMPO_PENSAR_MIN_US = atoi(argv[2]) * 1000;
    TIEMPO_PENSAR_MAX_US = atoi(argv[3]) * 1000;
    TIEMPO_COMER_MIN_US = atoi(argv[4]) * 1000;
    TIEMPO_COMER_MAX_US = atoi(argv[5]) * 1000;
    int tiempo_simulacion = atoi(argv[6]);

    // Asignación de memoria dinámica basada en NUM_FILOSOFOS
    tenedores = malloc(NUM_FILOSOFOS * sizeof(sem_t));
    total_comidas = calloc(NUM_FILOSOFOS, sizeof(int)); // calloc inicializa en 0

    pthread_t filosofos[NUM_FILOSOFOS];
    int i;
    pthread_mutex_init(&print_mutex, NULL);

    // *** CLAVE: El semáforo se inicializa con el valor N-1 leído de los argumentos. ***
    sem_init(&comedor, 0, NUM_FILOSOFOS - 1);
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1);
    }

    printf("--- Iniciando simulación para %d filósofos por %d segundos ---\n", NUM_FILOSOFOS, tiempo_simulacion);

    for (i = 0; i < NUM_FILOSOFOS; i++) {
        int *id = malloc(sizeof(int)); *id = i;
        pthread_create(&filosofos[i], NULL, filosofo, id);
    }

    sleep(tiempo_simulacion);
    keep_running = 0;
    printf("\n--- SIMULACIÓN TERMINADA ---\n");
    sleep(2);

    printf("      RECUENTO FINAL DE COMIDAS (%d Filósofos)\n", NUM_FILOSOFOS);
    for (i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filósofo %d comió un total de: %d veces.\n", i, total_comidas[i]);
    }

    free(tenedores);
    free(total_comidas);
    return 0;
}

// ./variable 12 500 1500 500 1000 20
// filósofos comensales con 12 filósofos durante 20 segundos. Cada filósofo debe pensar por un tiempo aleatorio entre 500 y 1500 milisegundos, y comer por un tiempo aleatorio entre 500 y 1000 milisegundos.
