#include <stdio.h>
#include <pthread.h>

// Global variable.
int ctr=0;
pthread_t thread[10];

void *child(void *t)
{
    // Print out the parameter passed in, and the current value of ctr.
    printf("I am child %d. Ctr=%d\n", t, ctr);
    // Then increment ctr
    ctr++;
    pthread_exit(NULL);
}

int main()
{
    int i;

    // Initialize ctr
    ctr=0;

    // Create the threads
    for(i=0; i<10; i++) {
        pthread_create(&thread[i], NULL, child, (void *) i);
        pthread_join(thread[i], NULL);
    }

    // And print out ctr
    printf("Value of ctr=%d\n", ctr);
    return 0;
}
