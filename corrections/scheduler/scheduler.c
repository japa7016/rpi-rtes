#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <syslog.h>

#define NSEC_PER_SEC       1000000000L
#define TICK_NS            10000000L    // 10 ms tick
#define F10_TICKS          2            // f10 → every 20 ms
#define F20_TICKS          5            // f20 → every 50 ms
#define TEST_DURATION_S    10
#define MAX_TICKS          ((TEST_DURATION_S * NSEC_PER_SEC) / TICK_NS)

#define INIT_ITERS_10MS    170000UL     // initial guess
#define INIT_ITERS_20MS    340000UL

sem_t           semF10, semF20;
volatile sig_atomic_t abortTest = 0;
struct timespec global_start;
unsigned long   fib_iters_10, fib_iters_20;

static inline long ts_to_us(const struct timespec *ts) {
    return ts->tv_sec * 1000000L + ts->tv_nsec / 1000L;
}

static void log_event(const char *who, const char *evt, unsigned long cnt) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long rel = ts_to_us(&now) - ts_to_us(&global_start);
    if (cnt)
        syslog(LOG_INFO, "[%s] %-10s #%3lu @ %7ldµs", who, evt, cnt, rel);
    else
        syslog(LOG_INFO, "[%s] %-10s      @ %7ldµs", who, evt,     rel);
}

void signal_handler(int sig) {
    (void)sig;
    abortTest = 1;
}

static void perform_load(unsigned long iters) {
    volatile unsigned long a = 0, b = 1, c;
    for (unsigned long i = 0; i < iters; i++) {
        c = a + b; a = b; b = c;
    }
}

static unsigned long calibrate(unsigned long guess, long target_us) {
    unsigned long iters = guess;
    for (int pass = 0; pass < 3; pass++) {
        struct timespec s, e;
        clock_gettime(CLOCK_MONOTONIC, &s);
        perform_load(iters);
        clock_gettime(CLOCK_MONOTONIC, &e);
        long elapsed = (e.tv_sec  - s.tv_sec ) * 1000000L
                     + (e.tv_nsec - s.tv_nsec) / 1000L;
        iters = (unsigned long)(iters * (double)target_us / (double)elapsed);
    }
    return iters;
}

void *fib10(void *_) {
    unsigned long run = 0;
    while (1) {
        sem_wait(&semF10);
        if (abortTest) break;
        run++;
        log_event("fib10", "start", run);
        perform_load(fib_iters_10);
        log_event("fib10", "end",   run);
    }
    return NULL;
}

void *fib20(void *_) {
    unsigned long run = 0;
    while (1) {
        sem_wait(&semF20);
        if (abortTest) break;
        run++;
        log_event("fib20", "start", run);
        perform_load(fib_iters_20);
        log_event("fib20", "end",   run);
    }
    return NULL;
}

void *sequencer(void *_) {
    struct timespec next;
    unsigned long tick = 0;
    clock_gettime(CLOCK_MONOTONIC, &next);
    clock_gettime(CLOCK_MONOTONIC, &global_start);
    log_event("seq", "started", 0);

    sem_post(&semF10); log_event("seq", "postF10", 1);
    sem_post(&semF20); log_event("seq", "postF20", 1);

    while (!abortTest && tick < MAX_TICKS) {
        tick++;
        next.tv_nsec += TICK_NS;
        if (next.tv_nsec >= NSEC_PER_SEC) {
            next.tv_sec++;
            next.tv_nsec -= NSEC_PER_SEC;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

        if (tick % F10_TICKS == 0) {
            sem_post(&semF10);
            log_event("seq", "postF10", tick / F10_TICKS + 1);
        }
        if (tick % F20_TICKS == 0) {
            sem_post(&semF20);
            log_event("seq", "postF20", tick / F20_TICKS + 1);
        }
    }

    abortTest = 1;
    sem_post(&semF10);
    sem_post(&semF20);
    log_event("seq", "exiting", 0);
    return NULL;
}

int main(void) {
    openlog("schedular", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Calibrating...");
    fib_iters_10 = calibrate(INIT_ITERS_10MS, 10000);
    syslog(LOG_INFO, "fib10 iters = %lu", fib_iters_10);
    fib_iters_20 = calibrate(INIT_ITERS_20MS, 20000);
    syslog(LOG_INFO, "fib20 iters = %lu", fib_iters_20);
    syslog(LOG_INFO, "Starting scheduler");

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    sem_init(&semF10, 0, 0);
    sem_init(&semF20, 0, 0);

    pthread_attr_t    attr;
    struct sched_param prio;
    pthread_t         t10, t20, tSeq;

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    prio.sched_priority = 40;
    pthread_attr_setschedparam(&attr, &prio);
    pthread_create(&t10,  &attr, fib10,     NULL);

    prio.sched_priority = 50;
    pthread_attr_setschedparam(&attr, &prio);
    pthread_create(&t20,  &attr, fib20,     NULL);

    prio.sched_priority = 60;
    pthread_attr_setschedparam(&attr, &prio);
    pthread_create(&tSeq, &attr, sequencer, NULL);

    pthread_join(tSeq, NULL);
    pthread_join(t10,  NULL);
    pthread_join(t20,  NULL);

    sem_destroy(&semF10);
    sem_destroy(&semF20);
    syslog(LOG_INFO, "Scheduler terminated");
    closelog();
    return 0;
}
