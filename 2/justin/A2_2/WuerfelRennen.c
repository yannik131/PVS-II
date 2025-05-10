#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

void* play(void* id);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int sieger = -1;

int main(int argc, char** argv) {
    const int num_threads = 3;
    pthread_t threads[num_threads];
    int ids[num_threads];
    for (int i = 0; i < num_threads; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, play, &ids[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Main thread:\n\tThe winner is thread %d\n", sieger);
}

void* play(void* _id) {
    const int id = *((const int*) _id);

    // seed the rng with a different seed for every thread
    unsigned int seed = time(NULL) + id * 137;
    srandom(seed);

    int three_counter = 0;
    while (three_counter < 3) {
        int roll = random() % 6 + 1;
        if (roll == 3) {
            three_counter++;
        }
        else {
            three_counter = 0;
        }
        
        if (roll == 1) {
            pthread_mutex_lock(&mutex);
            // this check is needed, because the other threads may have finished after this one rolled a 1
            // then, without this check this thread would wait, while the others can no longer signal
            if (sieger != -1) {
                pthread_mutex_unlock(&mutex);
                break;
            }

            // signal a waiting thread (if any)
            pthread_cond_signal(&cond);

            // wait until another thread rolls a 1 or 6
            pthread_cond_wait(&cond, &mutex);

            // check if the game is over after being signalled
            if (sieger != -1) {
                pthread_mutex_unlock(&mutex);
                break;
            }

            pthread_mutex_unlock(&mutex);
        }
        else if (roll == 6) {
            // signal a waiting thread (if any)
            pthread_cond_signal(&cond);
        }
    }

    pthread_mutex_lock(&mutex);
    {
        // check if there already is a winner
        if (sieger == -1) {
            sieger = id;
        }

        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}