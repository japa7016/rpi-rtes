/*
 * This is a C++ version of the canonical pthread service example. It intends
 * to abstract the service management functionality and sequencing for ease
 * of use. Much of the code is left to be implemented by the student.
 *
 * Build with g++ --std=c++23 -Wall -Werror -pedantic
 * Steve Rizor 3/16/2025
 * 
 * #References used in this code:
 * This code combines parts of model code from Exercises 1 to 4,
 * along with help from LLM-based tools for C++ syntax and structure.
 */
#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>
#include <time.h>
#include <syslog.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/gpio.h>
#include <cstring>
#include <errno.h>
#include "Sequencer.hpp"
#include <sys/mman.h>
#define NSEC_PER_SEC (1000000000)

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
    int dt_sec = stop->tv_sec - start->tv_sec;
    int dt_nsec = stop->tv_nsec - start->tv_nsec;

    if (dt_sec >= 0)
    {
        if (dt_nsec >= 0)
        {
            delta_t->tv_sec = dt_sec;
            delta_t->tv_nsec = dt_nsec;
        }
        else
        {
            delta_t->tv_sec = dt_sec - 1;
            delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
        }
    }
    else
    {
        if (dt_nsec >= 0)
        {
            delta_t->tv_sec = dt_sec;
            delta_t->tv_nsec = dt_nsec;
        }
        else
        {
            delta_t->tv_sec = dt_sec - 1;
            delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
        }
    }

    return 1;
}

bool writeToFile(const std::string &path, const std::string &value) 
{
    std::ofstream fs;
    fs.open(path);
    fs << value;
    fs.close();
    return true;
}
void toggleGPIO1(void)
{
    struct timespec start_time, end_time, delta;

    clock_gettime(CLOCK_REALTIME, &start_time);
    syslog(LOG_INFO, "GPIO Toggling1 started execution at %ld.%09ld seconds", start_time.tv_sec, start_time.tv_nsec);

    static bool isLow = true;

    if(system("pinctrl set 2 op"))
    {
	syslog(LOG_INFO, "Failed to set GPIO 2 as output");
    }

    if(isLow)
    {
    	if(system("pinctrl set 2 dh"))
    	{ 
     	    syslog(LOG_INFO, "Failed to set GPIO 2");
   	    isLow = false;
    	}
    }
    else
    {
	if(system("pinctrl set 2 dl"))
    	{
	    syslog(LOG_INFO, "Failed to reset GPIO 2");
            isLow = true;
    	}
    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    delta_t(&end_time, &start_time, &delta);

    syslog(LOG_INFO, "GPIO Toggling1 completed execution at %ld.%09ld seconds", end_time.tv_sec, end_time.tv_nsec);
    syslog(LOG_INFO, "GPIO Toggling1 execution time: %ld.%09ld seconds", delta.tv_sec, delta.tv_nsec);
}

void toggleGPIO2(void)
{
    const std::string gpioNumber = "516";
    const std::string gpioExportPath = "/sys/class/gpio/export";
    const std::string gpioPath = "/sys/class/gpio/gpio" + gpioNumber + "/";
    const std::string directionPath = gpioPath + "direction";
    const std::string valuePath = gpioPath + "value";
    struct timespec start_time, end_time, delta;

    writeToFile(gpioExportPath, gpioNumber);
    clock_gettime(CLOCK_REALTIME, &start_time);
    
    syslog(LOG_INFO, "GPIO Toggling2 started execution at %ld.%09ld seconds", start_time.tv_sec, start_time.tv_nsec);
	
    if(!writeToFile(directionPath, "out")) 
    {
      syslog(LOG_INFO, "Failed to set GPIO 4 as output");
    }

    static bool isLow = true;

    if(isLow)
    {
        if(!writeToFile(valuePath, "1"))
        {
            syslog(LOG_INFO, "Failed to set GPIO 4");
        }
        isLow = false;
    }
    else
    {
        if(!writeToFile(valuePath, "0"))
        {
            syslog(LOG_INFO, "Failed to reset GPIO 2");
        }
        isLow = true;
    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    delta_t(&end_time, &start_time, &delta);

    syslog(LOG_INFO, "GPIO Toggling2 completed execution at %ld.%09ld seconds", end_time.tv_sec, end_time.tv_nsec);
    syslog(LOG_INFO, "GPIO Toggling2 execution time: %ld.%09ld seconds", delta.tv_sec, delta.tv_nsec);
}

void toggleGPIO3(void)
{
    struct timespec start_time, end_time, delta;

    clock_gettime(CLOCK_REALTIME, &start_time);
    syslog(LOG_INFO, "GPIO Toggling3 started execution at %ld.%09ld seconds", start_time.tv_sec, start_time.tv_nsec);
    
    int fd = open("/dev/gpiochip0", O_RDWR);
    if (fd < 0) 
    {
        syslog(LOG_ERR, "Failed to open /dev/gpiochip0: %s", strerror(errno));
        return;
    }
    struct gpiohandle_request rq;
    struct gpiohandle_data data;
    rq.lineoffsets[0] = 5;
    rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
    rq.lines = 1;
    strcpy(rq.consumer_label, "gpio_toggle5");
    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq) == -1) 
    {
        syslog(LOG_ERR, "GPIO_GET_LINEHANDLE_IOCTL failed: %s", strerror(errno));
        close(fd);
        return;
    }
    static bool isLow = true;
    data.values[0] = isLow ? 1 : 0;
    isLow = !isLow;
    if (ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) == -1) 
    {
        syslog(LOG_ERR, "GPIOHANDLE_SET_LINE_VALUES_IOCTL failed: %s", strerror(errno));
        close(rq.fd);
        close(fd);
        return;
    }
    close(rq.fd);
    close(fd);

    clock_gettime(CLOCK_REALTIME, &end_time);
    delta_t(&end_time, &start_time, &delta);

    syslog(LOG_INFO, "GPIO Toggling3 completed execution at %ld.%09ld seconds", end_time.tv_sec, end_time.tv_nsec);
    syslog(LOG_INFO, "GPIO Toggling3 execution time: %ld.%09ld seconds", delta.tv_sec, delta.tv_nsec);
}

void toggleGPIO4(void)
{
    struct timespec start_time, end_time, delta;

    clock_gettime(CLOCK_REALTIME, &start_time);
    syslog(LOG_INFO, "GPIO Toggling4 started execution at %ld.%09ld seconds", start_time.tv_sec, start_time.tv_nsec);

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem_fd < 0)
    {
        syslog(LOG_ERR, "Failed to open /dev/mem: %s", strerror(errno));
        return;
    }
    const off_t gpio_base = 0xFE200000;
    const size_t block_size = 4096;
    void *gpio_map = mmap(NULL, block_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, gpio_base);
    if(gpio_map == MAP_FAILED)
    {
        syslog(LOG_ERR, "mmap failed: %s", strerror(errno));
        close(mem_fd);
        return;
    }
    volatile unsigned int *gpio = (volatile unsigned int *)gpio_map;
    int gpioPin = 7;
    int fsel_reg = gpioPin / 10;
    int shift = (gpioPin % 10) * 3;
    unsigned int reg_val = gpio[fsel_reg];
    reg_val &= ~(7 << shift);
    reg_val |= (1 << shift);
    gpio[fsel_reg] = reg_val;

    static bool isLow = true;
    if(isLow)
    {
        gpio[7] = (1 << gpioPin);
        isLow = false;
    }
    else
    {
        gpio[10] = (1 << gpioPin);
        isLow = true;
    }
    munmap((void *)gpio_map, block_size);
    close(mem_fd);

    clock_gettime(CLOCK_REALTIME, &end_time);
    delta_t(&end_time, &start_time, &delta);

    syslog(LOG_INFO, "GPIO Toggling4 completed execution at %ld.%09ld seconds", end_time.tv_sec, end_time.tv_nsec);
    syslog(LOG_INFO, "GPIO Toggling4 execution time: %ld.%09ld seconds", delta.tv_sec, delta.tv_nsec);
}
int main(int argc, char *argv[])
{

    openlog("PthreadService", LOG_PID | LOG_CONS | LOG_PERROR, LOG_USER);
    
    if(argc < 2)
    {
         syslog(LOG_INFO, "Provide input arguments from 1 to 4");
         return -1;
    }
    int serviceNum = std::atoi(argv[1]);

    
    // Example use of the sequencer/service classes:
    Sequencer sequencer{};

    switch(serviceNum)
    {
         case 1:
             sequencer.addService(toggleGPIO1, 1, 99, 100);
	     break;
	 case 2:
	     sequencer.addService(toggleGPIO2, 1, 99, 100);
	     break;
	 case 3:
	     sequencer.addService(toggleGPIO3, 1, 99, 100);
	     break;
	 case 4:
	     sequencer.addService(toggleGPIO4, 1, 99, 100);
             break;
         default:
             syslog(LOG_INFO,"Use input arguments from 1 to 4");
             return -1;
    }

    syslog(LOG_INFO, "Starting sequencer with services...");
    sequencer.startServices();
    // todo: wait for ctrl-c or some other terminating condition
     std::this_thread::sleep_for(std::chrono::seconds(1));  // run for 5 seconds
    sequencer.stopServices();
    //std::this_thread::sleep_for(std::chrono::seconds(1));  // run for 5 seconds
    syslog(LOG_INFO, "Stopped sequencer and services.");
    closelog();
    
    return 0;
}
