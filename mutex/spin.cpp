#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>
#include <pthread.h>


pthread_mutex_t lock;
pthread_spinlock_t spin;

int use_spin = 1;
void f()
{
  volatile double d = 0;
  for(int n=0; n<10000; ++n) {

   // блокировка
   if(use_spin)
     pthread_spin_lock(&spin);
   else
     pthread_mutex_lock(&lock);

   for(int m=0; m<10000; ++m) {
       // или сюда блокировку
           d += d*n*m;
       // а сюда разблокировку
   }
// разблокировка   
   if(use_spin)
     pthread_spin_unlock(&spin);
   else
     pthread_mutex_unlock(&lock);

  }
}
 
int main()
{
  if (pthread_mutex_init(&lock, NULL) != 0) {
    std:: cerr <<"\n mutex init failed\n";
    return 1;
  }
  if (pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE) != 0) {
    std:: cerr <<"\n mutex init failed\n";
    return 1;
  }
  for(int e = 0; e < 2; e++) {
    use_spin = e;
    std::cout << "using " << (use_spin? "spin" : "mutex") << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    std::thread t1(f);
    std::thread t2(f); // f() is called on two threads
    // std::thread t3(f);
    // std::thread t4(f);
    t1.join();
    t2.join();
    // t3.join();
    // t4.join();
    std::clock_t c_end = std::clock();
    auto t_end = std::chrono::high_resolution_clock::now();
 
    std::cout << std::fixed << std::setprecision(2)
              << "Time passed: "
              << std::chrono::duration<double, std::milli>(t_end-t_start).count()
              << " ms\n";
  }
  pthread_mutex_destroy(&lock);
  pthread_spin_destroy(&spin);
}