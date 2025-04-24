#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>
#define COUNT  10

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[2];
threadParams_t threadParams[2];

sem_t sem;

// Unsafe global
int gsum=0;

void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
    sem_post(&sem);
}


void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    sem_wait(&sem);
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}




int main (int argc, char *argv[])
{
   int rc;

   int i=0;

   sem_init(&sem,0,0);
   threadParams[i].threadIdx=i;
   pthread_create(&threads[i],   // pointer to thread descriptor
                  (void *)0,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   threadParams[i].threadIdx=i;
   pthread_create(&threads[i], (void *)0, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
     pthread_join(threads[i], NULL);
   sem_destroy(&sem);
   printf("TEST COMPLETE\n");
}
