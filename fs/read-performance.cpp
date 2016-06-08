#include <vector>
#include <cmath>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <malloc.h>

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

size_t getFileSize(const std::string& file)
{
	int fd = open(file.c_str(), O_RDONLY);
	struct stat buf;
	fstat(fd, &buf);
	close(fd);
	return buf.st_size;
}

void sequentialRead(const std::vector<std::string> files)
{
	const int MEASUREMENTS = 10;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;

	const size_t BUFF_SIZE = 4096;
	char* buffer = (char*)memalign(BUFF_SIZE, BUFF_SIZE);
	for (auto file : files)
	{
		std::vector<uint64_t> diffNs;
		float fileSize = (float)getFileSize(file) / 1024 / 1024;
		int numReads = getFileSize(file) / BUFF_SIZE;

		int fd = open(file.c_str(), O_DIRECT | O_RDWR);
		for (int i = 0; i < MEASUREMENTS; ++i)
		{
			timeOne = rdtsc();
			for (int i = 0; i < numReads; ++i)
			{
				pread(fd, buffer, BUFF_SIZE, i * BUFF_SIZE);
			}
			timeTwo = rdtsc();
			
			double diff = (timeTwo - timeOne);
			diffNs.push_back((double)tHelper.ticksToNanoseconds(diff) / 1000000);
		}
		close(fd);

		std::cerr << "File size (MB) " << fileSize
				  << "\n\tSequential page read time (ms): " << meanVec(diffNs) / numReads
				  << ", std: " << stdVec(diffNs) / numReads << std::endl << std::endl;
	}

}

void randomRead(const std::vector<std::string> files)
{
	const int MEASUREMENTS = 10;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;

	const size_t BUFF_SIZE = 4096;
	char* buffer = (char*)memalign(BUFF_SIZE, BUFF_SIZE);
	for (auto file : files)
	{
		std::vector<uint64_t> diffNs;
		size_t fileSize = getFileSize(file) / 1024 / 1024;
		int numReads = getFileSize(file) / BUFF_SIZE;

		std::vector<size_t> offsets;
		for (int i = 0; i < numReads; ++i)
		{
			size_t offset = (rand() % numReads) * BUFF_SIZE;
			offsets.push_back(offset);
		}

		for (int i = 0; i < MEASUREMENTS; ++i)
		{
			int fd = open(file.c_str(), O_DIRECT);
			pread(fd, buffer, BUFF_SIZE, 0);
			timeOne = rdtsc();
			for (int i = 0; i < numReads; ++i)
			{
				pread(fd, buffer, BUFF_SIZE, offsets[i]);
			}
			timeTwo = rdtsc();
			close(fd);
			
			double diff = (timeTwo - timeOne);
			diffNs.push_back((double)tHelper.ticksToNanoseconds(diff) / 1000000);
		}

		std::cerr << "File size (MB) " << fileSize
				  << "\n\tRandom page read time (ms): " << meanVec(diffNs) / numReads
				  << ", std: " << stdVec(diffNs) / numReads << std::endl << std::endl;
	}

}


int main(int argc, char** argv)
{
	std::vector<std::string> fileList;
	for (size_t i = 1; i < argc; ++i) fileList.push_back(argv[i]);
	sequentialRead(fileList);
	randomRead(fileList);
	return 0;
}
