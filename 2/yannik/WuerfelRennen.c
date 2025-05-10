#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

bool is_blocked = false;
int thread_count = 0;
int winner = 0;
int three_counts[] = {0, 0, 0};

void *thread(void *) {
    pthread_mutex_lock(&mutex);
    int ID = ++thread_count;
    pthread_mutex_unlock(&mutex);

    int number;
    while (winner == 0) {
        number = rand() % 6 + 1;

        if (number == 3) {
            ++three_counts[ID - 1];

            pthread_mutex_lock(&mutex);
            if (three_counts[ID - 1] == 3 && winner == 0) {
                winner = ID;
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
                break;
            }
            pthread_mutex_unlock(&mutex);
        } else
            three_counts[ID - 1] = 0;

        if (number != 1 && number != 6)
            continue;

        pthread_mutex_lock(&mutex);
        if (winner != 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        if (number == 1 && !is_blocked) {
            is_blocked = true;
            pthread_cond_wait(&cond, &mutex);
            is_blocked = false;

        } else if ((number == 1 || number == 6) && is_blocked)
            pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[3];

    for (int i = 0; i < 3; ++i) {
        int result = pthread_create(&threads[i], NULL, thread, NULL);
        if (result != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < 3; ++i) {
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "Error joinging thread\n");
            exit(1);
        }
    }

    printf("Winner: Thread %d\n", winner);

    return 0;
}