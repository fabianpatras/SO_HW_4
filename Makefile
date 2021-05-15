CPP = gcc
CFLAGS = -Wall -g -shared
INC = -I util/

build:libscheduler.so

libscheduler.so: so_scheduler.c
	$(CC) $(CFLAGS) $(INC) $^ -o $@

clean:
	rm -rf libscheduler.so