
#include <vector>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <signal.h>

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

static int gSocket = 0;

static void sigingHandler(int signo)
{
	if (gSocket != 0) 
	{
		close(gSocket);
	}
	_exit(0);
}

int main(int argc , char *argv[])
{
	static const int NUM_MEASUREMENTS = 100000;

	signal(SIGINT, sigingHandler);
    sockaddr_in serverStruct;
    serverStruct.sin_family = AF_INET;
    serverStruct.sin_addr.s_addr = INADDR_ANY;
    serverStruct.sin_port = htons(9999);
     
	int yes = 1;
    gSocket = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(gSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (bind(gSocket, (sockaddr*)&serverStruct , sizeof(serverStruct)) != 0)
		throw std::runtime_error("Bind failed");
    listen(gSocket, 10);
     
    while(true)
    {
		sockaddr_in client_struct;
    	int structSize = sizeof(sockaddr_in);
    	int clientSock = accept(gSocket, (sockaddr*)&client_struct, 
								 (socklen_t*)&structSize);
    	if (clientSock < 0)
			throw std::runtime_error("Can't accept");

		std::cerr << "Connected\n";
        char buff;
        for (int j = 0; j < NUM_MEASUREMENTS; ++j) 
		{
            recv(clientSock, &buff, sizeof(buff), 0);
            send(clientSock, &buff, sizeof(buff), 0);
        }
		//size_t te = rdtsc();
        close(clientSock);
		//size_t ts = rdtsc();
    }
    return 0;
}
