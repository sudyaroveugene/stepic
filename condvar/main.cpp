#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define BUFSIZE 20
#define STORAGE_MIN 10
#define STORAGE_MAX 20
#define MIN_QUEUE 2

/* –аздел€емый ресурс */
int storage[3] = {STORAGE_MIN, STORAGE_MIN, STORAGE_MIN};
int numproducts = STORAGE_MIN;	// остаток продукции дл€ продажи

pthread_mutex_t mutex;
pthread_cond_t condition;
pthread_barrier_t bar;

/* ‘ункци€ потока потребител€ */
void *consumer(void *args)
{
	puts("[CONSUMER] thread started");
	int toConsume = 0;
	int arg = *((int*) args);
	
	while(1)
	{
		pthread_mutex_lock(&mutex);
		/* ≈сли значение общей переменной меньше максимального, 
		 * то поток входит в состо€ние ожидани€ сигнала о достижении
		 * максимума */
		while (storage[arg] < STORAGE_MAX)
		{
			pthread_cond_wait(&condition, &mutex);
		}
		toConsume = storage[arg]-STORAGE_MIN;
		printf("[CONSUMER %d] storage is maximum, consuming %d\n", arg,	toConsume);
		/* "ѕотребление" допустимого объема из значени€ общей 
		 * переменной */
		storage[arg] -= toConsume;
		printf("[CONSUMER %d] storage = %d\n", arg, storage[arg]);
		pthread_mutex_unlock(&mutex);
			
		pthread_barrier_wait( &bar );
	}
	
	return NULL;
}

/* ‘ункци€ потока производител€ */
void *producer(void *args)
{
	int k=0, j;
	puts("[PRODUCER] thread started");
	
	while (1)
	{
//		usleep(20000);
		pthread_mutex_lock(&mutex);
		/* ѕроизводитель посто€нно увеличивает значение общей переменной */

// этот блок закоментирован дл€ сдачи задани€		
		// k=(int) (3.0*rand()/(RAND_MAX+1.0));
		// j=1+(int) (3.0*rand()/(RAND_MAX+1.0));
		// storage[k]+= j;
		
//		printf("[PRODUCER] storage = %d\n", storage);
		/* ≈сли значение общей переменной достигло или превысило
		 * максимум, поток потребитель уведомл€етс€ об этом */
		if (storage[k] >= STORAGE_MAX)
		{
			printf("[PRODUCER] storage[%d] maximum\n", k);
			pthread_cond_signal(&condition);			
		}
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int res = 0;
	int fd;
	char buf[BUFSIZE];
	pthread_t thProducer, thConsumer;
	
	
	fd = open( "/home/box/main.pid", O_RDWR | O_CREAT | O_TRUNC, 0666 );
//	fd = open( "/home/eugene/condvar/main.pid", O_RDWR | O_CREAT | O_TRUNC, 0666 );
	if( fd==-1 )
	{
		perror( "error open output" );
		return 1;
	}
	snprintf( buf, BUFSIZE, "%d", getpid() );
	printf( "Main process PID=%s\n", buf );
	res = write( fd, buf, strlen( buf ) );
	if( res==-1 )
	{
		perror( "error writing PID to output file" );
		return 1;
	}
	close( fd );

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&condition, NULL);
	pthread_barrier_init( &bar, NULL, MIN_QUEUE );
	
	res = pthread_create(&thProducer, NULL, producer, NULL);
	if (res != 0)
	{
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	int k = 0;
	res = pthread_create(&thConsumer, NULL, consumer, &k);
	if (res != 0)
	{
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	int l = 1;
	res = pthread_create(&thConsumer, NULL, consumer, &l);
	if (res != 0)
	{
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	int m = 2;
	res = pthread_create(&thConsumer, NULL, consumer, &m);
	if (res != 0)
	{
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	
	pthread_join(thProducer, NULL);
	pthread_join(thConsumer, NULL);
	
	return EXIT_SUCCESS;
}