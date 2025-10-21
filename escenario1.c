#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_FILOSOFOS 5
#define TIEMPO_SIMULACION 10

sem_t tenedores[NUM_FILOSOFOS];
pthread_mutex_t print_mutex;
sem_t barrera; // Nuevo semáforo para sincronizar el bloqueo
int listos_para_bloqueo = 0; // Contador de filósofos listos

void *filosofo(void *arg) {
    int id = *(int*)arg;
    int tenedor_izq = id;
    int tenedor_der = (id + 1) % NUM_FILOSOFOS;

    pthread_mutex_lock(&print_mutex);
    printf("[Filósofo %d] -> Pensando...\n", id);
    pthread_mutex_unlock(&print_mutex);
    usleep(50000 * id);

    pthread_mutex_lock(&print_mutex);
    printf("[Filósofo %d] Tiene hambre. Intentando tomar tenedor izquierdo (%d).\n", id, tenedor_izq);
    pthread_mutex_unlock(&print_mutex);
    
    sem_wait(&tenedores[tenedor_izq]);

    pthread_mutex_lock(&print_mutex);
    printf("[Filósofo %d] ¡ÉXITO! Tomó tenedor izquierdo (%d).\n", id, tenedor_izq);
    
    listos_para_bloqueo++;
    if (listos_para_bloqueo == NUM_FILOSOFOS) {
          printf("\nTodos los filosofos han tomado su tenedor izquierzo. \n");
          printf("\nAhora todos intentaran tomar el tenedor derecho simultaneamente. \n");
        // Libera a todos los que esperan en la barrera
        for(int i=0; i<NUM_FILOSOFOS; i++) {
            sem_post(&barrera);
        }
    }
    pthread_mutex_unlock(&print_mutex);
    
    sem_wait(&barrera); // Todos los filósofos esperan aquí

    // --- PASO 3: INTENTO DE TOMAR EL TENEDOR DERECHO (¡AQUÍ OCURRE EL DEADLOCK!) ---
    pthread_mutex_lock(&print_mutex);
    printf("[Filósofo %d] Esperando por tenedor derecho (%d), que lo tiene el Filósofo %d.\n", id, tenedor_der, tenedor_der);
    pthread_mutex_unlock(&print_mutex);

    sem_wait(&tenedores[tenedor_der]); // <-- NUNCA PASARÁ DE AQUÍ

    // Esta parte del código es inalcanzable
    printf("ESTE MENSAJE NUNCA DEBERÍA APARECER - FILÓSOFO %d\n", id);

    free(arg);
    return NULL;
}

int main() {
    pthread_t filosofos[NUM_FILOSOFOS];
    int i;
    pthread_mutex_init(&print_mutex, NULL);
    sem_init(&barrera, 0, 0);

    for (i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1);
    }

    for (i = 0; i < NUM_FILOSOFOS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&filosofos[i], NULL, filosofo, id);
    }

    // Esperamos a que los hilos se ejecuten y lleguen al deadlock
    sleep(TIEMPO_SIMULACION);
    
    printf("\n--- FIN DE LA SIMULACIÓN ---\n");
    printf("El programa está congelado como se esperaba.\n");
    printf("Cada filósofo tiene un tenedor y espera por el del vecino, creando un círculo de espera sin fin.\n");


    // El programa se quedará "colgado" aquí.
    for (i = 0; i < NUM_FILOSOFOS; i++) {
         pthread_join(filosofos[i], NULL);
    }

    return 0;
}