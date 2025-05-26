/*
    Protokoll a)
    Read Thread 1: #0
    Read Thread 2: #0
    Read Thread 2: #1
    Read Thread 1: #1
    Read Thread 2: #2
    Read Thread 1: #2
    Read Thread 2: #3
    Read Thread 1: #3
    Read Thread 2: #4
    Read Thread 1: #4
    Write Thread: #0
    Write Thread: #1
    Write Thread: #2
    Write Thread: #3
    Write Thread: #4

    Protokoll b)
    Read Thread 1: #0
    Read Thread 2: #0
    Write Thread: #0
    Write Thread: #1
    Read Thread 1: #1
    Write Thread: #2
    Write Thread: #3
    Read Thread 1: #2
    Write Thread: #4
    Read Thread 2: #1
    Read Thread 1: #3
    Read Thread 1: #4
    Read Thread 2: #2
    Read Thread 2: #3
    Read Thread 2: #4
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Aktiviert die Verbesserung
#ifndef A3_2
    #define A3_2
#endif

typedef struct _rwlock_t {
    pthread_mutex_t m;
    pthread_cond_t c;
    int num_r, num_w;
    #ifdef A3_2
    int is_waiting;
    #endif
} rw_lock_t;

void rw_lock_init(rw_lock_t *rwl);
int rw_lock_rlock(rw_lock_t *rwl);
int rw_lock_wlock(rw_lock_t *rwl);
int rw_lock_runlock(rw_lock_t *rwl);
int rw_lock_wunlock(rw_lock_t *rwl);

void* read_thread_1(void* _arg);
void* read_thread_2(void* _arg);
void* write_thread(void* _arg);

rw_lock_t RWL;
const int NUM_READS = 5;
const int NUM_WRITES = 5;

int main() {
    rw_lock_init(&RWL);
    pthread_t pids[3];
    pthread_create(&pids[0], NULL, read_thread_1, NULL);
    pthread_create(&pids[1], NULL, read_thread_2, NULL);
    pthread_create(&pids[2], NULL, write_thread, NULL);
    
    for (int i = 0; i < sizeof(pids) / sizeof(pthread_t); i++) {
        pthread_join(pids[i], NULL);
    }

    return 0;
}

void rw_lock_init(rw_lock_t *rwl) {
    rwl->num_r = rwl->num_w = 0;
    #ifdef A3_2
    rwl->is_waiting = 0;
    #endif
    pthread_mutex_init(&rwl->m, NULL);
    pthread_cond_init(&rwl->c, NULL);
}

int rw_lock_rlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);
    #ifdef A3_2
    while (rwl->num_w > 0 || rwl->is_waiting > 0) {
        pthread_cond_wait(&rwl->c, &rwl->m);
    }
    #else
    while (rwl->num_w > 0) {
        pthread_cond_wait(&rwl->c, &rwl->m);
    }
    #endif
    rwl->num_r++;
    pthread_mutex_unlock(&rwl->m);
    return 0;
}

int rw_lock_wlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);
    #ifdef A3_2
    while (rwl->num_w > 0 || rwl->num_r > 0) {
        rwl->is_waiting++;
        pthread_cond_wait(&rwl->c, &rwl->m);
        rwl->is_waiting--;
    }
    #else
    while (rwl->num_w > 0 || rwl->num_r > 0) {
        pthread_cond_wait(&rwl->c, &rwl->m);
    }
    #endif
    rwl->num_w = 1;
    pthread_mutex_unlock(&rwl->m);
    return 0;
}

int rw_lock_runlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);
    rwl->num_r--;
    #ifdef A3_2
    if (rwl->num_r == 0 || rwl->is_waiting > 0) {
        pthread_cond_broadcast(&rwl->c);
    }
    #else
    if (rwl->num_r == 0) {
        pthread_cond_signal(&rwl->c);
    }
    #endif
    pthread_mutex_unlock(&rwl->m);
    return 0;
}

int rw_lock_wunlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);
    rwl->num_w = 0;
    pthread_cond_broadcast(&rwl->c);
    pthread_mutex_unlock(&rwl->m);
    return 0;
}

void* read_thread_1(void* _arg) {
    for (int i = 0; i < NUM_READS; i++) {
        rw_lock_rlock(&RWL);
        printf("Read Thread 1: #%d\n", i);
        sleep(2);
        rw_lock_runlock(&RWL);
    }
    return NULL;
}

void* read_thread_2(void* _arg) {
    for (int i = 0; i < NUM_READS; i++) {
        rw_lock_rlock(&RWL);
        printf("Read Thread 2: #%d\n", i);
        if (i == 0) {
            sleep(1);
        }
        else {
            sleep(2);
        }
        rw_lock_runlock(&RWL);
    }
    return NULL;
}

void* write_thread(void* _arg) {
    for (int i = 0; i < NUM_WRITES; i++) {
        rw_lock_wlock(&RWL);
        printf("Write Thread: #%d\n", i);
        rw_lock_wunlock(&RWL);
    }
    return NULL;
}