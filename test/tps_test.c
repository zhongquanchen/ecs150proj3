#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <tps.h>
#include <sem.h>

static sem_t sem1;
static char msg[TPS_SIZE] = "Hello World!\n";


void *thread1(void* arg)
{
	char* buffer = malloc(TPS_SIZE);
	printf("successed enter thread1 and creating tps\n");
	int check_create = tps_create();
	assert(check_create == 0);
	printf("after assert create tps\n");
	
	tps_write(0, TPS_SIZE, msg);


	memset(buffer, 0, TPS_SIZE);
	tps_read(0, TPS_SIZE, buffer);
	assert(!memcmp(msg, buffer, TPS_SIZE));
	printf("thread2: read OK!\n");

	tps_destroy();
	free(buffer);

	return NULL;
}

int main()
{
	pthread_t tid;

	sem1 = sem_create(1);

	tps_init(1);

	pthread_create(&tid, NULL, thread1, NULL);
	pthread_join(tid, NULL);

	sem_destroy(sem1);
	return 0;
}
