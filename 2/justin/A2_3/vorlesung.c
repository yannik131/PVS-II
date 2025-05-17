#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h> // needed for sleep()
#define EBUSY 16

void *lock_backward(void *arg);
void *lock_forward(void *arg);

pthread_mutex_t mutex[3] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};

int backoff = 1; // == 1: mit Backoff-Strategie
int yield_flag = 0; // > 0: Verwende sched_yield, <= 0: sleep

int main(int argc, char *argv[]) {
    pthread_t f, b;

    if (argc > 1) backoff = atoi(argv[1]);
    if (argc > 2) yield_flag = atoi(argv[2]);
    pthread_create(&f, NULL, lock_forward, NULL);
    pthread_create(&b, NULL, lock_backward, NULL);
    pthread_exit(NULL); // die beiden anderen Threads laufen weiter
}

void *lock_forward(void *args) {
    int iterate, i, status;
    for (iterate = 0; iterate < 10; iterate++) {
        for (i = 0; i < 3; i++) {
            if (i == 0 || !backoff)
                status = pthread_mutex_lock(&mutex[i]);
            else status = pthread_mutex_trylock(&mutex[i]);
            if (status == EBUSY)
                for (--i; i >= 0; i--) pthread_mutex_unlock(&mutex[i]);
            else printf("forward locker got mutex %d\n", i);
            if (yield_flag) {
                if (yield_flag > 0) sched_yield();
                else sleep(1);
            }
        }
        for (i = 2; i >= 0; i--)
            pthread_mutex_unlock(&mutex[i]);
        sched_yield(); // Neuer Versuch mit evtl . anderer Reihenfolge
    }
}

void *lock_backward(void *arg) {
    int iterate, i, status;
    for (iterate = 0; iterate < 10; iterate++) {
        for (i = 2; i >= 0; i--) {
            if (i == 2 || !backoff)
                status = pthread_mutex_lock(&mutex[i]);
            else status = pthread_mutex_trylock(&mutex[i]);
            if (status == EBUSY)
                for (++i; i < 3; i++) pthread_mutex_unlock(&mutex[i]);
            else printf("backward locker got mutex %d\n", i);
            if (yield_flag)
                if (yield_flag > 0) sched_yield();
                else sleep(1);
        }
        for (i = 0; i < 3; i++)
            pthread_mutex_unlock(&mutex[i]);
        sched_yield();
    }
}