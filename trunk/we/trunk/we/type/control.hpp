// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONTROL_HPP
#define _WE_TYPE_CONTROL_HPP

#include <iostream>

struct control 
{
  friend std::ostream & operator << (std::ostream &, const control &);
};

inline bool operator == (const control &, const control &) { return true; }

std::ostream & operator << (std::ostream & s, const control &)
{
  return s << "[]";
}

#endif
