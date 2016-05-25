all: main.o
	g++ main.o -o memory

main.o: main.cpp timer.h
	g++ -O0 main.cpp -c -o main.o

clean:
	rm *.o
	rm memory
