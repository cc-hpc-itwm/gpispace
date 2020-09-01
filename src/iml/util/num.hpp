#pragma once

#include <iml/util/parse/position.hpp>

#include <boost/variant.hpp>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      unsigned long read_ulong (parse::position&);
      unsigned int read_uint (parse::position&);
      std::size_t read_size_t (parse::position&);
      long read_long (parse::position&);
      int read_int (parse::position&);
      double read_double (parse::position&);
      float read_float (parse::position&);

      typedef boost::variant< int
                            , long
                            , unsigned int
                            , unsigned long
                            , float
                            , double
                            > num_type;

      num_type read_num (parse::position&);

      int read_int (std::string const&);
      std::size_t read_size_t (std::string const&);
    }
  }
}
