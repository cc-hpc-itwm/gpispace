// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/signature.hpp>

#include <list>
#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/function.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct structure_type : with_position_of_definition
      {
      public:
        structure_type ( const util::position_type&
                       , const pnet::type::signature::structured_type& sig
                       );

        const pnet::type::signature::structured_type& signature() const;
        const std::string& name() const;

        void specialize (const std::unordered_map<std::string, std::string>&);
        void resolve (const std::unordered_map<std::string, structure_type>&);

      private:
        pnet::type::signature::structured_type _sig;
      };

      typedef std::list<structure_type> structs_type;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const structure_type & st
                  );
      }
    }

    namespace structure_type_util
    {
      typedef std::unordered_map<std::string, type::structure_type> set_type;
      typedef std::unordered_map<std::string, std::string> forbidden_type;

      set_type make (const type::structs_type & structs, state::type const&);

      set_type join (set_type const& above, set_type const& below);
    }
  }
}
