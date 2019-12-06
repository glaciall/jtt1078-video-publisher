#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define SUCCESS 0

void log(char *msg)
{
	time_t now;
	struct tm *timeinfo;
	char str[64];

	time(&now);
	timeinfo = localtime(&now);
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", timeinfo);

	printf("[jtt1078] %s %s\n", str, msg);
}

int terminate(int code, char *msg)
{
	log(msg);
	return code;
}

int x = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *test(void *arg)
{
	log("thread started...");
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 1;
	pthread_mutex_lock(&mutex);
	while (x == 0)
	{
		pthread_cond_timedwait(&cond, &mutex, &ts);
		log("lock timedout");
		ts.tv_sec += 1;
	}
	pthread_mutex_unlock(&mutex);
	log("unlocked...");
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t) * 1);
	pthread_create(thread, NULL, test, NULL);
	log("thread created...");

	sleep(5);

	log("notify...");
	pthread_mutex_lock(&mutex);
	x = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_join(*thread, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	return 0;
}
