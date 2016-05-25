#include <vector>
#include <cmath>
#include <cstdlib>

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

void memoryAccessTime(int memSize)
{
	const int ITERS_PER_MEASUREMENT = 10000;
	const int MEASUREMENTS = 10000;
	memSize = memSize / sizeof(size_t);

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;

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
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}

	delete[] mem;
	std::cerr << "Memory access time for "
			  << memSize / 1024 << " kbytes: \n\t"
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

void memoryReadBandwidth()
{
	const int MEASUREMENTS = 100;
	const int MEM_SIZE = 32 * 1024 * 1024 / sizeof(int);

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffNs;

	size_t* mem = new size_t[MEM_SIZE];
	for (int i = 0; i < MEM_SIZE; ++i) mem[i] = 0;

	size_t tmp;
	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		timeOne = rdtsc();
		size_t p = 0;
		while (p < MEM_SIZE)
		{
			tmp = mem[p++];
		}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne);
		diffNs.push_back(32 / (tHelper.ticksToNanoseconds(diff) / 1000000000));
	}

	delete[] mem;
	std::cerr << "Memory read bandwidth (MB/s)\n\t"
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

void memoryWriteBandwidth()
{
	const int MEASUREMENTS = 100;
	const int MEM_SIZE = 32 * 1024 * 1024 / sizeof(int);

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffNs;

	size_t* mem = new size_t[MEM_SIZE];
	for (int i = 0; i < MEM_SIZE; ++i) mem[i] = 0;

	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		timeOne = rdtsc();
		size_t p = 0;
		while (p < MEM_SIZE)
		{
			mem[p++] = 42;
		}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne);
		diffNs.push_back(32 / (tHelper.ticksToNanoseconds(diff) / 1000000000));
	}

	delete[] mem;
	std::cerr << "Memory write bandwidth (MB/s)\n\t"
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

void pagefaultTime()
{
	const int ITERS_PER_MEASUREMENT = 10000;
	const int MEASUREMENTS = 10000;

	const size_t MEM_SIZE = 2LL * 1024LL * 1024LL * 1024LL;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;
	std::vector<uint64_t> diffCycles;
	std::vector<uint64_t> diffNs;

	size_t* arrOne = new size_t[MEM_SIZE];
	for (size_t i = 0; i < MEM_SIZE; ++i) arrOne[i] = 0;

	size_t* arrTwo = new size_t[MEM_SIZE];
	for (size_t i = 0; i < MEM_SIZE; ++i) arrTwo[i] = 0;

	size_t tmp;
	for (int i = 0; i < MEASUREMENTS; ++i)
	{
		std::vector<size_t> indices(ITERS_PER_MEASUREMENT);
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)
		{
			indices[i] = rand() % MEM_SIZE;
		}

		timeOne = rdtsc();
		for (int i = 0; i < ITERS_PER_MEASUREMENT; ++i)
		{
			tmp = arrOne[indices[i]];
		}
		timeTwo = rdtsc();
		double diff = (double)(timeTwo - timeOne) / ITERS_PER_MEASUREMENT;
		diffCycles.push_back(diff);
		diffNs.push_back(tHelper.ticksToNanoseconds(diff));
	}

	delete[] arrOne;
	delete[] arrTwo;
	std::cerr << "Page fault time "
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffNs) 
			  << ", std: " << stdVec(diffNs) << std::endl << std::endl;
}

int main(int argc, char** argv)
{
	memoryAccessTime(1024);
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
