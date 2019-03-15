#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h> 

#define BUFSIZE 20

void* thread_func( void *arg )
{
// делаем тут чЁ-то	
	double e=10.;
	for( int i=0; i<100000; i++ )
	{
		e = i/e + e/i;
		if( i%10000 == 0 )
			printf( "i=%d\n", i );
		*((int*) arg) = i;
	}
	*((int*) arg) = 1;
	pthread_exit( 0 );
}

int main(int argc, char **argv)
{
	int fd;		// декскриптор выходного файла
	char buf[BUFSIZE];
	int res, arg;
	pthread_t new_thread; 
	
//	fd = open( "/home/box/main.pid", O_RDWR | O_CREAT | O_TRUNC, 0666 );
	fd = open( "/home/eugene/thread1/main.pid", O_RDWR | O_CREAT | O_TRUNC, 0666 );
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

	arg = 10;
	res = pthread_create( &new_thread, NULL, thread_func, &arg );
	if( res )
	{
		perror( "error pthread_create" );
		return 1;
	}

// делаем тут чЁ-то
	// double s=10.0;
	// int j=1;
	// while( j<1000000 ) 
	// {
		// s = j/s+s/j;
		// if( j%10000 == 0 )
			// printf( "j=%d  s=%f\n", j, s );
		// j++;
	// }

	// res = pthread_join( new_thread, NULL );
	printf( "wait...\n" );
	while( 1 );
	printf( "Return arg=%d\n", arg );
	
	return 0;
}
