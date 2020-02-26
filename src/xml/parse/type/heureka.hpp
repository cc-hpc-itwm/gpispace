#pragma once

#include <xml/parse/type/heureka.fwd.hpp>

#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/heureka.hpp>

#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct heureka_type : with_position_of_definition
      {
      public:
        //! \note port
        using unique_key_type = we::type::heureka_id_type;

        heureka_type ( util::position_type const&
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
        void dump (::fhg::util::xml::xmlstream&, heureka_type const&);
      }
    }
  }
}
