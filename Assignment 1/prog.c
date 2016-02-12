/*
    Name: Wang Riwu
    Matric: A0135766W

    Name: Jeremy Heng Wen Ming
    Matric: A0123456A
*/

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {
    const char *envVariable = "SHELL_PATH";
    char *value = "UNKNOWN";

    char substr[strlen(envVariable) + 1];
    strcpy(substr, envVariable);
    strcat(substr, "="); //ensures exact match

    while(*envp) {
        char *address = strstr(*envp, substr);
        if (address && (address - *envp == 0)) { //ensures that it starts with envVariable
            value = address + strlen(substr);
        }
        printf("%s\n", *envp++);
    }
    printf("\n%s value is %s\n", envVariable, value);
}
