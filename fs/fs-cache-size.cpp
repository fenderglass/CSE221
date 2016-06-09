#include <vector>
#include <cmath>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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

void fileReadTime(const std::vector<std::string> files)
{
	const int MEASUREMENTS = 10;

	uint64_t timeOne = 0;
	uint64_t timeTwo = 0;

	const size_t BUFF_SIZE = 128 * 1024;
	char* buffer = new char[BUFF_SIZE];
	for (auto file : files)
	{
		size_t fileSize = getFileSize(file) / 1024 / 1024;
		size_t numPages = getFileSize(file) / 4096;

		std::vector<uint64_t> diffNs;
		for (int i = 0; i < MEASUREMENTS; ++i)
		{
			int fd = open(file.c_str(), O_RDONLY);
			timeOne = rdtsc();
			while (read(fd, buffer, BUFF_SIZE)) {;}
			timeTwo = rdtsc();
			close(fd);
			
			double diff = (timeTwo - timeOne);
			diffNs.push_back((double)tHelper.ticksToNanoseconds(diff) / 1000000);
		}

		std::cerr << "File size (MB) " << fileSize
				  << "\n\tPage reading time (milisec): " << meanVec(diffNs) / numPages
				  << ", std: " << stdVec(diffNs) / numPages << std::endl << std::endl;
	}
}


int main(int argc, char** argv)
{
	std::vector<std::string> fileList;
	for (size_t i = 1; i < argc; ++i) fileList.push_back(argv[i]);
	fileReadTime(fileList);
	return 0;
}
