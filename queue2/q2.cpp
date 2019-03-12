#include <mqueue.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFSIZE 80

int main(int argc, char **argv)
{
	int fd;		// декскриптор выходного файла
	mqd_t qfd;	// дескриптор очереди
	char buf[BUFSIZE];
	struct mq_attr at, old_at;
	int res;
	
	fd = open( "/home/box/message.txt", O_RDWR | O_CREAT | O_TRUNC, 0666 );
//	fd = open( "/home/eugene/queue2/message.txt", O_RDWR | O_CREAT | O_TRUNC, 0666 );
	if( fd==-1 )
	{
		perror( "error open output" );
		return 1;
	}
	if(mq_unlink("/test.mq") == 0)
		printf( "Message queue %s removed from system.\n", "/test.mq");
	at.mq_msgsize = BUFSIZE;
	qfd = mq_open( "/test.mq", O_RDWR | O_CREAT, 0666, &at );
	if( qfd==-1 )
	{
		perror( "error mq_open" );
		return 1;
	}
	res = mq_getattr( qfd, &at );
	if( res==-1 )
	{
		perror( "error mq_getattr" );
		return 1;
	}
	printf( "Queue msg_size=%ld\n", at.mq_msgsize );
	printf( "Queue is ready\n" );
	// int pid = fork();
	// if( pid )	// для отладки создаем 2 потока
	// {	// parent - тут пишем
		// while( 1 )
		// {
			// printf( "Ready to send message to queue...\n" );
			// fgets( buf, BUFSIZE, stdin );
			// res = strlen( buf );
			// buf[res] = '\0'; // обрезаем мусор
			// if( !strncmp(buf, "exit", 4 ) )
				// break;
			// printf( "Sending to queue\n%s", buf );		
			// res = mq_send( qfd, buf, res, 0 );
			// if( res==-1 )
			// {
				// perror("error sending to queue" );
				// return 1;
			// }
		// }
		// printf( "Exit in parent\n" );
		// mq_unlink( "/test.mq" );
		// kill( pid, SIGTERM );
		// wait( &res );
	// }
	// else
	{	// child	- тут читаем
		while( 1 )
		{
			printf( "Reading message from queue...\n" );
			int res = mq_receive( qfd, buf, BUFSIZE, NULL );
			if( res==-1 )
			{
				perror("error reading from queue" );
				return 1;
			}
			buf[res] = '\0'; 	// обрезаем мусор
			printf( "Read from queue\n%s", buf );
			if( !strncmp(buf, "exit", 4 ) )
				break;
			printf( "Write to output...\n" );
			write( fd, buf, res );
		}
		mq_unlink( "/test.mq" );
//		mq_close( qfd );
	}
	return 0;
}
