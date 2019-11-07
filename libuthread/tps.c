#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"

#define MMAP_SIZE 4096

static queue_t MMAPS;

typedef struct TPS {
    void* map_addr;
    pthread_t tid;
} TPS;

/* helper function to find a thread of certain pid
 * Also used in project 2 by Zhongquan Chen group
 * [link] https://github.com/zhongquanchen/ecs150proj2/blob/81007dd70e552f215751c4e8aec464e3aa7b957b/libuthread/uthread.c#L44
 */
static int find_tid(void* data, void* arg)
{
    TPS* cur_tps = (TPS*)data;
    if ( cur_tps->tid == (*(pthread_t*)arg) )
        return 1;
    return 0;
}

int tps_init(int segv)
{
    MMAPS = queue_create();
    return 0;
}

int tps_create(void)
{
    enter_critical_section();
    /* to check if there already exist a tps in map */

    pthread_t cur_thread = pthread_self();
    TPS* exist_tps = NULL;
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    if (check == 1 && check == -1){
        printf("queue_iterate fail tps : 50, function return %d\n", check);
        return -1;
    }

    /* if it exist a tps in map, it will never run the following
     * otherwise it will create a map using mmap() */
    TPS* tps = malloc(sizeof(TPS));
    tps->map_addr = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANON, 0, 0);
    if (tps->map_addr == MAP_FAILED)
        return -1;
    queue_enqueue(MMAPS, (void*)tps);

    exit_critical_section();
    return 0;
}

int tps_destroy(void)
{
    enter_critical_section();
    pthread_t cur_thread = pthread_self();
    TPS* exist_tps = NULL;

    int check_iter = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    if (check_iter == 0 && check_iter == -1){
        printf("queue_iterate fail tps : 74, function return %d\n", check_iter);
        return -1;
    }

    int check_rmmap = munmap((void*)exist_tps->map_addr, MMAP_SIZE);
    if(check_rmmap == -1){
        printf("removing map address fail tps.c : 79\n");
        return -1;
    }

    int check_del = queue_delete(MMAPS, (void*)exist_tps);
    if (check_del == -1){
        printf("queue_delete fail at tps.c :79\n");
        return -1;
    }

    free(exist_tps);
    exit_critical_section();
    return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
    enter_critical_section();
    TPS* exist_tps = NULL;
    pthread_t cur_thread = pthread_self();
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    if(check == 0 && check == -1){
        printf("queue_iterate fail tps.c : 100\n");
        exit_critical_section();
        return -1;
    }

    /* -1 if current thread doesn't have a TPS, or if the reading operation
     * is out of bound, or if @buffer is NULL, or in case of internal failure.*/
    if (length + offset > MMAP_SIZE){
        exit_critical_section();
        return -1;
    }
    if (buffer == NULL){
      exit_critical_section();
      return -1;
    }

    /* Copies "numBytes" bytes from address "from" to address "to"
     * void * memcpy(void *to, const void *from, size_t numBytes); */
    memcpy(buffer, exist_tps->map_addr + offset, length);
    exit_critical_section();
    return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
    enter_critical_section();
    TPS* exist_tps = NULL;
    pthread_t cur_thread = pthread_self();
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    if(check == 0 && check == -1){
        printf("queue_iterate fail tps.c : 100\n");
        exit_critical_section();
        return -1;
    }

    /* -1 if current thread doesn't have a TPS, or if the writing operation
     * is out of bound, or if @buffer is NULL, or in case of failure. */
    if (length + offset > MMAP_SIZE){
        exit_critical_section();
        return -1;
    }

    if (buffer == NULL){
        exit_critical_section();
        return -1;
    }

    /* Copies "numBytes" bytes from address "from" to address "to"
     * void * memcpy(void *to, const void *from, size_t numBytes); */
    memcpy(exist_tps->map_addr + offset, buffer, length);
    exit_critical_section();
    return 0;
}

int tps_clone(pthread_t tid)
{
    enter_critical_section();
    /* check if target tid exist */
    TPS* target_tps = NULL;
    int check_tar = queue_iterate(MMAPS, find_tid, (void*)&tid, (void**)&target_tps);
    if (check_tar == 0 && check_tar == -1){
        printf("cannot find target tid exist tps\n");
        exit_critical_section();
        return -1;
    }

    /* check if current tid has already exist */
    TPS* cur_tps = NULL;
    pthread_t cur_thread = pthread_self();
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&cur_tps);
    if(check == 1 && check == -1){
        printf("queue_iterate fail tps.c : 100\n");
        exit_critical_section();
        return -1;
    }

    /* after the condition of clone establish, do following... */
    cur_tps = malloc(sizeof(TPS));
    cur_tps->map_addr = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANON, 0, 0);
    if (cur_tps->map_addr == MAP_FAILED)
        return -1;
    memcpy((void*)(cur_tps->map_addr), (void*)(target_tps->map_addr), MMAP_SIZE);
    int check_cur = queue_enqueue(MMAPS, (void*)cur_tps);
    if (check_cur == -1){
        printf("enqueue fail tps.c : 168 \n");
        exit_critical_section();
        return -1;
    }

    exit_critical_section();
    return 0;
}
