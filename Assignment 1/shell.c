/*
    Name: Wang Riwu
    Matric: A0135766W

    Name: Jeremy Heng Wen Ming
    Matric: A0146789H
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CHAR 1024

char * getInput() {
    printf("> ");
    static char input[MAX_CHAR] = {0};
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';
    return input;
}

char ** split(char *input) {
    static char *argv[MAX_CHAR] = {0};
    char *str;
    int i;
    for (i = 0, str = input; i < MAX_CHAR; i++, str = NULL) {
        char *last;
        char *token = strtok_r(str, " ", &last);
        argv[i] = token;
        if (token == NULL) break;
    }
    return argv;
}

void execute(char *input) {
    pid_t childPID = fork();
    if (childPID > 0) {
        printf("Loading new process with id %d\n", childPID);
        int status;
        wait(&status);
        printf("\n");
    } else if (childPID == 0) {
        printf("> Parent pid: %d\n", getppid());

        char **argv = split(input);
        if (execvp(*argv, argv) < 0) {
            printf("Shell error: : %s\n", strerror(errno));
            exit(1);
        }
    } else {
        printf("Fork failed!");
        exit(1);
    }
}

int main() {
    while (1) {
        char *input = getInput();
        execute(input);
    }
}

