#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define COUNT  10

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[2];
threadParams_t threadParams[2];


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
}


void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

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

   pthread_attr_t attr1,attr2;
   struct sched_param param1, param2;

   pthread_attr_init(&attr1);
   pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
   pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);

   pthread_attr_init(&attr2);
   pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
   pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);

   param1.sched_priority = 40;
   pthread_attr_setschedparam(&attr1, &param1);

   threadParams[i].threadIdx=i;
   pthread_create(&threads[i],   // pointer to thread descriptor
                  &attr1,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   param2.sched_priority = 50;
   pthread_attr_setschedparam(&attr2, &param2); 
   
   threadParams[i].threadIdx=i;
   pthread_create(&threads[i], &attr2, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
   pthread_join(threads[i], NULL);
   pthread_attr_destroy(&attr1);
   pthread_attr_destroy(&attr2);
   printf("TEST COMPLETE\n");
}
