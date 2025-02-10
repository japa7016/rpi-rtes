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
	openlog("Wait", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
	sem_t *sem = sem_open(SEMAPHORE,O_CREAT,0644,0);
	if(sem == SEM_FAILED)
	{
	        syslog(LOG_ERR, "sem_open failed: %s", strerror(errno));
        	closelog();
        	return 1;
	}
	syslog(LOG_INFO, "Waiting on semaphore...");
	sem_wait(sem);
	
	printf("Semaphore Signal received! Exitting..\n");
	sem_close(sem);
	sem_unlink(SEMAPHORE);
	closelog();
	return 0;

}
