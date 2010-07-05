#include <iostream>
#include <pthread.h>
#include <stdlib.h>

struct A
{
private:
  A()
  {
    std::cout << "A()" << std::endl;
  }
public:
  static A const & instance()
  {
    std::cout << "A::instance()" << std::endl;
    static A a;
    return a;
  }
};

void *f (void *)
{
  std::cout << "thread started" << std::endl;
  A a (A::instance());
  std::cout << "thread stopping" << std::endl;
  return NULL;
}

struct B
{
private:
  B ()
  {
    std::cout << "enter B()" << std::endl;
    pthread_t t;
    std::cout << "    " << "starting thread" << std::endl;
    pthread_create ( &t, NULL, &f, NULL );
    A a (A::instance());
    std::cout << "    " << "joining thread" << std::endl;
    pthread_join ( t, NULL );
    std::cout << "leave B()" << std::endl;
  }

public:
  static B const & instance ()
  {
    std::cout << "B::instance()" << std::endl;
    static B b;
    return b;
  }
};

int main ()
{
  std::cout << "enter main()" << std::endl;
  B b (B::instance());
  std::cout << "leave main()" << std::endl;
  return EXIT_SUCCESS;
}

