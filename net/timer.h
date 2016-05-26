#pragma once

#include <ctime>
#include <iostream>
#include <sched.h>
#include <stdint.h>
 
#define LOAD_CPU {for(int x = 0; x < 100000; ++x) {;}}

static inline uint64_t rdtsc()
{
	unsigned int hi, lo; 
	__asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi)); 
	return ((uint64_t) hi << 32) | lo; 
}

class TimerHelper
{
public:
	TimerHelper():
		_ticksPerNs(0.0f)
	{
		cpu_set_t cpuMask;
		CPU_ZERO(&cpuMask);
		CPU_SET(0, &cpuMask);	//fix CPU
		sched_setaffinity(0, sizeof(cpuMask), &cpuMask);
		this->calibrateTicks();
	}

	double ticksToNanoseconds(uint64_t ticks)
	{
		return (double)ticks / _ticksPerNs;
	}

private:
	void calibrateTicks()
	{
		static const int NANO_SECONDS_IN_SEC = 1000000000;

		timespec beginSpec;
		timespec endSpec;

		uint64_t begin;
		uint64_t end;

		clock_gettime(CLOCK_MONOTONIC, &beginSpec);
		begin = rdtsc();
		LOAD_CPU;
		end = rdtsc();
		clock_gettime(CLOCK_MONOTONIC, &endSpec);

		static struct timespec timeDiff;
		timeDiff.tv_sec = endSpec.tv_sec - beginSpec.tv_sec;
		timeDiff.tv_nsec = endSpec.tv_nsec - beginSpec.tv_nsec;
		if (timeDiff.tv_nsec < 0) 
		{
			timeDiff.tv_sec--;
			timeDiff.tv_nsec += NANO_SECONDS_IN_SEC;
		}

		uint64_t nsecElapsed = timeDiff.tv_sec * NANO_SECONDS_IN_SEC + 
							   timeDiff.tv_nsec;
		_ticksPerNs = (double)(end - begin) / (double)nsecElapsed;
		std::cerr << "Ticks per ns: " << _ticksPerNs << std::endl;
	}

	double _ticksPerNs;
};
