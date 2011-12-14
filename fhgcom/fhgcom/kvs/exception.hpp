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
        struct no_such : public std::runtime_error
        {
          explicit
          no_such (std::string const & name)
            : std::runtime_error ("no_such: " + name)
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
