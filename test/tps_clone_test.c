#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <tps.h>
#include <sem.h>

/* simple test to check if tps_clone works
 */

static char msg[TPS_SIZE] = "Hello World!\n";
pthread_t tid1, tid2;

void *thread2(void* arg)
{
    char *buffer = malloc(TPS_SIZE);
    /* check current thread tid */
    tps_clone(tid1);

    tps_read(0, TPS_SIZE, buffer);
    assert(!memcmp(msg, buffer, TPS_SIZE));
    printf("thread2: clone OK!\n");

    tps_destroy();
    free(buffer);
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
  	printf("thread1: read OK!\n");

    	/* test clone */
    	pthread_create(&tid2, NULL, thread2, NULL);
    	pthread_join(tid2, NULL);

  	tps_destroy();
  	free(buffer);
  	return NULL;
}


int main()
{
  	tps_init(1);
  	pthread_create(&tid1, NULL, thread1, NULL);
  	pthread_join(tid1, NULL);
  
	return 0;
}
