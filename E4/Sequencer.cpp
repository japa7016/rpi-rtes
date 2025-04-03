/*
 * This is a C++ version of the canonical pthread service example. It intends
 * to abstract the service management functionality and sequencing for ease
 * of use. Much of the code is left to be implemented by the student.
 *
 * Build with g++ --std=c++23 -Wall -Werror -pedantic
 * Steve Rizor 3/16/2025
 */
#include <cstdint>
#include <cstdio>
#include "Sequencer.hpp"
 
void runFib10(void)
{
    static uint32_t fib10Cnt = 0;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    //printf("fib10 started execution at %ld.%09ld seconds\n", ts.tv_sec, ts.tv_nsec);

    for(int idx = 0; idx < 170000; idx++)
    {
        int fib0 = 0, fib1 = 1, fib = 0;
        for(int jdx = 0; jdx < 10; jdx++)
        {
            fib0 = fib1;
            fib1 = fib;
            fib = fib0 + fib1;
        }
    }

    fib10Cnt++;

    clock_gettime(CLOCK_REALTIME, &ts);
    //printf("fib10 completed execution at %ld.%09ld seconds\n", ts.tv_sec, ts.tv_nsec);
    //printf("fib10 executed %d times\n", fib10Cnt);
}

void runFib20(void)
{
     static uint32_t fib20Cnt = 0;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    //printf("fib20 started execution at %ld.%09ld seconds\n", ts.tv_sec, ts.tv_nsec);

    for(int idx = 0; idx < 340000; idx++)
    {
        int fib0 = 0, fib1 = 1, fib = 0;
        for(int jdx = 0; jdx < 20; jdx++)
        {
            fib0 = fib1;
            fib1 = fib;
            fib = fib0 + fib1;
        }
    }

    fib20Cnt++;

    clock_gettime(CLOCK_REALTIME, &ts);
    //printf("fib20 completed execution at %ld.%09ld seconds\n", ts.tv_sec, ts.tv_nsec);
    //printf("fib20 executed %d times\n", fib20Cnt);
}

int main()
{
    // Example use of the sequencer/service classes:
    Sequencer sequencer{};

    sequencer.addService(runFib10, 1, 99, 20);
    sequencer.addService(runFib20, 1, 98, 50);

    sequencer.startServices();
    // todo: wait for ctrl-c or some other terminating condition
     std::this_thread::sleep_for(std::chrono::seconds(5));  // run for 5 seconds
    sequencer.stopServices();
}
