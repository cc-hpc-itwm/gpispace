// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_FIELD_HPP
#define PNET_SRC_WE_FIELD_HPP

#include <we2/exception.hpp>
#include <we2/type/signature.hpp>
#include <we2/type/value.hpp>

#include <list>
#include <string>

namespace pnet
{
  class path
  {
  public:
    path (const std::string&);
    ~path();

    const std::list<std::string>::const_iterator& key() const;
    const std::list<std::string>::const_iterator& end() const;
    const std::list<std::string>& operator() () const;

  private:
    const std::list<std::string>::const_iterator _key;
    const std::list<std::string>::const_iterator _end;
  };

  const type::value::value_type& field ( const path&
                                       , const type::value::value_type&
                                       , const type::signature::signature_type&
                                       );

  template<typename T>
    const T& field_as ( const path& p
                      , const type::value::value_type& v
                      , const type::signature::signature_type& signature
                      )
  {
    const type::value::value_type& value (field (p, v, signature));

    const T* x (boost::get<const T> (&value));

    if (!x)
    {
      throw exception::type_mismatch (signature, value, p());
    }

    return *x;
  }
}

#endif
