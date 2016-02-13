/*
    Name: Wang Riwu
    Matric: A0135766W

    Name: Jeremy Heng Wen Ming
    Matric: A0146789H
*/

#include <stdio.h>
#include <string.h>

/* The following implementation of the string length function resolves the
   length of a string in an array at compile time. It differs from libc's strlen
   by including the terminating null byte. This eases our calculations when
   deciding buffer size allocations */
#define STRLEN(st) (sizeof(st)/sizeof(st[0]))

int main(int argc, char *argv[], char *envp[]) {
    const char envVariable[] = "SHELL_PATH";
    char *value = "UNKNOWN";

    /* Allocate a buffer on the stack to store our concatenated final string.
       We initialise it with null bytes to prevent information leakage
       vulnerabilities. */
    char substr[STRLEN(envVariable) + 1] = {0};

    /* We use the 'n' versions of the following string functions to
       explicitly avoid overflows due to bad string management. */
    strncpy(substr, envVariable, STRLEN(envVariable));
    strncat(substr, "=", 1); //ensures exact match

    while(*envp) {
        char *address = strstr(*envp, substr);
        if (address && (address - *envp == 0)) { // ensures that it starts with
             value = address + strlen(substr);   // envVariable
        }
        printf("%s\n", *envp++);
    }
    printf("\n%s value is %s\n", envVariable, value);
}
