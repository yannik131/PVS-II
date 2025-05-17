#include "util.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void test_palindrome() {
    // even
    const char *s1 = "otto";
    const char *s2 = "hallo";

    // odd
    const char *s3 = "oto";
    const char *s4 = "asd";

    assert(is_palindrome(s1));
    assert(!is_palindrome(s2));
    assert(is_palindrome(s3));
    assert(!is_palindrome(s4));
}

void *is_palindrome_func(void *arg) {
    const char *str = (const char *)arg;

    pthread_t threadID = pthread_self();
    printf("Thread with ID %lu is now working on string %s\n", (unsigned long)threadID, str);

    if (is_palindrome((const char *)str))
        printf("%s is a palindrome\n", str);
    else
        printf("%s is not a palindrome\n", str);

    return NULL;
}

int main(int argc, char **argv) {
    test_palindrome();

    if (argc < 2) {
        printf("Usage: %s <str1> <str2> ...\n", argv[0]);
        return 1;
    }

    int N = argc - 1;
    pthread_t *threads = (pthread_t *)malloc(N * sizeof(pthread_t));
    abort_on_failed_allocation(threads);

    for (int i = 0; i < N; ++i) {
        int result = pthread_create(&threads[i], NULL, is_palindrome_func, (void *)argv[i + 1]);

        if (result != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < N; ++i) {
        int result = pthread_join(threads[i], NULL);

        if (result != 0) {
            fprintf(stderr, "Error joining thread\n");
            exit(1);
        }
    }

    free(threads);

    printf("Main: Threads joined successfully.\n");

    return 0;
}