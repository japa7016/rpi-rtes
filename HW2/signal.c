#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#define SEMAPHORE "/BIN_SEM"

int main(void)
{
	openlog("Signal", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
	sem_t *sem = sem_open(SEMAPHORE, 0);
	if(sem == SEM_FAILED)
	{
        	syslog(LOG_ERR, "sem_open failed: %s", strerror(errno));
        	closelog();
        	return 1;
	}

	printf("Signaling semaphore..\n");

	sem_post(sem);

   	sem_close(sem);
    	syslog(LOG_INFO, "Done. Exiting.");
    	closelog();

    	return 0;
}
