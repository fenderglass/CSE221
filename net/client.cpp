
#include <vector>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h>

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


int main(int argc , char *argv[])
{
	const int MSG_SIZE = 1024 * 1024;
	const int NUM_BAND = 1000;
	const int NUM_ECHO = 100000;

	size_t timeOne = 0;
	size_t timeTwo = 0;
	
	sockaddr_in serverStruct;
    serverStruct.sin_addr.s_addr = inet_addr(argv[1]);
    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(9998);
    
    int socketDesc = socket(AF_INET, SOCK_STREAM, 0);

	timeOne = rdtsc();
	connect(socketDesc, (sockaddr *)&serverStruct, sizeof(serverStruct));
	timeTwo = rdtsc();
	std::cerr << "Setup: " << tHelper.ticksToNanoseconds(timeTwo - timeOne) 
			  << " ns\n";
     
	char buff[MSG_SIZE];
	for (int i = 0; i < MSG_SIZE; ++i) buff[i] = 42;

	std::vector<double> diffCycles;
	std::vector<double> diffTime;
	char echoBuff;
	for (int j = 0; j < NUM_ECHO; ++j) 
	{
		timeOne = rdtsc();
		send(socketDesc, &echoBuff, sizeof(echoBuff), 0);
		recv(socketDesc, &echoBuff, sizeof(echoBuff), 0);
		timeTwo = rdtsc();

		double diff = (double)(timeTwo - timeOne);
		diffCycles.push_back(diff);
		diffTime.push_back(tHelper.ticksToNanoseconds(diff));
	}

	std::cerr << "Roundtrip time: "
			  << "Cycles mean: " << meanVec(diffCycles) 
			  << ", std: " << stdVec(diffCycles)
			  << "\n\tTime (ns) mean: " << meanVec(diffTime) 
			  << ", std: " << stdVec(diffTime) << std::endl << std::endl;

	diffTime.clear();
	for (int j = 0; j < NUM_BAND; ++j) 
	{
		ssize_t bytesRead;
		timeOne = rdtsc();
		bytesRead = recv(socketDesc, buff, MSG_SIZE, MSG_WAITALL);
		timeTwo = rdtsc();

		double diff = (double)(timeTwo - timeOne);
		diffTime.push_back(1000000000 / tHelper.ticksToNanoseconds(diff));
	}

	std::cerr << "Bandwidth (MBytes/s): "
			  << "\n\tMean: " << meanVec(diffTime) 
			  << ", std: " << stdVec(diffTime) << std::endl << std::endl;

	close(socketDesc);
    return 0;
}
