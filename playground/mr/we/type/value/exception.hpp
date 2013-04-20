// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_EXCEPTION_HPP
#define PNET_SRC_WE_TYPE_VALUE_EXCEPTION_HPP

#include <we/type/value.hpp>
#include <we/type/value/signature.hpp>

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
#define MEMBER(_name,_type)                               \
      public:                                             \
        const _type& _name() const { return _ ## _name; } \
      private:                                            \
        const _type _ ## _name

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

          MEMBER (signature, signature_type);
          MEMBER (value, value_type);
          MEMBER (path, std::list<std::string>);
        };

        class missing_field : public type_error
        {
        public:
          missing_field ( const signature_type&
                        , const std::list<std::string>&
                        );
          ~missing_field() throw() {}

          MEMBER (signature, signature_type);
          MEMBER (path, std::list<std::string>);
        };

        class unknown_field : public type_error
        {
        public:
          unknown_field ( const value_type&
                        , const std::list<std::string>&
                        );
          ~unknown_field() throw() {}

          MEMBER (value, value_type);
          MEMBER (path, std::list<std::string>);
        };

#undef MEMBER
      }
    }
  }
}

#endif
