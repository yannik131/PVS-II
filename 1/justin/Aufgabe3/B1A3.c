#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool check_palindrome(const char* const string);

int main(int argc, char** argv) {
    const unsigned int n = 1000;
    char* const string = malloc(sizeof(char) * n);

    printf("Please give the string to be checked to be a palindrome (max length %u):\n", n);
    scanf("%s", string);

    printf("The string is %s palindrome.\n", check_palindrome(string) ? "a" : "NOT a");

    free(string);
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