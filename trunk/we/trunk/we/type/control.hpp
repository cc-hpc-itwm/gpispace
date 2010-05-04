// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONTROL_HPP
#define _WE_TYPE_CONTROL_HPP

#include <iostream>

#include <boost/serialization/nvp.hpp>

struct control 
{
  friend std::ostream & operator << (std::ostream &, const control &);
  friend bool operator == (const control &, const control &);

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & , const unsigned int)
  {
  }
};

inline bool operator == (const control &, const control &) { return true; }

inline std::ostream & operator << (std::ostream & s, const control &)
{
  return s << "[]";
}

#endif
