#include <stdio.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <unistd.h>

#define SEMINIT 66

int main(int argc, char **argv)
{
	sem_t* semd;	// дескриптор семафора
	int res;
	
	sem_unlink( "/test.sem" );
	semd = sem_open( "/test.sem", O_CREAT, 0666, SEMINIT );
	if( semd==SEM_FAILED )
	{
		perror( "error sem_open" );
		return 1;
	}
	sem_getvalue( semd, &res );
	printf( "Semaphor = %d.\n", res );
	while( 1 ) 
		sleep(1);
	
	return 0;
}
