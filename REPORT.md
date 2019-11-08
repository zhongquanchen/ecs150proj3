# ECS150 Project 3 Report #

The aim of Project 3 is to implement threads by building semaphore to sychronize
threads to avoide the situition like race condition. Then build a TPS to let
every thread to have its own private storage.

## Phase 1 ##

### sem_creat ###

We set the space of the semaphore and set the number of the block and set a
queue of block threads.

### sem_destory ###

We first check whether the semaphore is null than check whether there are still
some block threads, the free the semaphore.

### sem down&up ###

We check whether the resources in semaphore is empty if yes. We save the threads
which need resources in the queue and then block it to waiting for other threads
to release the resources.We check the semaphore and release the resources then
unblock the old thread which was blocked.

### sem_getvalue ###


## Phase 2 ##

### struct page ###

We set a basic structure of page which contains a map pointer and the counter of
how many TPS point to it.

### struct TPS ###

The basic structure of TPS which contains a page and Tid.

### Segv_handler ###

we check whether the p fault is in the mmap queue.

### TPS create ###

We create a new TPS here. We make the page counter be 1 since now one TPS point
to the page. We also set the tid to the current tid. We also set the memory space
for the page. We store it in a queue so we can call it later we also set the flag
in mmap to be PROT_NONE so that no one can access it.

### TPS destory ###

We check whether the TPS is exists then delete it form queue then free the whole
TPS space.

### TPS Read ###

We first give the right to read from the TPS, then We use memcpy to copy the
resources in TPS to buffer. Finally we close the right of read from the TPS.

### TPS Write ###

We check whether the page we want to write was copied. If so we call the function
Copy_On_Write which will copy the page and edit. If not copied, we just edit it.
In both situition we all give the right to edit the page and close it after
editing.

### TPS Clone ###

Because of the request of phase 2.3 we do not copy the page at first when we clone.
We just make the new TPS point to the same page and add 1 on the page so that we
know that this page was copied and if we want to edit it, we need copy it first.

## Test ##

We run the test in the test folder and both the three test of the semaphore and
the simple test of TPS all work. We write a test with 2 threads which has message
in its TPS and another one did not have. The we make it copy the memory page and
read from it to check whether the clone was success. 
