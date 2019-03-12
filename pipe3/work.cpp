#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
	ssize_t	size;
	struct msghdr msg;
	struct iovec  iov;
	union {
		struct	cmsghdr cmsghdr;
		char	control[CMSG_SPACE(sizeof(int))];
	} cmsgu;
	struct cmsghdr  *cmsg;

	iov.iov_base = buf;
	iov.iov_len = buflen;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if( fd!=-1 ) 
	{
		msg.msg_control = cmsgu.control;
		msg.msg_controllen = sizeof(cmsgu.control);

		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;

		printf( "passing fd %d\n", fd );
		*((int *) CMSG_DATA(cmsg)) = fd;
	} 
	else 
	{
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		printf ("not passing fd\n");
	}

	size = sendmsg(sock, &msg, 0);

	if( size<0 )
		perror ("error sendmsg");
	return size;
}

ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
	ssize_t size;

	if( fd ) 
	{
		struct msghdr   msg;
		struct iovec    iov;
		union {
			struct cmsghdr cmsghdr;
			char   control[CMSG_SPACE(sizeof (int))];
		} cmsgu;
		struct cmsghdr *cmsg;

		iov.iov_base = buf;
		iov.iov_len = bufsize;

		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = cmsgu.control;
		msg.msg_controllen = sizeof(cmsgu.control);
		size = recvmsg(sock, &msg, 0);
		if( size<0 ) 
		{
			perror ("error recvmsg");
			exit(1);
		}
		cmsg = CMSG_FIRSTHDR(&msg);
		if( cmsg && cmsg->cmsg_len==CMSG_LEN(sizeof(int)) ) 
		{
			if( cmsg->cmsg_level!=SOL_SOCKET ) 
			{
				fprintf( stderr, "invalid cmsg_level %d\n", cmsg->cmsg_level);
				exit(1);
			}
			if( cmsg->cmsg_type!=SCM_RIGHTS ) 
			{
				fprintf( stderr, "invalid cmsg_type %d\n", cmsg->cmsg_type );
				exit(1);
			}
			*fd = *((int *) CMSG_DATA(cmsg));
			printf( "received fd %d\n", *fd );
		} 
		else
			*fd = -1;
	} 
	else 
	{
		size = read(sock, buf, bufsize);
		if (size < 0)
		{
			perror("error read");
			exit(1);
		}
	}
	return size;
}

void child(int sock)
{
	int fd;
	char buf[80];
	ssize_t size;

	sleep(2);
	while( 1 )
	{
		size = sock_fd_read(sock, buf, sizeof(buf), &fd);
		if( size<=0 )
			break;
		// закрываем прочитанную строку, а то в ней могут остаться левые символы
		buf[size] = '\0';  
		printf( "read %d bytes: %s\n", size, buf );
		if(fd != -1) 
		{
			printf( "write reading bytes to fd=%d\n", fd );
			write(fd, buf, size );
			close(fd);
		}
	}
}

void parent(int sock)
{
	ssize_t size;
	int i, len;
	int fd;
	char buf[20];

	fd = open( "/home/eugene/pipe3/out", O_RDWR | O_CREAT | O_TRUNC, 0666 );
	printf( "open file \"out\" fd=%d\n", fd );
	strcpy( buf, "Hello, world!\n" );
	len = strlen( buf );
	size = sock_fd_write(sock, buf, len, fd);
	for( i=1; ; i++ )
	{
		sprintf( buf, "%d\n", i );
		len = strlen( buf );
		size = sock_fd_write(sock, buf, len, -1);
		printf ("wrote %d bytes with fd=%d\n", size, fd);
		if( i==1000 )
			i=0;
	}
}

int main(int argc, char **argv)
{
	int sv[2];
	int pid;

	if( socketpair(AF_LOCAL, SOCK_STREAM, 0, sv)<0 ) 
	{
		perror("error socketpair");
		exit(1);
	}
	if( fork() ) 
	{	// parent
		close(sv[1]);
		parent(sv[0]);
	}
	else	//child
	{
		close(sv[0]);
		child(sv[1]);
	}
	return 0;
}
