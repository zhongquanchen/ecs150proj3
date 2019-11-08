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

enum FLAG {CLONE, NORMAL, MODIFY};

static queue_t MMAPS;

typedef struct Page {
    void * map_addr;
    int counter;
} Page;

typedef struct TPS {
    Page* page;
    pthread_t tid;
} TPS;

/* helper function to find a thread of certain pid
 * Also used in project 2 by Zhongquan Chen group
 * [link] https://github.com/zhongquanchen/ecs150proj2/blob/81007dd70e552f215751c4e8aec464e3aa7b957b/libuthread/uthread.c#L44
 */
static int find_tid(void* data, void* arg)
{
    pthread_t tid = (*(pthread_t*)arg);
    TPS* cur_tps = (TPS*)data;

    // printf("cur_tps->tid : %ld\n", cur_tps->tid);
    // printf("tid is :%ld\n", tid);
    if ( cur_tps->tid == tid )
        return 1;
    return 0;
}

static int find_sig(void* data, void* arg)
{
    TPS* cur_tps = (TPS*)data;

    if(cur_tps->page->map_addr == arg)
        return -1;
    return 0;
}

static void segv_handler(int sig, siginfo_t *si, void *context)
{
    TPS* exist_tps = NULL;
    /*
     * Get the address corresponding to the beginning of the page where the
     * fault occurred
     */
    void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));

    /*
     * Iterate through all the TPS areas and find if p_fault matches one of them
     */
     queue_iterate(MMAPS, find_sig, (void*)p_fault, (void**)&exist_tps );

    if (exist_tps != NULL)
        /* Printf the following error message */
        fprintf(stderr, "TPS protection error!\n");

    /* In any case, restore the default signal handlers */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    /* And transmit the signal again in order to cause the program to crash */
    raise(sig);
}

int page_create(TPS* tps, int flag)
{
    if (flag == CLONE){
        tps->page = NULL;
    }else{
        if (tps->page != NULL) {
            printf("tps->page is not null so free it");
            free(tps->page);
        }
        tps->page = malloc(sizeof(Page));
        tps->page->counter = 1;
        tps->page->map_addr = mmap(NULL, TPS_SIZE, PROT_NONE,
                            MAP_PRIVATE | MAP_ANON, -1, 0);
        if(tps->page->map_addr == MAP_FAILED){
            printf("create mmap fail at tps.c line 86\n");
            return -1;
        }
    }
    return 0;
}

int Copy_On_Write(TPS* tps)
{
    Page* temp_page = malloc(sizeof(Page));
    temp_page->counter = 1;
    temp_page->map_addr = mmap(NULL, TPS_SIZE, PROT_WRITE,
                        MAP_PRIVATE | MAP_ANON, -1, 0);

    mprotect(tps->page->map_addr, TPS_SIZE, PROT_READ);
    memcpy((void*)temp_page->map_addr, (void*)tps->page->map_addr, TPS_SIZE);
    mprotect(tps->page->map_addr, TPS_SIZE, PROT_NONE);
    mprotect(temp_page->map_addr, TPS_SIZE, PROT_NONE);

    tps->page = temp_page;

    return 0;
}

int tps_init(int segv)
{
    MMAPS = queue_create();
    if (segv) {
        struct sigaction sa;

        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = segv_handler;
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGSEGV, &sa, NULL);
    }

    return 0;
}

int tps_create(void)
{
    enter_critical_section();
    /* to check if there already exist a tps in map */

    pthread_t cur_thread = pthread_self();
    TPS* exist_tps = NULL;
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    if (check == 1 || check == -1){
        printf("queue_iterate fail tps : 50, function return %d\n", check);
        exit_critical_section();
        return -1;
    }
    // printf("print check in creation tps :%d", check);

    /* if it exist a tps in map, it will never run the following
     * otherwise it will create a map using mmap() */
    TPS* tps = malloc(sizeof(TPS));
    tps->tid = pthread_self();

    int check_page = page_create(tps, NORMAL);
    if (check_page == -1 || tps->page->counter != 1 || tps->page == NULL){
        printf("create page error at tps.c at line 132\n");
        return -1;
    }

    int check_enq = queue_enqueue(MMAPS, (void*)tps);
    if (check_enq != 0)
        return -1;

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

    int check_rmmap = munmap((void*)exist_tps->page->map_addr, MMAP_SIZE);
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
    if(check == -1){
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
    if (exist_tps->page->map_addr == NULL){
        printf("doesn't exist\n");
    }

    mprotect(exist_tps->page->map_addr, TPS_SIZE, PROT_READ);
    memcpy((void*)buffer, (void*)(exist_tps->page->map_addr + offset), length);
    mprotect(exist_tps->page->map_addr, TPS_SIZE, PROT_NONE);
    exit_critical_section();
    return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
    //printf("enter tps write\n");
    enter_critical_section();
    TPS* exist_tps = NULL;
    pthread_t cur_thread = pthread_self();
    int check = queue_iterate(MMAPS, find_tid, (void*)&cur_thread, (void**)&exist_tps);
    //printf("check in write_tps :%d", check);
    if(check == -1){
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

    if (exist_tps->page->counter > 1){
        Copy_On_Write(exist_tps);
    }

    /* Copies "numBytes" bytes from address "from" to address "to"
     * void * memcpy(void *to, const void *from, size_t numBytes); */
    mprotect(exist_tps->page->map_addr, TPS_SIZE, PROT_WRITE);
    memcpy((void*)(exist_tps->page->map_addr + offset), (void*)buffer, length);
    mprotect(exist_tps->page->map_addr, TPS_SIZE, PROT_NONE);
    //printf("%s", (char*)(exist_tps->page->map_addr));
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
    cur_tps->tid = pthread_self();

    int check_create = page_create(cur_tps, CLONE);
    if (check_create == -1){
        printf("create page error at tps.c at line 267\n");
        return -1;
    }

    target_tps->page->counter ++;
    cur_tps->page = target_tps->page;

    int check_cur = queue_enqueue(MMAPS, (void*)cur_tps);
    if (check_cur == -1){
        printf("enqueue fail tps.c : 168 \n");
        exit_critical_section();
        return -1;
    }

    exit_critical_section();
    return 0;
}
