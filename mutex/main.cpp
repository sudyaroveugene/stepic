#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h> 

#define BUFSIZE 20

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t spin;
pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
int arg;

void* thread_func( void *mutex_type )
{
	int type = *((int*) mutex_type);
// лочимся
	switch( type )
	{
		case 1: 
			pthread_mutex_lock( &mut );
			break;
		
		case 2:
			pthread_spin_lock( &spin );
			break;
		
		case 3:
			pthread_rwlock_rdlock( &rw );
			break;
		
		case 4:
			pthread_rwlock_wrlock( &rw );
			break;		
		default: ;
			// ничего не делаем
	}
// делаем тут чЁ-то	
	double e=10.0;
	for( int i=0; i<100000; i++ )
	{
		e = i/e + e/i;
		// if( i%10000 == 0 )
			// printf( "i=%d\n", i );
	}
	arg = 1;
//	анлочимся	
	switch( type )
	{
		case 1: 
			pthread_mutex_unlock( &mut );
			break;
		
		case 2:
			pthread_spin_unlock( &spin );
			break;
		
		case 3:
		case 4:
			pthread_rwlock_unlock( &rw );
			break;
		
		default: ;
			// ничего не делаем
	}
//	pthread_exit( 0 );
	return NULL;
}

int main(int argc, char **argv)
{
	int fd;		// декскриптор выходного файла
	char buf[BUFSIZE];
	int res;
	pthread_t thread1, thread2, thread3, thread4; 
	
	fd = open( "/home/box/main.pid", O_RDWR | O_CREAT | O_TRUNC, 0666 );
//	fd = open( "/home/eugene/mutex/main.pid", O_RDWR | O_CREAT | O_TRUNC, 0666 );
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
	
// инициализируем spin_lock - для него нет статической инициализации
	res = pthread_spin_init( &spin, PTHREAD_PROCESS_PRIVATE );
	if( res )
	{
		perror( "error init spin_lock" );
		return 1;
	}
	
	
	printf( "Lock all mutex\n" );	
	res = pthread_mutex_lock( &mut );
	if( res )
	{
		perror( "error mutex lock" );
		return 1;
	}
	res = pthread_spin_lock( &spin );
	if( res )
	{
		perror( "error spin mutex lock" );
		return 1;
	}
	res = pthread_rwlock_rdlock( &rw );
	if( res )
	{
		perror( "error rw mutex read lock " );
		return 1;
	}
	res = pthread_rwlock_rdlock( &rw );
	if( res )
	{
		perror( "error rw mutex read lock" );
		return 1;
	}
	
	printf( "Creating threads\n" );		
	arg = 10;
	int d[4] = {1, 2, 3, 4};
	res = pthread_create( &thread1, NULL, thread_func, &d[0] );
	if( res )
	{
		perror( "error mutex pthread_create" );
		return 1;
	}
	res = pthread_create( &thread2, NULL, thread_func, &d[1] );
	if( res )
	{
		perror( "error mutex pthread_create" );
		return 1;
	}
	res = pthread_create( &thread3, NULL, thread_func, &d[2] );
	if( res )
	{
		perror( "error mutex pthread_create" );
		return 1;
	}
	res = pthread_create( &thread4, NULL, thread_func, &d[3] );
	if( res )
	{
		perror( "error mutex pthread_create" );
		return 1;
	}

	printf( "Go sleeping... Start check button!\n" );		
	sleep( 30 );
	printf( "Unlock all mutex\n" );		
	if( pthread_mutex_unlock( &mut ) )
	{
		perror( "error mutex unlock" );
		return 1;			
	}	
	if( pthread_spin_unlock( &spin ) )
	{
		perror( "error spin unlock" );
		return 1;			
	}	
	if( pthread_rwlock_unlock( &rw ) )
	{
		perror( "error rwlock unlock" );
		return 1;			
	}	
	if( pthread_rwlock_unlock( &rw ) )
	{
		perror( "error rwlock unlock" );
		return 1;			
	}	

// делаем тут чЁ-то
	double s=10.0;
	int j=1;
	while( j<1000000 ) 
	{
		s = j/s+s/j;
		// if( j%100000 == 0 )
			// printf( "j=%d  s=%f\n", j, s );
		j++;
	}

	if( pthread_join( thread1, NULL ) )
	{
		perror( "error mutex pthread1 join" );
		return 1;			
	}
	if( pthread_join( thread2, NULL ) )
	{
		perror( "error mutex pthread2 join" );
		return 1;			
	}
	if( pthread_join( thread3, NULL ) )
	{
		perror( "error mutex pthread3 join" );
		return 1;			
	}
	if( pthread_join( thread4, NULL ) )
	{
		perror( "error mutex pthread4 join" );
		return 1;			
	}	
	
	pthread_mutex_destroy( &mut );
	pthread_spin_destroy( &spin );
	pthread_rwlock_destroy( &rw );

	printf( "Return arg=%d\n", arg );
	
	return 0;
}
