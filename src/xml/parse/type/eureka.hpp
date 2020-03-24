#pragma once

#include <xml/parse/type/eureka.fwd.hpp>

#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/eureka.hpp>

#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct eureka_type : with_position_of_definition
      {
      public:
        using unique_key_type = we::type::eureka_id_type;

        eureka_type ( util::position_type const&
                     , std::string const& port
                     );

        std::string const& port() const { return _port; }

        unique_key_type unique_key() const
        {
          return _port;
        }

      private:
        std::string const _port;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, eureka_type const&);
      }
    }
  }
}
