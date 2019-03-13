#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define NUMSEM 16

int main(int argc, char **argv)
{
	int semd;	// дескриптор очереди
	unsigned short values[NUMSEM];	// значения для семафоров
	int res;
	
	key_t tok = ftok( "/tmp/sem.temp", 1 );

	semd = semget( tok, 16, IPC_CREAT | 0666 );
	if( semd==-1 )
	{
		perror( "error semget" );
		return 1;
	}
	printf( "Semaphores is ready. Setting values\n" );
	for( res=0; res<NUMSEM; res++ )
		values[res] = res;
	res = semctl( semd, 0, SETALL, values );
	if( res==-1 )
	{
		perror( "error semctl" );
		return 1;
	}
	printf( "Semaphor values successfully setted\n" );
	// for( res=0; res<NUMSEM; res++ )
		// values[res] = 0;
	// for( int i=0; i<NUMSEM; i++ )
	// {
		// res = semctl( semd, i, GETVAL, NULL );
		// printf( "Semaphor value[%d]=%d\n", i, res );
	// }
	return 0;
}
