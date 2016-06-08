
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
	const int MSG_SIZE = 1024 * 1024;
	const int NUM_BAND = 1000;
	const int NUM_ECHO = 100000;

	size_t timeOne = 0;
	size_t timeTwo = 0;

	signal(SIGINT, sigingHandler);
    sockaddr_in serverStruct;
    serverStruct.sin_family = AF_INET;
    serverStruct.sin_addr.s_addr = INADDR_ANY;
    serverStruct.sin_port = htons(9998);
     
	int yes = 1;
    gSocket = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(gSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (bind(gSocket, (sockaddr*)&serverStruct , sizeof(serverStruct)) != 0)
		throw std::runtime_error("Bind failed");
    listen(gSocket, 10);
     
    char buff[MSG_SIZE];
	for (int i = 0; i < MSG_SIZE; ++i) buff[i] = 42;
    while(true)
    {
		sockaddr_in client_struct;
    	int structSize = sizeof(sockaddr_in);
    	int clientSock = accept(gSocket, (sockaddr*)&client_struct, 
								 (socklen_t*)&structSize);
    	if (clientSock < 0)
			throw std::runtime_error("Can't accept");

		std::cerr << "Connected\n";

        char echoBuf[1024];
        for (int j = 0; j < NUM_ECHO; ++j) 
		{
            recv(clientSock, &echoBuf, sizeof(echoBuf), 0);
            send(clientSock, &echoBuf, sizeof(echoBuf), 0);
        }

		for (int i = 0; i < NUM_BAND; ++i)
		{
            recv(clientSock, &echoBuf, sizeof(echoBuf), 0);
        	send(clientSock, &buff, MSG_SIZE, 0);
		}

		timeOne = rdtsc();
        close(clientSock);
		timeTwo = rdtsc();
		std::cerr << "Teardown: " << tHelper.ticksToNanoseconds(timeTwo - timeOne) 
				  << " ns\n";
    }
    return 0;
}
