#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
    /* to keep track of the avaible resources*/
    size_t count;
    int block_count;
    queue_t block_q;
};

sem_t sem_create(size_t count)
{
    /* 1.initialize static queue
     * 2.malloc a space and cast to sem_t type */
    sem_t sem = (sem_t)malloc(sizeof(struct semaphore));
    sem->count = count;
    sem->block_q = queue_create();
    return sem;
}

int sem_destroy(sem_t sem)
{
    if (sem == NULL)
        return -1;
    if (queue_length(sem->block_q) != 0)
        return -1;
    free(sem);
    return 0;
}

int sem_down(sem_t sem)
{
    //printf("i am in sem_down\n");
    if (sem == NULL)
        return -1;
    //printf("sem is not null\n");
    enter_critical_section();
    //printf("enter critical count is %zu\n", sem->count);
    if (sem->count == 0){
        //printf("sem is not null\n");
        int check = queue_enqueue(sem->block_q, (void*)pthread_self());
        //printf("enqueue success \n");
        //MARK:- detele this when finish debuging
        assert(check == 0);
        sem->block_count += 1;
        /* thread_block will exit critical section by itself */
        //printf("check success \n");
        thread_block();
    }
    //printf("count is: %zu\n", sem->count);
    sem->count -= 1;
    exit_critical_section();
    return 0;
}

int sem_up(sem_t sem)
{
    if (sem == NULL)
        return -1;

    enter_critical_section();
    if (sem->count == 0){
        if (queue_length(sem->block_q) != 0){
            pthread_t* will_unblock = malloc(sizeof(pthread_t));
            int check = queue_dequeue(sem->block_q, (void**)will_unblock);
            assert(check == 0);
            thread_unblock(*will_unblock);
        }
    }
    sem->count += 1;
    //printf("sem->count : %zu\n", sem->count);
    exit_critical_section();
    return 0;
}

int sem_getvalue(sem_t sem, int *sval)
{
    if (sem == NULL)
        return -1;
    if (sem->count == 0)
        *sval = sem->block_count * -1;
    else
        *sval = (int)sem->count;
    return 0;
}
