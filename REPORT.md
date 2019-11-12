# ECS150 Project 3 Report #
The aim of Project 3 is to implement semaphore lock on threads, which will gain
stable and safty to the threads library. The use of semaphore eliminate race 
condition where both thread access the same data at almost the same time.

## Phase 1 ##

### [sem_creat](https://github.com/zhongquanchen/ecs150proj3/blob/ddd85a60f9c8cff59177354358493df40586cc4a/libuthread/sem.c#L17) ###

Initialize a semaphore lock. @count parameter will use to set the total availible
lock for semaphore. First, allocate a size for sem struct. Then, assign the count
to total avalible locks. And assign a queue within the struct.

### [sem_destory](https://github.com/zhongquanchen/ecs150proj3/blob/ddd85a60f9c8cff59177354358493df40586cc4a/libuthread/sem.c#L27) ###

This function will check & free semaphore location. 
We first check whether the semaphore is null than check whether there are still
some block threads, the free the semaphore.

### [sem down](https://github.com/zhongquanchen/ecs150proj3/blob/ddd85a60f9c8cff59177354358493df40586cc4a/libuthread/sem.c#L37) ###

This function will take a lock, which locks the resource to access. 
We check whether the locks are avalible in semaphore (count != 0). if count = 0
it will put itself into block state, wait until other thread wakes it. We set
up a while loop in case that when it wake by A thread but B thread is taking 
the resource. After locking, count - 1.

### [sem_up](https://github.com/zhongquanchen/ecs150proj3/blob/ddd85a60f9c8cff59177354358493df40586cc4a/libuthread/sem.c#L57) ###

This function will release the lock, which count + 1.
After it release the lock, it will check if there is any thread block in the 
queue. If there is, it will dequeue it, otherwise exit the function.

### sem_getvalue ###

This function check if the semaphore is null (not created) will return -1.
Otherwise return 0, @sval will capture the value of availible locks.

## [Phase 2](https://github.com/zhongquanchen/ecs150proj3/blob/ddd85a60f9c8cff59177354358493df40586cc4a/libuthread/tps.c#L25) ##

### struct page ###

We set a basic structure of page which contains a pointer which points to a page 
of memory and a refernce showing that there is sharing pages of this memory.

### struct TPS ###

The basic structure of TPS which contains a page struct and thread id.

### static int find_tid(void* data, void* arg) ###

We set up this function because it will be needed to search for a corresponding 
thread and using by ` queue_iterate ` function. Bascly, it compare both address
and return 1 if they are the same. 

### static int find_sig(void* data, void* arg) ###

Other helper function that will used by ` queue iterate `. This function will
capture the signal and compare it to each page memory location, to see if the
signal is comming from TPS segmentation fault. Same working idea above. 

### Segv_handler ###

This function will handle the signal that is comming from TPS, and distinguish
them from seg fault. in other words, it can tell whether this seg fault is cause
by TPS part.

### TPS create ###

This function will create a TPS for current thread. 
First, it will detect if it exist a TPS in this thread, return -1 if it exist.
Then, it will malloc a TPS struct for this thread, set its tid to the TPS. 
And enqueue it to the gloabe queue in order to gain access. 

### page_create ###

This function is used to create a page struct for TPS struct. 
First it will detect if @flag is clone or normal.
CLONE : it will set the page memory to NULL, and assign to other page later.
NORMAL : it will allocate a page struct, melloc a memory page in ` Page strcut `.

### TPS destory ###

We check whether the TPS is exists then delete it form queue then free the whole
TPS space.

### int Copy_On_Write(TPS* tps) ###

This function will perform a memory copy on the reference that it points to.
It will used by TPS Write to perform a copy before editing the page. So that
the new change will not affect other copy which points to the original.

### TPS Read ###

This function will reads the memory page copy into @buffer. 
We first give the right to read from the TPS, then We use memcpy to copy the
resources in TPS to buffer. Finally we close the right of read from the TPS.

### TPS Write ###

This function will write @buffer content into the memory page for current thread.
We check whether the page we want to write was copied. If so we call the function
Copy_On_Write which will copy the page and edit. If not copied, we just edit it.
In both situition we all give the right to edit the page and close it after
editing.

### TPS Clone ###

This function will let the page pointer in TPS struct points to the page where given
by @tid.
We make the new TPS points to the same page and add 1 on the page so that we
know that this page was copied and if we want to edit it, we need copy it first.

## Test ##

We run the test in the test folder. Three test of the semaphore and
the simple test of TPS all work. 
` tps_test.c ` will test if the tps struct create succeed. 
` tps_clone_test.c ` will test if the clone function works fine. (used in phase 2 but 
also work after project completed)
` tps_stress.c ` it will test the performence of TPS. To detect if TPS will have some 
 wrong or read wrong.
 Currently all test passed. 
