#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _thrarg_t {
    int tid;
    int n;
} thrarg_t;

// Beginn globale Variablen für Barrier
sem_t sem_waiting;
sem_t sem_mutex; // we simulate a mutex using a semaphore
int count;
// Ende globale Variablen für Barrier

void *work(void *arg) {
    thrarg_t *targ = (thrarg_t *)arg;
    int me = targ->tid;
    int n = targ->n;

    printf("Thread: %d vor der Barrier\n", me);

    // Beginn Barrier
    sem_wait(&sem_mutex); // this ensures only one thread access count concurrently
    count++;
    if (count < n) {
        // if not all threads have reached this point then wait
        sem_post(&sem_mutex); // frees the mutex
        sem_wait(&sem_waiting);
    }
    // each thread wake one other thread with sem_post.
    sem_post(&sem_waiting);
    // note that after the barrier the same semaphores can not be used for another barrier
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
    sem_init(&sem_waiting, 1, 0);
    sem_init(&sem_mutex, 1, 1);
    count = 0;
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
