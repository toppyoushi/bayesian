#include <sys/times.h>
#include <unistd.h>

#ifndef TIMER_HPP
#define TIMER_HPP

class Timer {
private:
	//double startTime_;
	clock_t startTime_;

public:
	void start() {
		//startTime_ = clock() / (double)CLOCKS_PER_SEC;
		struct tms timeValues;
		times(&timeValues);
		startTime_ = timeValues.tms_utime;
	}
	
	double elapsed() {
		//double currTime = clock() / (double)CLOCKS_PER_SEC;
		//return currTime - startTime_;
		struct tms timeValues;
		times(&timeValues);
		return (timeValues.tms_utime - startTime_) / (double)sysconf(_SC_CLK_TCK);
	}
};

#endif
