#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int fd;		// декскриптор выходного файла
	int qfd;	// дескриптор очереди
	struct message {
		long mtype;
		char mtext[80];
	} msg;
	
	key_t tok = ftok( "/tmp/msg.temp", 1 );

//	fd = open( "/home/box/message.txt", O_RDWR | O_CREAT | O_TRUNC, 0666 );
	fd = open( "/home/eugene/queue1/message.txt", O_RDWR | O_CREAT | O_TRUNC, 0666 );
	if( fd==-1 )
	{
		perror( "error open output" );
		return 1;
	}
	qfd = msgget( tok, IPC_CREAT | 0666 );
	if( qfd==-1 )
	{
		perror( "error msgget" );
		return 1;
	}
	printf( "Queue is ready\n" );
	// int pid = fork();
	// if( pid )	// для отладки создаем 2 потока
	// {	// parent - тут пишем
		// char buf[80];
		// int res;

		// while( 1 )
		// {
			// printf( "Ready to send message to queue...\n" );
			// fgets( buf, 80, stdin );
			// strcpy( msg.mtext, buf );
			// res = strlen( buf );
			// msg.mtext[res-1] = '\0'; // обрезаем перевод строки
			// if( !strcmp(msg.mtext, "exit" ) )
				// break;
			// printf( "Sending \"%s\" to queue...\n", msg.mtext );		
			// msg.mtype = 1;
			// res = msgsnd( qfd, &msg, strlen(msg.mtext), 0 );
			// if( res==-1 )
			// {
				// perror("error sending to queue" );
				// return 1;
			// }
		// }
		// printf( "Exit in parent\n" );
		// msgctl( qfd, IPC_RMID, NULL );
		// kill( pid, SIGTERM );
		// wait( &res );
	// }
	// else
	{	// child	- тут читаем
		while( 1 )
		{
			printf( "Reading message from queue...\n" );
			int res = msgrcv( qfd, &msg, 80, 0, 0 );
			if( res==-1 )
			{
				perror("error reading from queue" );
				return 1;
			}
			msg.mtext[res] = '\0'; 	// обрезаем мусор
			printf( "Read \"%s\" from queue\n", msg.mtext );
			// if( !strcmp(msg.mtext, "exit" ) )
				// break;
			printf( "Write to output...\n" );
			write( fd, msg.mtext, strlen(msg.mtext) );
		}
		msgctl( qfd, IPC_RMID, NULL );
	}
	return 0;
}
