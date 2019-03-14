#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <memory.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHM_MEMSIZE (1024*1024*1024)	// 1 Мб 
#define SET_CONST 42

int main(int argc, char **argv)
{
	int shm_d;	// дескриптор общей памяти
	char* mem;	// указатель для общей памяти
	int res;
	
	key_t tok = ftok( "/tmp/mem.temp", 1 );

	shm_d = shmget( tok, SHM_MEMSIZE, IPC_CREAT | 0666 );
	if( shm_d==-1 )
	{
		perror( "error shmget" );
		return 1;
	}
	printf( "Shared memory ready for use\n" );
	mem = (char*) shmat( shm_d, NULL, 0 );
	if( (int)mem==-1 )
	{
		perror( "error shmat" );
		return 1;
	}
	printf( "Setting memory to %d\n", SET_CONST );
	memset( mem, SET_CONST, SHM_MEMSIZE );
	printf( "Done\n" );

	// int pid = fork();	// для отладки создаем 2 потока
	// if( pid )	
	// {	// parent - тут пишем
		// printf( "Wait...\n" );
		// wait( NULL );
	// }
	// else
	// {	// child	- тут читаем
		// char* ch;
		// printf( "reading shared memory in child\n" );
		// sleep( 1 );
		// ch = (char*) shmat( shm_d, NULL, 0 );
		// if( (int)ch==-1 )
		// {
			// perror( "error shmat" );
			// return 1;
		// }
		// for( int i=0; i<SHM_MEMSIZE; i++ )
			// if( ch[i]!=SET_CONST )
				// printf( "offset=%d:%d", i, ch[i] );
		// printf( "Done\n" );

	// }
	sleep( 100 );
	printf( "Destroing shared memory\n" );
	res = shmctl( shm_d, IPC_RMID, NULL );
	if( res==-1 )
	{
		perror( "error shmctl" );
		return 1;
	}
	return 0;
}
