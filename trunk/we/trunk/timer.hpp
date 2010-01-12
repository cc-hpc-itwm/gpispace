// simple timer, mirko.rahn@itwm.fraunhofer.de

#ifndef _TIMER_H
#define _TIMER_H

#include <sys/time.h>
#include <string>
#include <ostream>

static inline double current_time()
{
  struct timeval tv;

  gettimeofday (&tv, NULL);

  return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
}

struct Timer_t
{
private:
  double t;
  const std::string msg;
  const unsigned int k;
  std::ostream & s;
public:
  Timer_t ( const std::string & _msg
          , const unsigned int & _k = 1
          , std::ostream & _s = std::cout
          )
    : t(-current_time())
    , msg(_msg)
    , k(_k)
    , s(_s)
  {}

  ~Timer_t ()
  {
    t += current_time();

    s << "time " << msg 
      << " [" << k << "]: " 
      << t
      << " [" << t / double(k) << "]"
      << " [" << double(k) / t << "]"
      << std::endl;
  }
};

#endif // _TIMER_H
