/*
 * This is a C++ version of the canonical pthread service example. It intends
 * to abstract the service management functionality and sequencing for ease
 * of use. Much of the code is left to be implemented by the student.
 *
 * Build with g++ --std=c++23 -Wall -Werror -pedantic
 * Steve Rizor 3/16/2025
 */

#pragma once

#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include <semaphore.h>
#include <atomic>
#include <csignal>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <queue>
#include <limits>
// The service class contains the service function and service parameters
// (priority, affinity, etc). It spawns a thread to run the service, configures
// the thread as required, and executes the service whenever it gets released.

class Service
{
public:
    Service(Service&&) = default;
    Service& operator=(Service&&) = default;

    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;
    template<typename T>
    Service(T&& doService, uint8_t affinity, uint8_t priority, uint32_t period) :
        _doService(std::forward<T>(doService)),
        _service(),
        _running(true),
        _affinity(affinity),
        _priority(priority),
        _period(period)

    {
        // todo: store service configuration values
        // todo: initialize release semaphore
        sem_init(&_sem, 0, 0);
        // Start the service thread, which will begin running the given function immediately
        _service = std::jthread(&Service::_provideService, this);
    }
 
    void stop(){
        // todo: change state to "not running" using an atomic variable
        // (heads up: what if the service is waiting on the semaphore when this happens?)
        _running.store(false);
        sem_post(&_sem);  // unblock

    }
 

	void printStatistics() const {
	std::cout << "Service (Period: " << _period << " ms) Statistics:\n";
	if (_startJitterCount > 0) {
	    std::cout << "  Start Time Jitter (us): "
	              << "min = " << _minStartJitter << ", "
	              << "max = " << _maxStartJitter << ", "
	              << "avg = " << (_totalStartJitter / _startJitterCount) << "\n";
	}
	if (_execTimeCount > 0) {
	    std::cout << "  Execution Time (us): "
	              << "min = " << _minExecTime << ", "
	              << "max = " << _maxExecTime << ", "
	              << "avg = " << (_totalExecTime / _execTimeCount) << "\n";
	}
	}

	void release(){
        // todo: release the service using the semaphore
        
        struct timespec releaseTime;
    	clock_gettime(CLOCK_REALTIME, &releaseTime);
    	{
            std::lock_guard<std::mutex> lock(_timestampMutex);
            _releaseTimes.push(releaseTime);
    	}
        sem_post(&_sem);
    }
    
    ~Service()
    {
        stop();
        sem_destroy(&_sem);
    }

    sem_t& getSemaphore() { return _sem; }
    uint32_t getPeriod() const { return _period; }
 
private:
    std::function<void(void)> _doService;
    std::jthread _service;


    std::atomic<bool> _running;
    sem_t _sem;

    uint8_t _affinity;
    uint8_t _priority;
    uint32_t _period;
    

    double _minStartJitter = std::numeric_limits<double>::max();
    double _maxStartJitter = 0;
    double _totalStartJitter = 0;
    size_t _startJitterCount = 0;
    double _minExecTime = std::numeric_limits<double>::max();
    double _maxExecTime = 0;
    double _totalExecTime = 0;
    size_t _execTimeCount = 0;

    std::mutex _timestampMutex;
    std::queue<struct timespec> _releaseTimes;



    static inline double diffTimeUs(const struct timespec &start, const struct timespec &end) {
        return (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
    }

    void _taskLoop()
    {
        while (_running.load())
        {
            sem_wait(&_sem);
            if (!_running.load())
	    {
		break;
	    }
	    
        // Retrieve the release time
   	struct timespec releaseTime;
        {
            std::lock_guard<std::mutex> lock(_timestampMutex);
            if (!_releaseTimes.empty()) {
                releaseTime = _releaseTimes.front();
                _releaseTimes.pop();
            } else {
                // If for some reason the queue is empty, fallback
                clock_gettime(CLOCK_REALTIME, &releaseTime);
            }
        }
        
        // Record start time immediately after wakeup
        struct timespec startTime;
        clock_gettime(CLOCK_REALTIME, &startTime);
        
        // Calculate start time jitter
        double startJitter = diffTimeUs(releaseTime, startTime);
        _minStartJitter = std::min(_minStartJitter, startJitter);
        _maxStartJitter = std::max(_maxStartJitter, startJitter);
        _totalStartJitter += startJitter;
        _startJitterCount++;
		
         _doService();
        
	 struct timespec endTime;
        clock_gettime(CLOCK_REALTIME, &endTime);
        
        // Calculate execution time
        double execTime = diffTimeUs(startTime, endTime);
        _minExecTime = std::min(_minExecTime, execTime);
        _maxExecTime = std::max(_maxExecTime, execTime);
        _totalExecTime += execTime;
        _execTimeCount++;
		
	}
    }
    
    void _initializeService()
    {
        // todo: set affinity, priority, sched policy
        // (heads up: the thread is already running and we're in its context right now)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(_affinity, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    sched_param sch_params{};
    sch_params.sched_priority = _priority;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_params);
    }

    void _provideService()
    {
        _initializeService();
        // todo: call _doService() on releases (sem acquire) while the atomic running variable is true
        _taskLoop();
    }


     
 
};
 
// The sequencer class contains the services set and manages
// starting/stopping the services. While the services are running,
// the sequencer releases each service at the requisite timepoint.
class Sequencer
{
public:
    template<typename... Args>
    void addService(Args&&... args)
    {
        // Add the new service to the services list,
        // constructing it in-place with the given args
        //_services.emplace_back(std::forward<Args>(args)...);
            _services.emplace_back(std::make_unique<Service>(std::forward<Args>(args)...));
    }

    void startServices()
    {
        // todo: start timer(s), release services
                _instance = this;

        struct sigaction sa{};
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = _timerHandler;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGRTMIN, &sa, nullptr);

        struct sigevent sev{};
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGRTMIN;
        sev.sigev_value.sival_ptr = &_timerId;

        timer_create(CLOCK_REALTIME, &sev, &_timerId);

        struct itimerspec its{};
        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = 10 * 1000000;        // 10 ms
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 10 * 1000000;     // 10 ms

        timer_settime(_timerId, 0, &its, nullptr);
    }

    void stopServices()
    {
        // todo: stop timer(s), stop services
        timer_delete(_timerId);
        for (auto& service : _services)
            service->stop();
            //service.stop();

        for (auto& service : _services)
            service->printStatistics();

    }

private:
    //std::vector<Service> _services;
    std::vector<std::unique_ptr<Service>> _services;

     static inline Sequencer* _instance = nullptr;
    timer_t _timerId;

    static void _timerHandler(int sig, siginfo_t* si, void* uc)
    {

        static int tick = 0;
        tick += 10;

        if (!_instance) return;

        for (auto& service : _instance->_services)
        {
            //if (tick % service.getPeriod() == 0)
            if (tick % service->getPeriod() == 0)
            {
                //sem_post(&service.getSemaphore());
                sem_post(&service->getSemaphore());
                std::cout << "[Sequencer] Released at tick: " << tick << " ms (Period: "
                          << service->getPeriod() << " ms)\n";
            }
        }

        if (tick >= 5000) tick = 0; // Reset after 1s
    }
};
