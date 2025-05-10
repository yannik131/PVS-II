#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

void split_strings(char* const string, char*** strings, unsigned int* n);
void* thread_check_palindrome(void* _string);
bool check_palindrome(const char* const string);

int main(int argc, char** argv) {
    const unsigned int n = 10000;
    char* string = malloc(sizeof(char) * n);

    printf("Please give the (alphanumeric) strings to be checked to be palindromes (total max length %u). Each string has to be ended by '|'. Example: a|b|c|.:\n", n);
    scanf("%s", string);

    char** strings = NULL;
    unsigned int num_strings = 0;
    split_strings(string, &strings, &num_strings);

    printf("%u strings have been found.\n", num_strings);
    printf("The given strings are:\n");
    for (int i = 0; i < num_strings; i++) {
        printf("%s\n", strings[i]);
    }
    printf("\n");

    pthread_t* const pid_list = malloc(sizeof(pthread_t) * num_strings);
    for (int i = 0; i < num_strings; i++) {
        pthread_create(&pid_list[i], NULL, thread_check_palindrome, strings[i]);
    }

    for (int i = 0; i < num_strings; i++) {
        pthread_join(pid_list[i], NULL);
    }

    free(pid_list);
    free(string);
}

// splits string into multiple strings (on spaces)
// string may not be used afterwards
// each element of strings is a string, n is the number of strings
void split_strings(char* const _string, char*** strings, unsigned int* n) {
    *n = 0;
    unsigned int max_strings = 8; // current max number of strings
    unsigned int strings_index = 0; // current index of strings
    (*strings) = (char**) calloc(max_strings, sizeof(char*));
    char* string = _string;
    char* start = string; // start of the current string
    
    while (*string != '\0') {
        if (*string == '|') {
            // a string has ended
            *string = '\0';

            // point the start of the current strings element to the start of the string
            (*strings)[strings_index] = start;
            strings_index++;
            (*n)++;
            // then move on to the next string
            string++;
            start = string;

            if (strings_index == max_strings) {
                // since the amount of strings is unknown, we may have to extend strings
                max_strings *= 2;
                (*strings) = realloc((*strings), max_strings * sizeof(char*));
            }
        }
        else {
            string++;
        }
    }
}

// pass to pthread_create to create a thread to check the given _string (has to be char*)
void* thread_check_palindrome(void* _string) {
    char* string = (char*) _string;
    printf("Thread %ld checking string: '%s'\n\tThe string is %s palindrome.\n\n", pthread_self(), string, check_palindrome(string) ? "a" : "NOT a");
    return NULL;
}

bool check_palindrome(const char* const string) {
    const char* start = string;
    const char* end = start;
    while (*end != '\0') {
        end++;
    }
    
    if (end == start) {
        // we consider the empty string a palindrome
        return true;
    }
    else {
        // jump back to the last not \0 char
        end--;
    }

    while (*end == *start) {
        if (end == start) {
            return true;
        }
        end--;
        if (end == start) {
            return true;
        }
        start++;
    }

    return false;
}