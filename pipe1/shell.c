#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
//#include <sys/stat.h>

#define BUFLEN 1024     // длина входного буфера
#define DELIM_CHAR '|'  // символ - разделитель команд

int main()
{
	char buf[BUFLEN];
    char **argv = NULL; // list of pointers to argvs(лексемы) in input buf
    unsigned *command;   // список номеров начала команд среди списка argv(лексем). Команда завершается NULL-лексемой
    unsigned argc;	// number of argv, min=1
    unsigned numcom;     // number of commands in list
    unsigned i, j;     // counter
    char *ch;
	
    fgets( buf, BUFLEN, stdin );
//    strcpy( buf, "who | sort | uniq -c | sort -nk1");
//    printf( "%s\n", buf );
// считаем лексемы в строке из буфера
// [<пробелы>]<лексема><пробелы><лексема><DELIM_CHAR><лексема>...<пробелы><лексема>[<пробелы>]'\0'
    ch = buf;
    argc = 0;
    while( *ch && isspace(*ch) ) ch++; // пропуск пробелов, если есть
    if( !ch )
    {
        printf( "Empty argv line! Nothing to do" );
        return -1;
    }
    if( *ch==DELIM_CHAR )
    {
        printf( "Invalid command. Unexpected symbol \'%c\'", DELIM_CHAR );
        return -1;
    }
    do {
        argc++;
        while( *ch && !isspace(*ch) && *ch!=DELIM_CHAR ) ch++; // пропуск лексемы
        while( *ch && (isspace(*ch) || *ch==DELIM_CHAR) ) ch++; // пропуск пробелов
    } while (*ch);
// считаем команды
    ch = buf;
    numcom = 0;
    int lexem = 1;  // есть ли лексема между разделителями. Одна есть - мы уже проверяли
    while( *ch && isspace(*ch) ) ch++; // пропуск пробелов, если есть
    do {
        while( *ch && *ch!=DELIM_CHAR ) // пропуск символов до разделителя
        {
            if( !isspace(*ch) ) lexem=1;
            ch++;
        }
        if( lexem ) // если между разделителями есть лексемы, то прибавляем команду
        {
            numcom++;
            lexem = 0;
        }
        if( *ch == DELIM_CHAR ) ch++;
    } while (*ch);
// создаем массив команд
    command = (unsigned*) malloc( numcom*sizeof(unsigned) );
// создаем массив ссылок на лексемы, на numcom больше чтобы в конец каждой команды добавить NULL-лексему
    argv = (char**) malloc( (argc+numcom)*sizeof(char*) );

// заполняем массивы лексем и команд
    ch = buf;
    i = j = 0;  // i - счетчик лексем, j - счетчик команд
    lexem = 0;
    command[0] = 0;	// первая команда = первая лексема
    do {
        while( *ch && isspace(*ch) ) ch++; // пропуск пробелов, если есть
        if( *ch && *ch != DELIM_CHAR )
        {
            argv[i++] = ch;
            lexem = 1;
        }
        while( *ch && !isspace(*ch) && *ch!=DELIM_CHAR ) ch++;  // пропуск лексемы
        if( *ch == DELIM_CHAR ) // разделитель команд
        {
            if( lexem )
            {
                j++; i++;
                command[j] = i;
                argv[i] = NULL; // добавляем NULL лексему в конец команды
                lexem = 0;
            }
            *ch = '\0'; // закрываем лексему
            ch++;   // идем дальше
        }
        else if( isspace(*ch) )
        {
            *ch = '\0'; // закрываем лексему
            ch++;   // идем дальше
        }
    } while (*ch);
    argv[i] = NULL; // добавляем NULL лексему в конец команды

    // for( i=0; i<argc+numcom; i++ )
        // printf( "argv[%d]=%s\n", i, argv[i] );
	printf( "numcom = %d\n", numcom );
    for( i=0; i<numcom; i++ )
		printf( "i=%d command[%d]=%s %s\n", i, i, *(argv+command[i]), *(argv+command[i]+1) );

	int p;
	int *pfd = (int*) malloc( 2*numcom*sizeof(int) );

	for( i=0; i<numcom-1; i++ )
	{
		j = i*2;
		pipe( pfd+j );
		if( !fork() )
		{   // child
			close( STDOUT_FILENO );
			dup2( pfd[j+1], STDOUT_FILENO );
			close( pfd[j+1] );
			close( pfd[j] );
			execvp( argv[command[i]], argv+command[i] );
		}
		else	// parent
		{
			close( STDIN_FILENO );
			dup2( pfd[j], STDIN_FILENO );
			close( pfd[j] );
			close( pfd[j+1] );
			wait( &p );

		}
	}
//	if( i==numcom-1 )
	{
//    int fd_out = open( "/home/box/result.out", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
		// int fd_out = open( "/home/eugene/pipe1/result.out", O_RDWR | O_CREAT | O_TRUNC, 0666 );
		// close( STDOUT_FILENO );
		// dup2( fd_out, STDOUT_FILENO );
		// close( fd_out );
		execvp( argv[command[numcom-1]], argv+command[numcom-1] );
	}

    free( argv );
    free( command );
    return 0;
}
