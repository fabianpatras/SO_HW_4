# SO_HW_4

> Proof of concept thread scheduler for a virtual single core CPU

## Main idea

* `so_init()` initialises the scheduler
	* remembers the id of the thread that called it because that thread forks the first thread then should call `so_end()`
	* initialises the internal `TScheduler` struct that has several mutexes, conditional variables, a barrier and a lot more information that is needed to run a thread scheduler

* when a thread starts after a `so_fork()` call the followint things happen:
	* the parent thread creates a `TThread_struct` that has informations about the newly created thread such as `id` and a `conditional variable` that is used to wake up this specific thread when needs be
	* adds the struct into the `ready_q` of the `scheduler`
	* the child thread has to be stopped somehow not to execute its conde until it's planned
	* that is done with a `barrier` and a `wait` on its conditional variable
	* after all this happens: the parent calls `check_scheduler()` that handles who goes next

* there is one mutex in the scheduler that acts as a global lock
* this global lock is used to lock all the operations
* when a thread is put to sleep to wait to be scheduled it waits on its conditional variable, but when it wakes up it tries to acquire to global lock
* until the thread that woke it up releases the global lock, the next thread doesn't start its execution

* this way one thread calls another and only the passing over the status of running thread is locked, the actual execution of code of each thread is not locked

* when a thread calls `so_wait` on some event it is put into a list of waiting threads. this list is associated with that event.

* when a thread calls `so_signal` on some event, then the list of waiting threads is flushed into the `ready_q` of the scheduler

## Build

```make```

* builds a shared librari that you then can import like this

``` gcc main.c -L. -lscheduler -o main```