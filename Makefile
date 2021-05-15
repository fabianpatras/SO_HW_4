CPP = gcc
CFLAGS = -Wall -g -shared -pthread
INC = -I util/

build: libscheduler.so

libscheduler.so: so_scheduler.c
	$(CC) $(CFLAGS) $(INC) $^ -o $@

test: build
	cp libscheduler.so checker-lin/ && cd checker-lin/ && make -f Makefile.checker

clean:
	rm -rf libscheduler.so