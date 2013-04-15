// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_EXCEPTION_HPP
#define PNET_SRC_WE_TYPE_VALUE_EXCEPTION_HPP

#include <we/type/value.hpp>
#include <we/type/value/signature/signature.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace exception
      {
        class type_error : public std::runtime_error
        {
        public:
          type_error (const std::string&);
          type_error (const boost::format&);
          ~type_error() throw() {}
        };

        class type_mismatch : public type_error
        {
        public:
          type_mismatch ( const signature_type&
                        , const value_type&
                        , const std::list<std::string>&
                        );
          ~type_mismatch() throw() {}
        };

        class missing_field : public type_error
        {
        public:
          missing_field ( const signature_type&
                        , const std::list<std::string>&
                        );
          ~missing_field() throw() {}
        };

        class unknown_field : public type_error
        {
        public:
          unknown_field ( const value_type&
                        , const std::list<std::string>&
                        );
          ~unknown_field() throw() {}
        };
      }
    }
  }
}

#endif
