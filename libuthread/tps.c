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

struct TPS {
    void* map_addr;
    pthread_t tid;
};

/* helper function to find a thread of certain pid
 * Also used in project 2 by Zhongquan Chen group
 * [link] https://github.com/zhongquanchen/ecs150proj2/blob/81007dd70e552f215751c4e8aec464e3aa7b957b/libuthread/uthread.c#L44
 */
static int find_tid(void* data, void* arg)
{
    struct TPS* cur_tps = (struct TPS*)data;
    if ( cur_tps->tid == (*(pthread_t*)arg) )
        return 1;
    return 0;
}

int tps_init(int segv)
{
    return 0;
}

int tps_create(void)
{
    enter_critical_section();
    /* to check if there already exist a tps in map */

    pthread_t cur_thread = pthread_self();
    struct TPS* exist_tps = NULL;
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    if (check == 1)
        return -1;

    exit_critical_section();
    enter_critical_section();
    /* if it exist a tps in map, it will never run the following
     * otherwise it will create a map using mmap() */

    struct TPS* tps = malloc(sizeof(struct TPS));
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
	/* TODO: Phase 2 */
    return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
    return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
    return 0;
}

int tps_clone(pthread_t tid)
{
	/* TODO: Phase 2 */
    return 0;
}
