#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <memory.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_MEMSIZE (1024*1024)	// 1 Мб 
#define SET_CONST 13

int main(int argc, char **argv)
{
	int shm_d;	// дескриптор общей памяти
	char* mem;	// указатель для общей памяти
	int res;
	
	shm_unlink("/test.shm");
	shm_d = shm_open("/test.shm", O_RDWR | O_CREAT, 0666);
	if( shm_d==-1 )
	{
		perror( "error shm_open" );
		return 1;
	}
	if( ftruncate(shm_d, SHM_MEMSIZE) == -1 ) 
	{
		perror( "error ftruncate" );
		return 1;
	}	
	printf( "Shared memory ready for use\n" );
	mem = (char*) mmap( 0, SHM_MEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_d, 0 );
	if( mem==MAP_FAILED )
	{
		perror( "error mmap" );
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
		// int shm_d2; // дескриптор разделяемой памяти для потомка
		
		// shm_d2 = shm_open("/test.shm", O_RDONLY, 0666);
		// if( shm_d2==-1 )
		// {
			// perror( "error child shm_open" );
			// return 1;
		// }
		// printf( "reading shared memory in child\n" );
		// ch = (char*) mmap( 0, SHM_MEMSIZE, PROT_READ, MAP_SHARED, shm_d2, 0 );
		// if( ch==MAP_FAILED )
		// {
			// perror( "error mmap in child" );
			// return 1;
		// }
		// for( int i=0; i<SHM_MEMSIZE; i++ )
			// if( ch[i]!=SET_CONST )
			// {
				// printf( "Incorrect value at offset=%d:%d\n", i, ch[i] );
				// break;
			// }
		// printf( "Done\n" );

	// }
	sleep( 50 );
	printf( "Destroing shared memory\n" );
	shm_unlink("/test.shm");	// убиваем файл. 
	
	return 0;
}
