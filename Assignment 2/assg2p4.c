#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

int glob;

void *child(void *t)
{
    // Increment glob by 1, wait for 1 second, then increment by 1 again.
    printf("Child %d entering. Glob is currently %d\n", t, glob);
    pthread_mutex_lock(&mutex);
    glob++;
    sleep(1);
    glob++;
    pthread_mutex_unlock(&mutex);
    printf("Child %d exiting. Glob is currently %d\n", t, glob);
}

int main()
{
    int i;
    glob=0;

    pthread_t thread[10];
    for(i=0; i<10; i++)
        pthread_create(&thread[i], NULL, child, (void *) i);

    //wait for all threads to finish executing
    for(i=0; i<10; i++)
        pthread_join(thread[i], NULL);
    printf("Final value of glob is %d\n", glob);
    pthread_mutex_destroy(&mutex);
    return 0;
}
