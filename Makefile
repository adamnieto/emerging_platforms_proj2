CFLAGS=-g -Wall

all: main 

sat: sat.h sat.c
	gcc $(CFLAGS) -c --std=c11 sat.c -o sat.o

main: sat 
	gcc $(CFLAGS) --std=c11 -o main main.c sat.o libutil.a

.PHONY: clean

clean:
	rm -f *.o main




