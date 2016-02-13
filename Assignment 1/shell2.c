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
#include <limits.h>

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
            printf("Shell error: : No such file or directory\n");
            exit(1);
        }
    } else {
        printf("Fork failed!");
        exit(1);
    }
}

void setShellPath(char *shellPath) {
    const char *varname = "SHELL_PATH";
    char resolvedPath[PATH_MAX + 1] = {0};
    // We use the multiple methods to get the absolute path to the currently
    // executing program:
    // 1. /proc/self/exe - Works on Linux systems
    // 2. argv[0] - Not fool proof but the assignment spec allows this
    if (realpath("/proc/self/exe", resolvedPath) == 0) {
        if (realpath(shellPath, resolvedPath) == 0) {
            printf("Failed to resolve real path of the shell.");
            exit(1);
        }
    }
    if (setenv(varname, resolvedPath, 1) != 0) {
        printf("Failed to set environment variable.");
        exit(1);
    }
}

int main(int argc, char ** argv) {
    setShellPath(argv[0]);
    while (1) {
        char *input = getInput();
        execute(input);
    }
}

