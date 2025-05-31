#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * 3.2 Logs:
Using the version from the lecture
Started read thread with ID 1
Thread with ID 1 acquired read access
Started read thread with ID 2
Thread with ID 2 acquired read access
Thread with ID 2 acquired read access
Thread with ID 1 acquired read access
Thread with ID 2 acquired read access
Thread with ID 1 acquired read access
Thread with ID 2 acquired read access
Thread with ID 1 acquired read access
Thread with ID 2 acquired read access
Thread with ID 1 acquired read access
Write thread acquired write access

Wie zu erwarten kommt der Schreibthread hier erst zum Ende dran

Using the modified version
Started read thread with ID 1
Thread with ID 1 acquired read access
Started read thread with ID 2
Thread with ID 2 acquired read access
Thread with ID 2 acquired read access
Thread with ID 1 acquired read access
Thread with ID 2 acquired read access
Thread with ID 1 acquired read access
Thread with ID 1 acquired read access
Thread with ID 1 acquired read access
Write thread acquired write access
Thread with ID 2 acquired read access
Thread with ID 2 acquired read access

Da in der modifizierten Variante die Lesethreads auch mal warten, kommt hier der Schreibthread etwas
früher dran.
 */

typedef struct {
    bool modified;
    bool provocative;
} read_options_t;

typedef struct _rw_lock_t {
    pthread_mutex_t m;
    pthread_cond_t c;
    int num_r, num_w;
    int num_wr; // write requests
} rw_lock_t;

rw_lock_t lock;

int rw_lock_init(rw_lock_t *rwl) {
    rwl->num_r = 0;
    rwl->num_w = 0;
    rwl->num_wr = 0;
    pthread_mutex_init(&rwl->m, NULL);
    pthread_cond_init(&rwl->c, NULL);
    return 0;
}

int rw_lock_rlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);

    while (rwl->num_w > 0)
        pthread_cond_wait(&rwl->c, &rwl->m);

    rwl->num_r++;

    pthread_mutex_unlock(&rwl->m);
    return 0;
}

int rw_lock_rlock_modified(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);

    while (rwl->num_w > 0 || (rwl->num_wr > 0 && rwl->num_r > 0))
        pthread_cond_wait(&rwl->c, &rwl->m);

    rwl->num_r++;

    pthread_mutex_unlock(&rwl->m);
    return 0;
}

int rw_lock_wlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);

    rwl->num_wr++;
    while (rwl->num_w > 0 || rwl->num_r > 0)
        pthread_cond_wait(&rwl->c, &rwl->m);
    rwl->num_wr--;

    rwl->num_w = 1;

    pthread_mutex_unlock(&rwl->m);
    return 0;
}

int rw_lock_runlock(rw_lock_t *rwl) {
    pthread_mutex_lock(&rwl->m);
    rwl->num_r--;

    if (rwl->num_r == 0)
        pthread_cond_signal(&rwl->c);

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

void *read_thread(void *arg) {
    pthread_t threadID = pthread_self();
    read_options_t read_options = *(read_options_t *)(arg);
    printf("Started read thread with ID %lu\n", (unsigned long)(threadID));

    for (int i = 0; i < 5; ++i) {
        if (read_options.modified)
            rw_lock_rlock_modified(&lock);
        else
            rw_lock_rlock(&lock);

        printf("Thread with ID %lu acquired read access\n", (unsigned long)threadID);

        if (read_options.provocative && i == 0)
            sleep(1);
        else
            sleep(2);

        rw_lock_runlock(&lock);
    }

    return NULL;
}

void *write_thread(void *) {
    sleep(5);
    rw_lock_wlock(&lock);
    printf("Write thread acquired write access\n");
    rw_lock_wunlock(&lock);

    return NULL;
}

int main(int argc, char **argv) {
    rw_lock_init(&lock);
    bool modified = false;

    if (argc == 2 && strcmp(argv[1], "-modified") == 0)
        modified = true;

    if (modified)
        printf("Using the modified version\n");
    else
        printf("Using the version from the lecture\n");

    pthread_t threads[3];
    read_options_t read_options[2];
    for (int i = 0; i < 2; ++i) {
        read_options[i].modified = modified;
        read_options[i].provocative = (i == 1);
    }

    for (int i = 0; i < 3; ++i) {
        int result;

        if (i == 0)
            result = pthread_create(&threads[i], NULL, read_thread, &read_options[i]);
        else if (i == 1)
            result = pthread_create(&threads[i], NULL, read_thread, &read_options[i]);
        else
            result = pthread_create(&threads[i], NULL, write_thread, NULL);

        if (result != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < 3; ++i) {
        int result = pthread_join(threads[i], NULL);

        if (result != 0) {
            fprintf(stderr, "Error joining thread\n");
            exit(1);
        }
    }
}