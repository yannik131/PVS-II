/*
    a)
    Ein Deadlock (in Bezug auf Threads) ist das Stillstehen mehrerer Thread,
    weil jeder Thread auf Ressourcen wartet, die von einerm anderen Thread bessessen werden.

    Bedingungen für einen Deadlock:
        * Die Ressourcen können nur von einem Thread gleichzeitig verwendet werden
        * Jeder Thread hat (mind.) eine Ressource und wartet auf eine weitere Ressource
        * Es gibt einen Zyklus in den Threads, die auf Ressourcen warten
        * Die Threads müssen die Ressourcen freiwillig zurückgeben

    b)
    Das modifizierte Programm kommt in einem Deadlock,
    weil (angenommen der forward locker (FL) ist zuerst an der yield flag) der Programmablauf folgendermaßen ist:
        FL lockt mutex[0] erreicht die yieldflag und wartet auf ein Signal
        BL lockt mutex[2] erreicht die yieldflag, weckt FL auf und wartet auf ein Signal
        FL lockt mutex[1] erreicht die yieldflag, weckt BL auf und wartet auf ein Signal
        BL versucht mutex[1] zu locken, der ist aber bereits von FL gesperrt, gibt also alle Mutexe frei, erreicht die yieldflag, weckt FL und wartet auf ein Signal
        FL lock mutex[2] erreicht die yieldflag, weckt BL auf und wartet auf ein Signal
        BL versucht mutex[2] zu locken, scheitert, und wartet darauf, dass mutex[2] frei wird
        => Beide Threads warten aufeinander
        => Deadlock

    Verwendet man statt pthread_mutex_lock pthread_mutex_unlock für mutex[0] in FL und mutex[2] in BL, dann läuft das Programm bis entweder FL oder BL fertig sind.
    Der andere Thread bleibt dann hängen, weil er kein Signal mehr bekommen kann.
*/

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

// condition to replace sched_yield()
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// we need a mutex for the cond
pthread_mutex_t turn = PTHREAD_MUTEX_INITIALIZER;

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
            else
                status = pthread_mutex_trylock(&mutex[i]);
            
            if (status == EBUSY)
                for (--i; i >= 0; i--) pthread_mutex_unlock(&mutex[i]);
            else printf("forward locker got mutex %d\n", i);

            if (yield_flag) {
                if (yield_flag > 0) {
                    pthread_mutex_lock(&turn);
                    pthread_cond_signal(&cond);
                    pthread_cond_wait(&cond, &turn);
                    pthread_mutex_unlock(&turn);
                }
                else sleep(1);
            }
        }

        for (i = 2; i >= 0; i--)
            pthread_mutex_unlock(&mutex[i]);
    }

    return NULL;
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
            
            if (yield_flag) {
                if (yield_flag > 0) {
                    pthread_mutex_lock(&turn);
                    pthread_cond_signal(&cond);
                    pthread_cond_wait(&cond, &turn);
                    pthread_mutex_unlock(&turn);
                }
                else sleep(1);
            }
        }

        for (i = 0; i < 3; i++)
            pthread_mutex_unlock(&mutex[i]);
    }

    return NULL;
}