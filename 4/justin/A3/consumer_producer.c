#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int special_number;
int found_flag = 0;

typedef struct{
    int time_to_sleep;
}t_arg;

typedef struct{
    int container[5];
    int size;
}queue_t;

queue_t queue = {{},0};

void * produce(void * p_arg);
void * consume(void * c_arg);
void offer(int item);
int poll();
void printQueueContent();

int main (int argc, char ** argv){
    if (argc != 2) {
        printf("USAGE:\n\t./%s <special number (0..21)>\n", argv[0]);
        exit(1);
    }

    special_number = atoi(argv[1]);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t consumer,producer;
    t_arg consumer_arg = {2};
    t_arg producer_arg = {1};

    printQueueContent();

    pthread_create(&consumer,NULL,consume,&consumer_arg);
    pthread_create(&producer,NULL,produce,&producer_arg);

    pthread_join(consumer,NULL);
    pthread_join(producer,NULL);
    return 0;
}

void * produce(void * p_arg){
    t_arg * arg = (t_arg*) p_arg;
    for(int i = 0; i <= 21; i++){
        //////////////////////////////
        //TO SYNCHRONIZE:
        pthread_mutex_lock(&mutex);
        while (queue.size == 5) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (found_flag) {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        offer(i);
        pthread_cond_signal(&cond);
        printQueueContent();
        pthread_mutex_unlock(&mutex);
        //////////////////////////////

        //sleep(arg->time_to_sleep);
    }
    printf("The Producer is finished producing!\n");
    return NULL;
}

void * consume(void * c_arg){
    t_arg * arg = (t_arg*) c_arg;
    int item;
    do{
        //////////////////////////////
        //TO SYNCHRONIZE:
        pthread_mutex_lock(&mutex);
        while (queue.size == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        item = poll();
        if (item == special_number) {
            printf("The special number has been found!\n");
            found_flag = 1;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_cond_signal(&cond);
        printQueueContent();
        pthread_mutex_unlock(&mutex);
        //////////////////////////////

        //sleep(arg->time_to_sleep);
    } while(item != 21);
    printf("The special number has NOT been found!\n");
    printf("The Consumer is finished consuming!\n");
    return NULL;	
}

/*
 * Offers an item to be inserted into the queue at it's end
*/
void offer(int item){
    queue.container[queue.size] = item;
    queue.size++;
}

/*
 * Removes and returns the first item of the queue
*/
int poll(){
    int return_val = queue.container[0];

    for(int i = 1; i < queue.size; i++)
        queue.container[i-1] = queue.container[i];

    queue.size--;
    return return_val;
}

/*
 * Utility print function
*/
void printQueueContent(){
    printf("Queue of size %d:\t", queue.size);

    for(int i = 0; i < queue.size-1; i++)
        printf("%d -- ", queue.container[i]);

    if(queue.size > 0)
        printf("%d",queue.container[queue.size-1]);

    printf("\n");
}

