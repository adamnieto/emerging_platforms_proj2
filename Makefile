CFLAGS=-g -Wall
THREADS=201
all: main 

sat: sat.h sat.c
	mpicc $(CFLAGS) -c --std=c11 sat.c -o sat.o

main: sat 
	#gcc $(CFLAGS) --std=c11 -o main main.c sat.o libutil.a
	mpicc $(CFLAGS) --std=c11 -o main main.c sat.o libutil.a

run: main
	mpirun  -n  $(THREADS) ./main simple.txt

checkmem: all 
		valgrind --leak-check=full --show-leak-kinds=all -v mpirun -n $(THREADS) ./main simple.txt

.PHONY: clean

clean:
	rm -f *.o main




