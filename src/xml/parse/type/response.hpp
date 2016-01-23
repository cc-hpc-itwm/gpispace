#pragma once

#include <xml/parse/type/response.fwd.hpp>

#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>

#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct response_type : with_position_of_definition
      {
      public:
        //! \note port
        using unique_key_type = std::string;

        response_type ( util::position_type const&
                      , std::string const& port
                      , std::string const& to
                      , we::type::property::type const& = {}
                      );

        std::string const& port() const { return _port; }
        std::string const& to() const { return _to; }

        we::type::property::type const& properties() const
        {
          return _properties;
        }

        unique_key_type unique_key() const
        {
          return _port;
        }

      private:
        std::string const _port;
        std::string _to;
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, response_type const&);
      }
    }
  }
}
