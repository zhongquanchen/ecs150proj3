#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#include <tps.h>
#include <sem.h>

/* Simple test to check if seg fault handle TPS protection */

void *latest_mmap_addr; // global variable to make address returned by mmap accessible
void *__real_mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
void *__wrap_mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    latest_mmap_addr = __real_mmap(addr, len, prot, flags, fildes, off);
    return latest_mmap_addr;
}

sem_t sem1, sem2;

void *thread2(void *arg)
{
    /* Change stderr output */
    fprintf(stderr, "TPS protection error!\n");	
    /* Get TPS page address as allocated via mmap() */
    char *tps_addr = latest_mmap_addr;
    /* Cause an intentional TPS protection error */
    tps_addr[0] = '\0';
    sem_up(sem1);
    return NULL;
}

void *thread1(void *arg)
{
    /* create a thread and assign a map point to it */
    pthread_t tid2;
    
    __wrap_mmap(&tid2, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    pthread_create(&tid2, NULL, thread2, NULL);
    
    sem_down(sem1);
    return NULL;    
}

int main(int argc, char **argv)
{
    pthread_t t1;
    sem1 = sem_create(0);
    sem2 = sem_create(0);

    tps_init(1);

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_join(t1, NULL);

    sem_destroy(sem1);
    sem_destroy(sem2);
}


