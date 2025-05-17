/**
 * @note 2.3 a)
 * Vgl. Vorlesung Betriebssysteme, Coffmann (1971):
 * - Wechselseitiger Ausschluss (mutual exclusion): Jedes involvierte Betriebsmittel ist entweder
 * exklusiv belegt oder frei
 * - Zusätzliche Belegung (Hold-and-wait): Die Prozesse haben bereits Betriebsmittel belegt, wollen
 * zusätzliche Betriebsmittel belegen und warten darauf, dass sie frei werden.
 * - Keine vorzeitige Rückgabe (No preemption): Bereits belegte Betriebsmittel können den Prozessen
 * nicht einfach wieder entzogen werden
 * - Gegenseitiges Warten (Circulat wait): Es existiert ein Zyklus von zwei oder mehr Prozessen, bei
 * denen jeweils einer die Betriebsmittel vom nächsten belegen will, die dieser belegt hat
 */

#include <errno.h> // Für EBUSY
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Für sleep()

pthread_mutex_t mutex[3] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
                            PTHREAD_MUTEX_INITIALIZER};

int backoff = 1;    // == 1: mit Backoff-Strategie
int yield_flag = 0; // > 0: Verwende sched_yield, <= 0: sleep

void *lock_forward(void *arg);
void *lock_backward(void *arg);

int main(int argc, char *argv[]) {
    pthread_t f, b;

    if (argc > 1) {
        backoff = atoi(argv[1]);
    }
    if (argc > 2) {
        yield_flag = atoi(argv[2]);
    }

    pthread_create(&f, NULL, lock_forward, NULL);
    pthread_create(&b, NULL, lock_backward, NULL);

    pthread_exit(NULL); // Die beiden anderen Threads laufen weiter
}

void *lock_forward(void *arg) {
    int iterate, i, status;

    for (iterate = 0; iterate < 10; iterate++) {
        for (i = 0; i < 3; i++) {
            if (i == 0 || !backoff) {
                status = pthread_mutex_lock(&mutex[i]);
            } else {
                status = pthread_mutex_trylock(&mutex[i]);
            }

            if (status == EBUSY) {
                for (--i; i >= 0; i--) {
                    pthread_mutex_unlock(&mutex[i]);
                }
            } else {
                // printf("forward locker got mutex %d\n", i);
            }

            if (yield_flag) {
                if (yield_flag > 0) {
                    sched_yield();
                } else {
                    sleep(1);
                }
            }
        }

        for (i = 2; i >= 0; i--) {
            pthread_mutex_unlock(&mutex[i]);
        }

        sched_yield(); // Neuer Versuch mit evtl. anderer Reihenfolge
    }

    return NULL;
}

void *lock_backward(void *arg) {
    int iterate, i, status;

    for (iterate = 0; iterate < 10; iterate++) {
        for (i = 2; i >= 0; i--) {
            if (i == 2 || !backoff) {
                status = pthread_mutex_lock(&mutex[i]);
            } else {
                status = pthread_mutex_trylock(&mutex[i]);
            }

            if (status == EBUSY) {
                for (++i; i < 3; i++) {
                    pthread_mutex_unlock(&mutex[i]);
                }
            } else {
                // printf("backward locker got mutex %d\n", i);
            }

            if (yield_flag) {
                if (yield_flag > 0) {
                    sched_yield();
                } else {
                    sleep(1);
                }
            }
        }

        for (i = 0; i < 3; i++) {
            pthread_mutex_unlock(&mutex[i]);
        }

        sched_yield();
    }

    return NULL;
}