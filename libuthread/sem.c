#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

static queue_t block_q;

struct semaphore {
    /* to keep track of the avaible resources*/
    size_t count;
    int block_count;
};

sem_t sem_create(size_t count)
{
    /* 1.initialize static queue
     * 2.malloc a space and cast to sem_t type */
    block_q = queue_create();
    sem_t sem = (sem_t)malloc(sizeof(struct semaphore));
    sem->count = count;
    return sem;
}

int sem_destroy(sem_t sem)
{
    if (sem == NULL)
        return -1;
    if (queue_length(block_q) != 0)
        return -1;
    free(sem);
    return 0;
}

int sem_down(sem_t sem)
{
    if (sem == NULL)
        return -1;
    
    enter_critical_section();
    if (sem->count == 0){
        int check = queue_enqueue(block_q, (void*)pthread_self());
        //MARK:- detele this when finish debuging
        assert(check == 0);
        sem->block_count += 1;
        /* thread_block will exit critical section by itself */
        thread_block();
    }
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
        if (block_q == NULL)
            return -1;
        
        pthread_t will_unblock = NULL;
        int check = queue_dequeue(block_q, (void**)will_unblock);
        assert(check == 0);
        thread_unblock(will_unblock);
    }
    sem->count += 1;
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

