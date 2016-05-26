all: server.o client.o
	g++ server.o -o server
	g++ client.o -o client

server.o: server.cpp timer.h
	g++ -O0 server.cpp -c -o server.o

client.o: client.cpp timer.h
	g++ -O0 client.cpp -c -o client.o

clean:
	rm *.o
	rm server
	rm client
