// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_FIELD_HPP
#define PNET_SRC_WE_TYPE_VALUE_FIELD_HPP

#include <we/type/value.hpp>
#include <we/type/value/exception.hpp>
#include <we/type/value/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
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

      const value_type& field ( const path&
                              , const value_type&
                              , const signature_type&
                              );

      template<typename T>
      const T& field_as ( const path& p
                        , const value_type& v
                        , const signature_type& signature
                        )
      {
        const value_type& value (field (p, v, signature));

        const T* x (boost::get<const T> (&value));

        if (!x)
        {
          throw exception::type_mismatch (signature, value, p());
        }

        return *x;
      }
    }
  }
}

#endif
