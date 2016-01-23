// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct use_type : with_position_of_definition
      {
      public:
        use_type ( const util::position_type&
                 , const std::string& name
                 );

        const std::string& name() const;

      private:
        std::string _name;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const use_type& u);
      }
    }
  }
}
