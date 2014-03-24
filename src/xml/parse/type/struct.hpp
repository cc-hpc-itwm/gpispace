// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_STRUCT_HPP
#define _XML_PARSE_TYPE_STRUCT_HPP

#include <xml/parse/id/generic.hpp>
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
        ID_SIGNATURES(structure);
        PARENT_SIGNATURES(function);

      public:
        structure_type ( ID_CONS_PARAM(structure)
                       , PARENT_CONS_PARAM(function)
                       , const util::position_type&
                       , const pnet::type::signature::structured_type& sig
                       );

        const pnet::type::signature::structured_type& signature() const;
        const std::string& name() const;

        void specialize (const std::unordered_map<std::string, std::string>&);
        void resolve (const std::unordered_map<std::string, structure_type>&);

        id::ref::structure clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

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

      set_type make (const type::structs_type & structs);

      set_type join ( const set_type & above
                    , const set_type & below
                    , const forbidden_type & forbidden
                    , const state::type & state
                    );

      set_type join ( const set_type & above
                    , const set_type & below
                    , const state::type & state
                    );

      bool struct_by_name (const std::string&, const type::structure_type&);
    }
  }
}

#endif
