#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> 
#include <unistd.h>
#include <string.h>
#include <wait.h>

#define MAXBUF 1024

main( int argc, char** argv )
{
	char buf[MAXBUF];
	
	// unlink("/home/eugene/pipe2/in.fifo"); 
	// unlink("/home/eugene/pipe2/out.fifo"); 
	int fifo_in = mkfifo( "/home/eugene/pipe2/in.fifo", O_RDWR );
//	int fifo_in = mkfifo( "/home/box/in.fifo", O_RDWR | O_CREAT | O_TRUNC );
	int fifo_out = mkfifo( "/home/eugene/pipe2/out.fifo", O_RDWR );
//	int fifo_out = mkfifo( "/home/box/out.fifo", O_RDWR | O_CREAT | O_TRUNC );
	if( (fifo_in==-1 || fifo_out==-1) && errno!=EEXIST )
	{
		printf( "Error creat fifo\n");
		return 1;
	}
//	int fd_out = open( "/home/eugene/pipe2/out.fifo", O_RDWR );	
	int fd_in = open( "/home/eugene/pipe2/in.fifo", O_RDONLY | O_NONBLOCK );	
//	int fd_in = open( "/home/box/in.fifo", O_RDONLY );	
	if( fd_in<0 )
	{
		printf( "Error open fifo in\n");
		return 1;
	}	
	
	int fd_out = open( "/home/eugene/pipe2/out.fifo", O_RDWR, 0666 );
//	int fd_out = open( "/home/box/out.fifo", O_WRONLY );	
	if( fd_out<0 )
	{
		printf( "Error open fifo out\n");
		return 1;
	}

	printf( "Ready to copy\n" );
	int p;
	while( 1 )
	{	
		int len = read( fd_in, buf, MAXBUF );
		buf[len] = '\0';
		if( len )
		{
			printf( "len=%d, %s\n", len, buf );
			write( fd_out, buf, len );
		}
	}
	close( fd_in );
	close( fd_out );
	
	return 0;
}
