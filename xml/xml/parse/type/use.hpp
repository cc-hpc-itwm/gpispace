// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_USE_HPP
#define _XML_PARSE_TYPE_USE_HPP

#include <xml/parse/id/generic.hpp>

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
        ID_SIGNATURES(use);
        PARENT_SIGNATURES(transition);

      public:
        use_type ( ID_CONS_PARAM(use)
                 , PARENT_CONS_PARAM(transition)
                 , const util::position_type&
                 , const std::string& name
                 );

        const std::string& name() const;

        id::ref::use clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

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

#endif
