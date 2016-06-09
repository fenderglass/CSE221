
#include "timer.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

namespace
{
	TimerHelper tHelper;
}

template <class T>
double meanVec(const std::vector<T>& vals)
{
	double sum = 0.0f;
	for (size_t i = 0; i < vals.size(); ++i) sum += vals[i];
	return sum / vals.size();
}

template <class T>
double stdVec(const std::vector<T>& vals)
{
	double mean = meanVec(vals);
	double sum = 0;
	for (size_t i = 0; i < vals.size(); ++i) 
	{
		sum += (vals[i] - mean) * (vals[i] - mean);
	}
	return sqrt(sum / vals.size());
}

#define MEASURE_DEF(name,func,iters,measurements) \
void name() \
{ \
	const int ITERS_PER_MEASUREMENT = iters; \
	const int MEASUREMENTS = measurements; \
\
	uint64_t timeOne = 0;\
	uint64_t timeTwo = 0;\
	std::vector<uint64_t> diffCycles;\
	std::vector<uint64_t> diffNs;\
\
	for (int i = 0; i < MEASUREMENTS; ++i)\
	{\
		timeOne = rdtsc();\
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)\
		{\
			func;\
		}\
		timeTwo = rdtsc();\
		double avg = (double)(timeTwo - timeOne) / ITERS_PER_MEASUREMENT;\
		diffCycles.push_back(avg);\
		diffNs.push_back(tHelper.ticksToNanoseconds(avg));\
	}\
\
	std::cerr << "\tCycles mean: " << meanVec(diffCycles) \
			  << ", std: " << stdVec(diffCycles)\
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) \
			  << ", std: " << stdVec(diffNs) << std::endl;\
\
}



/////////////////////////////////////////////////////////////////////////
//Time reading overhead

MEASURE_DEF(timeReadMes, rdtsc(), 100000, 1000);
void timeReadingOverhead()
{
	std::cerr << "Time reading overhead: \n";
	timeReadMes();
}

/////////
void loopOverhead()
{
	const int ITERS_PER_MEASUREMENT = 1000;
	const int MEASUREMENTS = 1000;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;

	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		timeOne = rdtsc();
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i) {;}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne) / ITERS_PER_MEASUREMENT;
		diffCycles.push_back(diff);
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}
	
	std::cerr << "Loop overhead: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl;
}

/////////////////////////////////////////////////////////
//Function call overhead

int dummyFunc0() {return 0;}
int dummyFunc1(int a1) {return 0;}
int dummyFunc2(int a1, int a2) {return 0;}
int dummyFunc3(int a1, int a2, int a3) {return 0;}
int dummyFunc4(int a1, int a2, int a3, int a4) {return 0;}
int dummyFunc5(int a1, int a2, int a3, int a4, int a5) {return 0;}
int dummyFunc6(int a1, int a2, int a3, int a4, int a5, int a6) {return 0;}
int dummyFunc7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {return 0;}

MEASURE_DEF(arg0, dummyFunc0(), 1000000, 1000);
MEASURE_DEF(arg1, dummyFunc1(1), 1000000, 1000);
MEASURE_DEF(arg2, dummyFunc2(1, 2), 1000000, 1000);
MEASURE_DEF(arg3, dummyFunc3(1, 2, 3), 1000000, 1000);
MEASURE_DEF(arg4, dummyFunc4(1, 2, 3, 4), 1000000, 1000);
MEASURE_DEF(arg5, dummyFunc5(1, 2, 3, 4, 5), 1000000, 1000);
MEASURE_DEF(arg6, dummyFunc6(1, 2, 3, 4, 5, 6), 1000000, 1000);
MEASURE_DEF(arg7, dummyFunc7(1, 2, 3, 4, 5, 6, 7), 1000000, 1000);

void funcCallOverhead()
{
	std::cerr << "0-args overhead: \n";
	arg0();
	std::cerr << "1-args overhead: \n";
	arg1();
	std::cerr << "2-args overhead: \n";
	arg2();
	std::cerr << "3-args overhead: \n";
	arg3();
	std::cerr << "4-args overhead: \n";
	arg4();
	std::cerr << "5-args overhead: \n";
	arg5();
	std::cerr << "6-args overhead: \n";
	arg6();
	std::cerr << "7-args overhead: \n";
	arg7();
}

/////////////////////////////

MEASURE_DEF(sysCallMes, syscall(SYS_getpid), 100000, 1000);
void systemCallOverhead()
{
	std::cerr << "System call overhead: \n";
	sysCallMes();
}

////////////////////////////
void processCreateOverhead()
{
	const int ITERS_PER_MEASUREMENT = 10000;
	const int MEASUREMENTS = 100;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;

	pid_t pid;
	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		timeOne = rdtsc();
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)
		{
			pid = fork();
			if (pid == 0) _exit(0);
		}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne) / ITERS_PER_MEASUREMENT;
		diffCycles.push_back(diff);
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}
	
	std::cerr << "Process create overhead: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl;
}

void* dummy(void* ) 
{
	pthread_exit(NULL);
}

void threadCreateOverhead()
{
	const int ITERS_PER_MEASUREMENT = 10000;
	const int MEASUREMENTS = 100;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;

	pthread_t t;
	size_t x = 0;
	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		timeOne = rdtsc();
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)
		{
			t = pthread_create(&t, NULL, dummy, (void *)x);
		}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne) / ITERS_PER_MEASUREMENT;
		diffCycles.push_back(diff);
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}
	
	std::cerr << "Thread create overhead: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl;
}

///////////////////////////////////
void processContextSwitchingOverhead()
{
	const int MEASUREMENTS = 1000;

	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;
	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;

	for (size_t i = 0; i < MEASUREMENTS; ++i)
	{
		int fd[2];
		pipe(fd);
		char buf;

		pid_t pid = fork();
		if (pid == 0)
		{
			write(fd[1], &buf, 1);
			_exit(0);
		}
		else
		{
			timeOne = rdtsc();
			read(fd[0], &buf, 1);
			timeTwo = rdtsc();

			double diff = (double)(timeTwo - timeOne) / 2;
			diffCycles.push_back(diff);
			diffNs.push_back(tHelper.ticksToNanoseconds(diff));
		}
	}

	std::cerr << "Process context switching overhead: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl;
}

void* pthreadWriter(void* pipe) 
{
	char buf;
	int* fd = (int*)pipe;

	write(fd[1], &buf, 1);

	pthread_exit(NULL);
}

void threadContextSwitchingOverhead()
{
	const int MEASUREMENTS = 10000;

	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;
	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;

	for (size_t i = 0; i < MEASUREMENTS; ++i)
	{
		int fd[2];
		pipe(fd);
		char buf;
		pthread_t t;
		
		t = pthread_create(&t, NULL, pthreadWriter, (void *)fd);

		timeOne = rdtsc();
		read(fd[0], &buf, 1);
		timeTwo = rdtsc();

		double diff = (double)(timeTwo - timeOne) / 2;
		diffCycles.push_back(diff);
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}

	std::cerr << "Thread context switching overhead: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl;
}

///////////////////////////////////

int main(int argc, char** argv)
{
	timeReadingOverhead();
	loopOverhead();
	funcCallOverhead();
	systemCallOverhead();
	processCreateOverhead();
	threadCreateOverhead();
	processContextSwitchingOverhead();
	threadContextSwitchingOverhead();
	return 0;
}
