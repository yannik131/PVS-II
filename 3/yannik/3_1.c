#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RECURSIVE

/**
 * @note 3.1 a)
 * Gemaess der manpage von pthread_mutex_lock gilt: "If the mutex type is PTHREAD_MUTEX_DEFAULT,
 * attempting to recursively lock the mutex results in undefined behavior."
 * Undefiniertes Verhalten kann hier ein Programmabsturz oder Entstehen eines Zombie-Threads sein.
 *
 * 3.1 b) #define RECURSIVE added
 *
 * 3.1 c) Fuer 1 Thread sucht das gegebene Programm in einem Thread nach dem key 0, den es nirgends
 * gibt. tree_lock wird intern einen Zaehler verwenden, der fuer jedes lock/unlock
 * inkrementiert/dekrementiert wird. Ist der Zaehler 0, darf wieder ein anderer Thread locken.
 * Ich habe eine Ausgabe generieren lassen, wo die Einrueckung den aktuellen Zaehlerstand
 * visualisiert:

  Examining node with key = 3 and value = 1
  Examining left child
    Examining node with key = 2 and value = 2
    Exiting search_tree
  Examining right child
    Examining node with key = 1 and value = 3
    Exiting search_tree
  Exiting search_tree

 * 3.1 d) Wenn jeder Thread beim Schreiben/Lesen den gesamten Baum sperrt, wird der Baum praktisch
 * sequentiell abgearbeitet. Das waere aber nicht notwendig, wenn
 * bspw. n Threads nur lesend zugreifen wollen und kein Schreibvorgang stattfinden soll.
 * Vernachlaessigbar waere das, wenn der Baum nur wenige Elemente hat und gut balanciert ist oder
 * wenn es nur wenige Threads gibt, die auf ihn zugreifen muessen.
 */

typedef struct node_ {
    int key;
    int value;
    struct node_ *left, *right;
} node_t;

pthread_mutex_t tree_lock;

typedef struct {
    int me;
    int key;
    node_t *node;
} thread_arg_t;

int lock_count = 0;

void pad(const char *format, ...) {
    for (int i = 0; i < lock_count; ++i)
        printf("  ");

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void lock() {
    pthread_mutex_lock(&tree_lock);
    ++lock_count;
}

void unlock() {
    pthread_mutex_unlock(&tree_lock);
    --lock_count;
}

int search_tree(node_t *node, int key) {
    lock();
    pad("Examining node with key = %d and value = %d\n", node->key, node->value);

    if (node->key == key) {
        /* solution is found here */
        printf("key = %d, value = %d\n", key, node->value);
        unlock();
        return 1;
    }

    if (node->left != NULL) {
        pad("Examining left child\n");
        if (search_tree(node->left, key)) {
            unlock();
            return 1;
        }
    }

    if (node->right != NULL) {
        pad("Examining right child\n");
        if (search_tree(node->right, key)) {
            unlock();
            return 1;
        }
    }

    pad("Exiting search_tree\n");

    unlock();
    return 0;
}

void *search_thread_func(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;

    if (!search_tree(targ->node, targ->key))
        printf("Thread %d: Search unsuccessfull\n", targ->me);

    return NULL;
}

int main(int argc, char *argv[]) {
    node_t A, B, C;
    pthread_mutexattr_t attr;
    char *omp_num_threads_as_char;
    int n_threads = 1;
    pthread_t *thread;
    thread_arg_t *arg;
    int i;

    omp_num_threads_as_char = getenv("OMP_NUM_THREADS");

    if (omp_num_threads_as_char != NULL)
        if (strlen(omp_num_threads_as_char) > 0)
            n_threads = atoi(omp_num_threads_as_char);

    thread = (pthread_t *)malloc(n_threads * sizeof(pthread_t));
    arg = (thread_arg_t *)malloc(n_threads * sizeof(thread_arg_t));

    pthread_mutexattr_init(&attr);

#ifdef RECURSIVE
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif

    pthread_mutex_init(&tree_lock, &attr);

    A.key = 3;
    A.value = 1;
    B.key = 2;
    B.value = 2;
    C.key = 1;
    C.value = 3;

    A.left = &B;
    A.right = &C;
    B.left = NULL;
    B.right = NULL;
    C.left = NULL;
    C.right = NULL;

    printf("Starting %d threads...\n", n_threads);

    for (i = 0; i < n_threads; i++) {
        arg[i].me = i;
        arg[i].key = i;
        arg[i].node = &A;
        pthread_create(&thread[i], NULL, search_thread_func, &arg[i]);
    }

    printf("Joining threads...\n");

    for (i = 0; i < n_threads; i++)
        pthread_join(thread[i], NULL);

    pthread_mutex_destroy(&tree_lock);
    pthread_mutexattr_destroy(&attr);

    free(thread);
    free(arg);

    return EXIT_SUCCESS;
}