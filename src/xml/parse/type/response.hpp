#pragma once

#include <xml/parse/type/response.fwd.hpp>

#include <xml/parse/id/generic.hpp>
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
        ID_SIGNATURES (response);
        PARENT_SIGNATURES (transition);

      public:
        //! \note port
        typedef std::string unique_key_type;

        response_type ( ID_CONS_PARAM (response)
                      , PARENT_CONS_PARAM (transition)
                      , util::position_type const&
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

        id::ref::response clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _port;
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
