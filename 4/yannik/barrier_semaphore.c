#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _thrarg_t {
    int tid;
    int n;
} thrarg_t;

// Beginn globale Variablen für Barrier
sem_t barrier_semaphore;
sem_t lock_semaphore;
int barrier_count;
// Ende globale Variablen für Barrier

void *work(void *arg) {
    thrarg_t *targ = (thrarg_t *)arg;
    int me = targ->tid;
    int n = targ->n;

    printf("Thread: %d vor der Barrier\n", me);

    // Beginn Barrier

    // Nutze semaphore als lock
    sem_wait(&lock_semaphore);
    ++barrier_count;

    if (barrier_count == n) {
        for (int i = 0; i < n - 1; ++i)
            sem_post(
                &barrier_semaphore); // Wecke andere threads (-1 weil aktiver Thread nicht wartet)
        sem_post(&lock_semaphore);
    } else {
        sem_post(&lock_semaphore);
        sem_wait(&barrier_semaphore);
    }
    // Ende Barrier

    printf("Thread: %d nach der Barrier\n", me);
    return NULL;
}

int main(int argc, char *argv[]) {
    int i, n;

    if (argc != 2) {
        printf("Usage: %s <numthreads> \n", argv[0]);
        exit(1);
    }
    n = atoi(argv[1]);

    pthread_t tid[n];
    thrarg_t arg[n];

    // Beginn Initialisierung der globalen Variablen für Barrier
    sem_init(&lock_semaphore, 0, 1);
    sem_init(&barrier_semaphore, 0, 0);
    barrier_count = 0;
    // Ende Initialisierung der globalen Variablen für Barrier

    for (i = 0; i < n; ++i) {
        arg[i].tid = i;
        arg[i].n = n;
        pthread_create(&tid[i], NULL, work, &arg[i]);
    }

    for (i = 0; i < n; ++i)
        pthread_join(tid[i], NULL);

    return EXIT_SUCCESS;
}