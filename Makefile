# LD_LIBRARY_PATH=. ./_test/run_test 6
# LD_LIBRARY_PATH=. ./main
# LD_LIBRARY_PATH=. valgrind --leak-check=full --show-leak-kinds=all ./_test/run_test 6
# LD_LIBRARY_PATH=. valgrind --leak-check=full --show-leak-kinds=all ./main
# LD_LIBRARY_PATH=. valgrind --leak-check=full --show-leak-kinds=all --show-reachable=yes --vex-iropt-register-updates=allregs-at-mem-access ./main
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