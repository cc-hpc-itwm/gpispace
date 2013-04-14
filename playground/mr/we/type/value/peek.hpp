// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_PEEK_HPP
#define PNET_SRC_WE_TYPE_VALUE_PEEK_HPP

#include <we/type/value.hpp>

#include <boost/optional.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      boost::optional<const value_type&>
      peek ( const std::list<std::string>::const_iterator&
           , const std::list<std::string>::const_iterator&
           , const value_type&
           );
      boost::optional<const value_type&>
      peek (const std::list<std::string>& path, const value_type& node);
      boost::optional<const value_type&>
      peek (const std::string&, const value_type&);

      boost::optional<value_type&>
      peek_ref ( const std::list<std::string>::const_iterator&
               , const std::list<std::string>::const_iterator&
               , value_type&
               );
      boost::optional<value_type&>
      peek_ref (const std::list<std::string>& path, value_type& node);
      boost::optional<value_type&>
      peek_ref (const std::string&, value_type&);
    }
  }
}

#endif
