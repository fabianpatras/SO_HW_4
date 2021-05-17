CPP = gcc
CFLAGS = -Wall -g -pthread
INC = -I util/
LIBS = -L. -lscheduler

build: libscheduler.so

libscheduler.so: so_scheduler.c
	$(CC) $(CFLAGS) $(INC) -shared $^ -o $@

test: build
	cp libscheduler.so checker-lin/ && cd checker-lin/ && make -f Makefile.checker

main: main.o
	$(CC) $(CFLAGS) $(INC) $< $(LIBS) -o $@

main.o: main.c build
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf libscheduler.so main main.o