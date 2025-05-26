/*
    a)
    Bei benachbarten Mutexen ist es möglich, dass diese auch im selben Cache-Block liegen.
    Dadurch kann es zu False sharing kommen, d.h. obwohl ein Wert nicht vom anderen thread (/Prozessor) geändert wurde,
    muss der (gemeinsame) Cache-Block neu geladen werden (da das System nur sieht, dass es in dem Block eine Änderung gab).

    Das tritt dementsprechend nicht auf, wenn auf zwei (im Array) benachbarte Mutexe zugegriffen wird,
    die aber trotzdem in unterschiedlichen Cache-Blöcken liegen.

    b)
    Die einzelnen Einträge können gepadded werden, d.h. es wird ein größer als nötiger Speicherplatz verwendet,
    wodurch nicht mehrere Einträge in den gleichen Cache-Block passen.

    c)
    Pthread-win32 (https://github.com/JoakimSoderberg/pthreads-win32/blob/master/pthread.h):
    struct pthread_mutex_t_
    {
        LONG lock_idx;
        int recursive_count;
        int kind;
        pthread_t ownerThread;
        HANDLE event;	
        ptw32_robust_node_t* robustNode;
    };
    (~36 Byte)

    Linux:
    typedef union
    {
        struct __pthread_mutex_s __data;
        char __size[__SIZEOF_PTHREAD_MUTEX_T];
        long int __align;
    } pthread_mutex_t;
    (sizeof(pthread_mutex_t) = 40 Byte)

    Pthread-win32 verwendet die Windows API für die Implementierung (-> HANDLE).
    Dadurch werden die Threads an Betriebssystem Threads gebunden, und liegen nicht im selben Cache-Block ( : unsure).
*/


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_THREADS 2
#define LOCK_COUNT (1024*1024*8)
#define MUTEX_COUNT 8

pthread_mutex_t mutexes[MUTEX_COUNT];

void *mutex_benchmark(void *argument)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)(argument);

    for(int i = 0; i < LOCK_COUNT; i++)
    {
        while(pthread_mutex_trylock(mutex) != 0);
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}
 
int main(void)
{    
    for(int i = 0; i < MUTEX_COUNT; i++) pthread_mutex_init(&mutexes[i], NULL);
    
    for(int mutex_id_1 = 0; mutex_id_1 < MUTEX_COUNT; mutex_id_1++)
        for(int mutex_id_2 = 0; mutex_id_2 < MUTEX_COUNT; mutex_id_2++)        
        {
            pthread_t thread_1, thread_2;
            struct timeval start, end;
            
            gettimeofday(&start, NULL); 
            
            pthread_create(&thread_1, NULL, &mutex_benchmark, &mutexes[mutex_id_1]);
            pthread_create(&thread_2, NULL, &mutex_benchmark, &mutexes[mutex_id_2]);
            
            pthread_join(thread_1, NULL);
            pthread_join(thread_2, NULL);      
            
            gettimeofday(&end, NULL);
            
            long secs_used =(end.tv_sec - start.tv_sec);    
            long micros_used = ((secs_used * 1000 * 1000) + end.tv_usec) - (start.tv_usec);
            double million_locks_per_second = ((double)LOCK_COUNT  / (double)micros_used);   
            
            printf("Performance of locking mutex[%d] and mutex[%d]:"
                "%f million locks per second\n",
                mutex_id_1, mutex_id_2, million_locks_per_second);
        }
        
    for(int i = 0; i < MUTEX_COUNT; i++) pthread_mutex_destroy(&mutexes[i]);
    
    return 0;
}
