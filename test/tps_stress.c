#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <tps.h>
#include <sem.h>

/* STRESS TEST TO CHECK IF TPS IS REALIABLE
 * GET READY TO COUNTER BUGS
 */

static char msg[TPS_SIZE] = "Stress1\n";
static char msg1[TPS_SIZE] = "stress1\n";
static char msg2[TPS_SIZE] = "STress1\n";

sem_t sem1, sem2, sem3;
pthread_t tid1, tid2, tid3, tid4;

void *thread4(void* arg)
{
    sem_up(sem1);
    printf("finished testing\n");
    return NULL;
}

void *thread3(void* arg)
{
    pthread_create(&tid4, NULL, thread4, NULL);

    sem_down(sem1);
    char* buffer = malloc(TPS_SIZE);
    int check_clone = tps_clone(tid2);
    assert(check_clone == 0);

    memset(buffer, 0, TPS_SIZE);
    tps_read(0, TPS_SIZE, buffer);
    assert(!strcmp(msg2, buffer));

    buffer[0] = 's';
    tps_write(0, 1, buffer);
    memset(buffer, 0, TPS_SIZE);
    tps_read(0, TPS_SIZE, buffer);
    assert(!strcmp(msg1, buffer));

    printf("t3, tps_read, tps_clone, tps_write OK!\n");
    printf("thread3 create tps succeed\n");

    return NULL;
}

void *thread2(void* arg)
{
    char* buffer = malloc(TPS_SIZE);
    int check_clone = tps_clone(tid1);
    assert(check_clone == 0);

    tps_read(0, TPS_SIZE, buffer);
    assert(!memcmp(msg, buffer, TPS_SIZE));
    sem_up(sem1);

    buffer[0] = 'S';
    buffer[1] = 'T';
    tps_write(0, 2, buffer);
    memset(buffer, 0, TPS_SIZE);
    tps_read(0, TPS_SIZE, buffer);
    printf("buffer : %s", buffer);
    assert(!memcmp(msg2, buffer, TPS_SIZE));

    printf("t2, tps_read, tps_clone, tps_write OK!\n");
    printf("thread2 create tps succeed\n");
    return NULL;
}

void *thread1(void* arg)
{
		char* buffer = malloc(TPS_SIZE);
		int check_create = tps_create();
		assert(check_create == 0);

		tps_write(0, TPS_SIZE, msg);
		memset(buffer, 0, TPS_SIZE);

		tps_read(0, TPS_SIZE, buffer);
		assert(!memcmp(msg, buffer, TPS_SIZE));

    printf("t1, tps_write OK!\n");
    printf("t1, tps_read OK!\n");
    printf("thread1 create tps succeed\n");

    sem_down(sem1);
    /* come back from thread4 */

		tps_destroy();
		free(buffer);
		return NULL;
}


int main()
{
    sem1 = sem_create(0);
    sem2 = sem_create(0);
    sem3 = sem_create(0);

		tps_init(1);
		pthread_create(&tid1, NULL, thread1, NULL);
    pthread_create(&tid2, NULL, thread2, NULL);
    pthread_create(&tid3, NULL, thread3, NULL);
		pthread_join(tid1, NULL);

    sem_destroy(sem1);
    sem_destroy(sem2);
    sem_destroy(sem3);

		return 0;
}
