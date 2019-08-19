#pragma once

#include <we/type/value.hpp>

#include <iosfwd>
#include <list>
#include <string>
#include <unordered_map>

namespace expr
{
  namespace eval
  {
    struct context
    {
    private:
      typedef std::unordered_map< std::string
                                  , pnet::type::value::value_type
                                  > container_type;

      container_type _container;

      typedef std::unordered_map< std::string
                                  , const pnet::type::value::value_type*
                                  > ref_container_type;

      ref_container_type _ref_container;

      friend std::ostream& operator<< (std::ostream&, context const&);

    public:
      void bind_ref (const std::string&, const pnet::type::value::value_type&);

      void bind_and_discard_ref ( const std::list<std::string>&
                                , const pnet::type::value::value_type&
                                );

      const pnet::type::value::value_type& value (const std::list<std::string>&) const;
    };
  }
}
