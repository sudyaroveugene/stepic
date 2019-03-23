#include <stdio.h>
#include <fcntl.h>
#include <thread>
#include <omp.h>
#include <string.h>
#include <time.h>

#define NUMTHREADS 4	

static pthread_spinlock_t spin;	// спин мьютекс
static int *a;			// массив для сортировки
static size_t len;		// размер исходного массива
static struct mas_struct
{				//	структура для частей исходного массива
	int *a;
    size_t len;
} massiv[NUMTHREADS];	// набор частей исходного массива

// функция сравнения целых чисел для сортировки
int compare_ints(const void* a, const void* b)
{

    const int arg1 = *(static_cast<const int*>( a ));
    const int arg2 = *(static_cast<const int*>( b ));

    return arg1 - arg2;
}

int main(int argc, char **argv)
{
    FILE *fd_in, *fd_out;		// дескрипторы входного и выходного файла
    int res, num;
    size_t i;
    std::string file_in, file_out;
	clock_t start, finish;
	
// инициализируем spin_lock - для него нет статической инициализации
	res = pthread_spin_init( &spin, PTHREAD_PROCESS_PRIVATE );
	if( res )
	{
		perror( "error init spin_lock" );
		return 1;
	}

// открываем указанные в параметрах файлы ввода-вывода
	switch (argc )
	{
        case 1:     // имена файлов не указаны, используем умолчания
            printf( "Using %s input_file output_file to sets input/output files\n", argv[0]);
            printf( "By default input_file=\"data.in\", output_file=\"data.out\"\n");
            file_in.append( "data.in" );
            file_out.append( "data.out" );
            break;
        case 2:     // указан только входной файл
            file_in.append( argv[1] );
            printf( "Using \"data.out\" as output_file\n");
            file_out.append( "data.out" );
            break;
        default:
            file_in.append( argv[1] );
            file_out.append( argv[2] );
	}
    fd_in = fopen( file_in.data(), "r" );
    if( !fd_in )
	{
        perror( "error open input file" );
		return 1;
	}
    fd_out = fopen( file_out.data(), "w" );
    if( !fd_out )
    {
        perror( "error open output file" );
        return 1;
    }

// читаем данные из входного файла
    printf( "reading from file %s\n", file_in.data() );
    len = 0;
    do
    {
        res = fscanf( fd_in, "%d",  &num );
        if( res>0 )
        {
            a = static_cast<int*>(realloc( a, (len+1)*sizeof(int)) );
            if( !a )
            {
                perror( "error realloc memory for input data" );
                return 1;
            }
            a[len] = num;
            len++;
        }
    } while( res!=EOF );
    fclose( fd_in );

// генерируем массив из случайных чисел
   // printf( "generation random integers\n" );
   // srand( 12345 );
   // for( i=0; i<1000000; i++ )
   // {
       // len++;
       // a = static_cast<int*>(realloc( a, len*sizeof(int)) );
       // if( !a )
       // {
           // perror( "error realloc memory for input data" );
           // return 1;
       // }
       // num = (int) (((double) rand()/RAND_MAX-0.5)*(-2000)); // от -1000 до +1000
       // a[i] = num;
    // }

// разбиваем массив на части по числу потоков

	start = clock();	// запускаем секундомер

    massiv[0].a = a;
    size_t curlen = 0;
    for( size_t k=0; k<NUMTHREADS; k++ )
    {
        massiv[k].len = (len - curlen)/(NUMTHREADS - k);    // длина к-го массива, подгоняем чтобы куски были равномерные
        curlen += massiv[k].len;
        if( k>0 )
            massiv[k].a = massiv[k-1].a+massiv[k-1].len;    // указатель на начало к-го массива
    }

// параллельно сортируем все массивы
#pragma omp parallel for num_threads( NUMTHREADS )
    for( int k=0; k<NUMTHREADS; k++ )
    {
        qsort( massiv[k].a, massiv[k].len, sizeof(int), compare_ints );
//        printf( "parallel thread\n");
    }

    for( int num_massivs = NUMTHREADS; num_massivs>1; num_massivs = (num_massivs+1)/2 )
	{
        // попарно объединяем подряд идущие массивы
#pragma omp parallel for //num_threads( 2 )
        for( int k=0; k<num_massivs-1; k+=2 )
        {
            int l=k+1;
            size_t new_len = massiv[k].len+massiv[l].len;   //  размер объединенного массива
            int *new_a=static_cast<int*>( malloc(new_len*sizeof(int)) );    // выделяем память под объединенный массив
//            printf( "k=%d, massiv[k].len=%u, l=%d, massiv[l].len=%u, new_len=%u\n", k, massiv[k].len, l, massiv[l].len, new_len );
            for( size_t i=0, j=0, ij=0; ij<new_len; ij++ )
            {
                if( i==massiv[k].len )  // первый массив кончился
                {
                    new_a[ij] = massiv[l].a[j];     // добиваем вторым массивом
                    j++;
                }
                else if( j==massiv[l].len )     // второй массив кончился
                {
                    new_a[ij] = massiv[k].a[i];     // добиваем первым массивом
                    i++;
                }
                else if( massiv[k].a[i]<massiv[l].a[j] )
                {
                    new_a[ij] = massiv[k].a[i];
                    i++;
                }
                else {
                    new_a[ij] = massiv[l].a[j];
                    j++;
                }
            }
            memcpy( massiv[k].a, new_a, new_len*sizeof(int) );  // копируем объединенный массив на место первого
            free( new_a );
            massiv[k].len = new_len;    // меняем размер нового массива
//            printf( "k=%d array`s merge \n", k );
        }
        for( int k=0; k<num_massivs-1; k+=2 )   // убираем из списка массивов объединенные с предыдущим
        {
            int l = k/2;
            massiv[l].a = massiv[k].a;
            massiv[l].len = massiv[k].len;
        }
        if( num_massivs%2 )	// есть "лишний" массив (нечетное число массивов)
		{            
            int l = (num_massivs-1)/2;
            massiv[l].a = massiv[num_massivs-1].a;      // меняем указатель нового массива
            massiv[l].len = massiv[num_massivs-1].len;    // меняем размер нового массива
        }
    }

	finish = clock();	// останавливаем секундомер
	
// выводим отсортированный массив в выходной файл 
    printf( "writing to file %s\n", file_out.data() );
	for( i=0; i<len; i++ )
	{
        if( fprintf( fd_out, "%d ", a[i])<0 )
		{
			perror( "error writing output file" );
			return 1;
		}
	}
    fprintf( fd_out, "\n" );	// добавляем в конец файла перевод строки
    printf( "Done\n" );
	printf( "elapsed %f ms\n", (finish-start)/1000. );
    fclose( fd_out );
	
	
	
    pthread_spin_destroy( &spin );
	free( a );
	return 0;
}
