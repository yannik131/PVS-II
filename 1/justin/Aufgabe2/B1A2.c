#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const char USAGE[50] = "USAGE:\n./B1A2 <num threads>\n";

void* print_id(void* _);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("%s", USAGE);
        exit(1);
    }

    const unsigned int n = atoi(argv[1]);

    pthread_t* const id_list = malloc(sizeof(pthread_t) * n);
    for (int i = 0; i < n; i++) {
        pthread_create(&(id_list[i]), NULL, print_id, NULL);
    }

    print_id(NULL);

    for (int i = 0; i < n; i++) {
        pthread_join(id_list[i], NULL);
    }

    printf("Hallo von Main. Ich habe %u Threads erzeugt.\n", n);

    free(id_list);
}

void* print_id(void* _) {
    printf("Hallo ich bin Thread %ld\n", pthread_self());
    return NULL;
}