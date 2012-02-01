#ifndef FHG_COM_KVS_EXCEPTION_HPP
#define FHG_COM_KVS_EXCEPTION_HPP 1

#include <string>
#include <stdexcept>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace exception
      {
        struct generic : public std::runtime_error
        {
          explicit
          generic (std::string const & msg)
            : std::runtime_error (msg)
          {}

          virtual ~generic() throw () {}
        };

        struct no_such : public generic
        {
          explicit
          no_such (std::string const & name)
            : generic ("no_such: " + name)
            , name_(name)
          {}

          const std::string & name (void) const
          {
            return name_;
          }

          virtual ~no_such() throw () {}
        private:
          std::string name_;
        };
      }
    }
  }
}

#endif
