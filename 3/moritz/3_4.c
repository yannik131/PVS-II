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