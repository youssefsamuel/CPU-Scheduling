all: run

run:
	g++ -o main main.cpp
clean:
	rm -f main *.o

