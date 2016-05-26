all: server client

server: server.cpp timer.h
	g++ -O0 server.cpp -o server

client: client.cpp timer.h
	g++ -O0 client.cpp -o client

clean:
	rm server
	rm client
