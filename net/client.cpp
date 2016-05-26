
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
	static const int NUM_MEASUREMENTS = 100000;

	sockaddr_in serverStruct;
    serverStruct.sin_addr.s_addr = inet_addr(argv[1]);
    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(9999);
    
    int socketDesc = socket(AF_INET, SOCK_STREAM, 0);
	connect(socketDesc, (sockaddr *)&serverStruct, sizeof(serverStruct));
     
	char buff;
	size_t timeOne = 0;;
	size_t timeTwo = 0;

	std::vector<double> diffCycles;
	std::vector<double> diffTime;
	for (int j = 0; j < NUM_MEASUREMENTS; ++j) 
	{
		timeOne = rdtsc();
		send(socketDesc, &buff, sizeof(buff), 0);
		recv(socketDesc, &buff, sizeof(buff), 0);
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

	close(socketDesc);
    return 0;
}
