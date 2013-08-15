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
#include <we2/type/signature.hpp>

#include <list>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>
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
                       , const signature::desc_t& sig
                       , const pnet::type::signature::structured_type& sig2
                       );

        const signature::desc_t& signature() const;
        const std::string& name() const;

        void specialize (const boost::unordered_map<std::string, std::string>&);
        void resolve (const boost::unordered_map<std::string, structure_type>&);

        signature::desc_t resolve_with_fun
          (const boost::function<boost::optional<signature::type> (const std::string&)>&) const;

        id::ref::structure clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        signature::desc_t _sig;
        pnet::type::signature::structured_type _sig2;
      };

      bool operator == (const structure_type & a, const structure_type & b);
      bool operator != (const structure_type & a, const structure_type & b);

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
      typedef boost::unordered_map<std::string, type::structure_type> set_type;
      typedef boost::unordered_map<std::string, std::string> forbidden_type;

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
