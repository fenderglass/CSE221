#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "timer.h"

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

void memoryAccessTime(int bytesSize)
{
	const int ITERS_PER_MEASUREMENT = 100000;
	const int MEASUREMENTS = 1000;
	size_t memSize = bytesSize / sizeof(size_t);

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;

	size_t* mem = new size_t[memSize];
	for (int i = 0; i < memSize; ++i) mem[i] = 0;

	size_t tmp;
	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		std::vector<size_t> indices(ITERS_PER_MEASUREMENT);
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)
		{
			indices[i] = rand() % memSize;
		}

		timeOne = rdtsc();
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)
		{
			tmp = mem[indices[i]];
		}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne) / ITERS_PER_MEASUREMENT;
		diffCycles.push_back(diff);
	}

	delete[] mem;
	std::cerr << "Memory access time for "
			  << (float)bytesSize / 1024 << " kbytes: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << tHelper.ticksToNanoseconds(meanVec(diffCycles))
			  << ", std: " << tHelper.ticksToNanoseconds(stdVec(diffCycles)) 
			  << std::endl << std::endl;
}

void memoryReadBandwidth()
{
	const int MEASUREMENTS = 10;
	const int MEM_SIZE = 256 * 1024 * 1024 / sizeof(size_t);

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffNs;

	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		size_t* memFrom = new size_t[MEM_SIZE];
		size_t* memTo = new size_t[MEM_SIZE];
		for (size_t i = 0; i < MEM_SIZE; ++i)
		{
			memFrom[i] = rand();
			memTo[i] = memFrom[i];
		}

		int p = 0;
		timeOne = rdtsc();
		p = memcmp(memFrom, memTo, MEM_SIZE);

		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne);
		diffNs.push_back(256 / (tHelper.ticksToNanoseconds(diff) / 1000000000));

		delete[] memFrom;
		delete[] memTo;
	}
	std::cerr << "Memory read bandwidth (MBytes/s)\n\t"
			  << "\n\tmean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

void memoryWriteBandwidth()
{
	const int MEASUREMENTS = 10;
	const int MEM_SIZE = 512 * 1024 * 1024 / sizeof(int);

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffNs;

	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		size_t* mem = new size_t[MEM_SIZE];
		for (int i = 0; i < MEM_SIZE; ++i) mem[i] = 0;

		timeOne = rdtsc();
		memset(mem, 1, MEM_SIZE);
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne);
		diffNs.push_back(512 / (tHelper.ticksToNanoseconds(diff) / 1000000000));

		delete[] mem;
	}
	std::cerr << "Memory write bandwidth (MBytes/s)\n\t"
			  << "\n\tmean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

void pagefaultTime()
{
	const size_t MEM_SIZE = 4LL * 1024LL * 1024LL * 1024LL;	//32 Gigs
	const size_t TEST_SIZE = MEM_SIZE / 32;
	const size_t MEASUREMENTS = 1000;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;

	size_t* arrTest = new size_t[TEST_SIZE];
	for (size_t i = 0; i < TEST_SIZE; ++i) arrTest[i] = 0;

	size_t* arrFill = new size_t[MEM_SIZE];
	for (size_t i = 0; i < MEM_SIZE; ++i) arrFill[i] = 0;

	//size_t pagesRead = TEST_SIZE * sizeof(size_t) / 4096;

	std::vector<size_t> indices(MEASUREMENTS);
	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		indices[i] = rand() % TEST_SIZE;
	}

	size_t tmp;
	for (size_t i = 0; i < MEASUREMENTS; ++i)
	{
		timeOne = rdtsc();
		tmp = arrTest[indices[i]];
		timeTwo = rdtsc();

		double diff = (double)(timeTwo - timeOne);
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}

	delete[] arrTest;
	delete[] arrFill;
	std::cerr << "Page fault time "
			  << "\n\tTime (ns) mean per page: " << meanVec(diffNs)
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

int main(int argc, char** argv)
{
	memoryAccessTime(2 * 1024);
	memoryAccessTime(4 * 1024);
	memoryAccessTime(8 * 1024);
	memoryAccessTime(16 * 1024);
	memoryAccessTime(32 * 1024);
	memoryAccessTime(64 * 1024);
	memoryAccessTime(128 * 1024);
	memoryAccessTime(256 * 1024);
	memoryAccessTime(512 * 1024);
	memoryAccessTime(1024 * 1024);
	memoryAccessTime(2 * 1024 * 1024);
	memoryAccessTime(4 * 1024 * 1024);
	memoryAccessTime(8 * 1024 * 1024);
	memoryAccessTime(16 * 1024 * 1024);
	memoryAccessTime(32 * 1024 * 1024);
	memoryAccessTime(64 * 1024 * 1024);
	memoryAccessTime(128 * 1024 * 1024);

	memoryReadBandwidth();
	memoryWriteBandwidth();
	pagefaultTime();

	return 0;
}
