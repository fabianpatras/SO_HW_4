# LD_LIBRARY_PATH=.
# LD_LIBRARY_PATH=. valgrind --leak-check=full ./_test/run_test 6
CPP = gcc
CFLAGS = -Wall -g -pthread
INC = -I util/
LIBS = -L. -lscheduler

build: libscheduler.so

libscheduler.so: so_scheduler.c list.o queue.o
	$(CC) $(CFLAGS) $(INC) -shared $^ -o $@

test: build
	cp libscheduler.so checker-lin/ && cd checker-lin/ && make -f Makefile.checker

main: main.o list.o queue.o
	$(CC) $(CFLAGS) $(INC) $^ $(LIBS) -o $@

main.o: main.c build
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

list.o:

queue.o:

clean:
	rm -rf libscheduler.so *.o main