all: main.o
	g++ main.o -lpthread -o cpu

main.o: main.cpp timer.h
	g++ -O0 main.cpp -c -o main.o

clean:
	rm *.o
	rm cpu
