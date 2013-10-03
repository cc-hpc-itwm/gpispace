// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_EXCEPTION_HPP
#define PNET_SRC_WE_TYPE_VALUE_EXCEPTION_HPP

#include <we/type/value.hpp>
#include <we/type/signature.hpp>

#include <we/expr/token/type.hpp>

#include <list>
#include <stdexcept>

namespace pnet
{
  namespace exception
  {
#define MEMBER(_name,_type)                             \
    public:                                             \
      const _type& _name() const { return _ ## _name; } \
    private:                                            \
      _type _ ## _name

    class type_error : public std::runtime_error
    {
    public:
      type_error (const std::string&);
      ~type_error() throw() {}
    };

    class type_mismatch : public type_error
    {
    public:
      type_mismatch ( const type::signature::signature_type&
                    , const type::value::value_type&
                    , const std::list<std::string>&
                    );
      ~type_mismatch() throw() {}

      MEMBER (signature, type::signature::signature_type);
      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class missing_field : public type_error
    {
    public:
      missing_field ( const type::signature::signature_type&
                    , const type::value::value_type&
                    , const std::list<std::string>&
                    );
      ~missing_field() throw() {}

      MEMBER (signature, type::signature::signature_type);
      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class unknown_field : public type_error
    {
    public:
      unknown_field ( const type::value::value_type&
                    , const std::list<std::string>&
                    );
      ~unknown_field() throw() {}

      MEMBER (value, type::value::value_type);
      MEMBER (path, std::list<std::string>);
    };

    class eval : public type_error
    {
    public:
      eval (const ::expr::token::type&, const type::value::value_type&);
      eval ( const ::expr::token::type&
           , const type::value::value_type&
           , const type::value::value_type&
           );
      ~eval() throw() {}

      MEMBER (token, ::expr::token::type);
      MEMBER (values, std::list<type::value::value_type>);
    };

    class missing_binding : public std::runtime_error
    {
    public:
      missing_binding (const std::string&);
      ~missing_binding() throw() {}

      MEMBER (key, std::string);
    };

    class could_not_resolve : public std::runtime_error
    {
    public:
      could_not_resolve (const std::string&, const std::list<std::string>&);
      ~could_not_resolve() throw() {}

      MEMBER (type, std::string);
      MEMBER (path, std::list<std::string>);
    };

#undef MEMBER
  }
}

#endif
