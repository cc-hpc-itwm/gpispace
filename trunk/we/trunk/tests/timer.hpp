// simple timer, mirko.rahn@itwm.fraunhofer.de

#ifndef _TIMER_H
#define _TIMER_H

#include <we/util/stat.hpp>

#include <iostream>

#include <string>

struct Timer_t
{
private:
  double t;
  const std::string msg;
  const unsigned int k;
  std::ostream & s;
public:
  explicit Timer_t ( const std::string & _msg
                   , const unsigned int & _k = 1
                   , std::ostream & _s = std::cout
                   )
    : t(-statistic::current_time())
    , msg(_msg)
    , k(_k)
    , s(_s)
  {}

  ~Timer_t ()
  {
    t += statistic::current_time();

    s << "time " << msg
      << " [" << k << "]: "
      << t
      << " [" << t / double(k) << "]"
      << " [" << double(k) / t << "]"
      << std::endl;
  }
};

#endif // _TIMER_H
