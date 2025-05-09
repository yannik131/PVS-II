// Compiler = Linux (Suse) gcc-Compiler
// compile with: gcc scalarproduct.c -lpthread -Wall -o scalarproduct
// execute with: ./scalarproduct

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct {
    int n;
    double *a;
    double *b;
    double *sum;
    pthread_mutex_t *mutex;
} work;

/*
 * Calculate scalarproduct of two double arrays a and b
 * with bad synchronization of 2 threads.
 */
void* scalarproduct_naive(void *arg) {
    work *par = (work*) arg;
    int n = par->n;
    double *a = par->a;
    double *b = par->b;
    for (int i = 0; i < n; i++) {
        double local_sum = a[i] * b[i];
        pthread_mutex_lock(par->mutex);
        *(par->sum) += local_sum;
        pthread_mutex_unlock(par->mutex);
    }
    return NULL;
}

/*
 * Calculate scalarproduct of two double arrays a and b
 * with better synchronization of 2 threads.
 */
void* scalarproduct_advanced(void *arg) {
    work *par = (work*) arg;
    int n = par->n;
    double *a = par->a;
    double *b = par->b;
    double local_sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        local_sum += a[i] * b[i];
    }
    
    pthread_mutex_lock(par->mutex);
    *(par->sum) += local_sum;
    pthread_mutex_unlock(par->mutex);
    return NULL;
}

int main(int argc, char **argv) {
    int n = 1048576; // == 2^20
    int num_threads = 2;
    
    //think up a and b
    srand(time(NULL));
    double *a = (double*) malloc(n * sizeof(double));
    double *b = (double*) malloc(n * sizeof(double));
    
    //fill a and b with random numbers between -1.0 and 1.0
    for (int i = 0; i < n; i++) {
        a[i] = (double) (2.0 * rand() / RAND_MAX) - 1.0;
        b[i] = (double) (2.0 * rand() / RAND_MAX) - 1.0;
    }
    
    //create work
    pthread_t* threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    work *all_work = (work*) malloc(num_threads * sizeof(work));
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    
    double sum = 0.0;
    int n_per_thread = n / num_threads;
    int remainder = n % num_threads;
    
    //fill work
    int start_n = 0;
    for (int i = 0; i < num_threads; i++) {
        //balance elements n per thread
        int local_n;
        if (i < remainder) {
            local_n = n_per_thread + 1;
        } else {
            local_n = n_per_thread;
        }
        all_work[i].n = local_n;
        all_work[i].a = a + start_n;
        all_work[i].b = b + start_n;
        start_n += local_n;
        all_work[i].mutex = &mutex;
        all_work[i].sum = &sum;
    }
    
    
    //timer
    struct timeval tvalBefore, tvalAfter;
    
    //start timer
    gettimeofday (&tvalBefore, NULL);

    //start threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, scalarproduct_naive, &all_work[i]);
    }

    //join threads
    for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
    }

    //end timer
    gettimeofday (&tvalAfter, NULL);

    
    //print info
    printf("Schlecht synchronisierte Methode:\n- Ergebnis Skalarprodukt: %f\n", sum);
    
    //print timer info
    printf("- Dauer in Millisekunden: %ld ms\n",
        (((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec)/1000
    );
    
    sum = 0.0;
    //start timer
    gettimeofday (&tvalBefore, NULL);
    
    //start threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, scalarproduct_advanced, &all_work[i]);
    }
    
    //join threads
    for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
    }
    
    //end timer
    gettimeofday (&tvalAfter, NULL);

    //print info
    printf("Besser synchronisierte Methode:\n- Ergebnis Skalarprodukt: %f\n", sum);
    
    //print timer info
    printf("- Dauer in Millisekunden: %ld ms\n",
        (((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec)/1000
    );
    
    free(a);
    free(b);
    free(threads);
    free(all_work);
    
    return 0;
}
