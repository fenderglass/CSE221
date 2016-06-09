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

void sequentialRead(const std::string& file, bool master)
{
	const int MEASUREMENTS = 10;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;

	const size_t BUFF_SIZE = 4096;
	char* buffer = (char*)memalign(BUFF_SIZE, BUFF_SIZE);
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

	if (master)
	{
		std::cerr << "File size (MB) " << fileSize
				  << "\n\tPage read time (ms): " << meanVec(diffNs) / numReads
				  << ", std: " << stdVec(diffNs) / numReads << std::endl << std::endl;
	}
}


int main(int argc, char** argv)
{
	std::string fileDir = argv[1];
	int numProc = atoi(argv[2]);

	for (size_t i = 0; i < numProc; ++i)
	{
		pid_t pid = fork();
		if (pid == 0)	//child process
		{
			std::string filename = fileDir + std::to_string(i) + ".bin";
			sequentialRead(filename, i == 0);
			exit(0);
		}
	}

	return 0;
}
