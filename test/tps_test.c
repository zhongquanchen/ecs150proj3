#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <tps.h>
#include <sem.h>

/* Simple test to check if the tps works
 * perform following calls
 * tps_write(), tps_read()
 * Output : thread1: read OK!
 */

static char msg[TPS_SIZE] = "Hello World!\n";


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

	tps_destroy();
	free(buffer);
	return NULL;
}


int main()
{
	pthread_t tid;
	tps_init(1);
	pthread_create(&tid, NULL, thread1, NULL);
	pthread_join(tid, NULL);
	
	return 0;
}
