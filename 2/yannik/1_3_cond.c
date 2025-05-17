/**
 * @note 2.3 a)
 * Vgl. Vorlesung Betriebssysteme, Coffmann (1971):
 * - Wechselseitiger Ausschluss (mutual exclusion): Jedes involvierte Betriebsmittel ist entweder
 * exklusiv belegt oder frei
 * - Zusaetzliche Belegung (Hold-and-wait): Die Prozesse haben bereits Betriebsmittel belegt, wollen
 * zusaetzliche Betriebsmittel belegen und warten darauf, dass sie frei werden.
 * - Keine vorzeitige Rueckgabe (No preemption): Bereits belegte Betriebsmittel koennen den
 * Prozessen nicht einfach wieder entzogen werden
 * - Gegenseitiges Warten (Circulat wait): Es existiert ein Zyklus von zwei oder mehr Prozessen, bei
 * denen jeweils einer die Betriebsmittel vom naechsten belegen will, die dieser belegt hat
 */

/*
    b)
    Das modifizierte Programm kommt in einem Deadlock,
    weil (angenommen der forward locker (FL) ist zuerst an der yield flag) der Programmablauf
   folgendermassen ist: FL lockt mutex[0] erreicht die yieldflag und wartet auf ein Signal BL lockt
   mutex[2] erreicht die yieldflag, weckt FL auf und wartet auf ein Signal FL lockt mutex[1]
   erreicht die yieldflag, weckt BL auf und wartet auf ein Signal BL versucht mutex[1] zu locken,
   der ist aber bereits von FL gesperrt, gibt also alle Mutexe frei, erreicht die yieldflag, weckt
   FL und wartet auf ein Signal FL lock mutex[2] erreicht die yieldflag, weckt BL auf und wartet auf
   ein Signal BL versucht mutex[2] zu locken, scheitert, und wartet darauf, dass mutex[2] frei wird
        => Beide Threads warten aufeinander
        => Deadlock

    Verwendet man statt pthread_mutex_lock pthread_mutex_unlock fuer mutex[0] in FL und mutex[2] in
   BL, dann laeuft das Programm bis entweder FL oder BL fertig sind. Der andere Thread bleibt dann
   haengen, weil er kein Signal mehr bekommen kann.
*/

#include <errno.h> // Fuer EBUSY
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Fuer sleep()

pthread_mutex_t mutex[3] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
                            PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t yield_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t yield_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void wait() {
    pthread_mutex_lock(&yield_mutex);
    pthread_cond_signal(&yield_cond);
    pthread_cond_wait(&yield_cond, &yield_mutex);
    pthread_mutex_unlock(&yield_mutex);
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
                printf("forward locker got mutex %d\n", i);
            }

            if (yield_flag) {
                if (yield_flag > 0) {
                    wait();
                } else {
                    sleep(1);
                }
            }
        }

        for (i = 2; i >= 0; i--) {
            pthread_mutex_unlock(&mutex[i]);
        }
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
                printf("backward locker got mutex %d\n", i);
            }

            if (yield_flag) {
                if (yield_flag > 0) {
                    wait();
                } else {
                    sleep(1);
                }
            }
        }

        for (i = 0; i < 3; i++) {
            pthread_mutex_unlock(&mutex[i]);
        }
    }

    return NULL;
}