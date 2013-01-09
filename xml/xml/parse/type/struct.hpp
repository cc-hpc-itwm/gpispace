// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_STRUCT_HPP
#define _XML_PARSE_TYPE_STRUCT_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/function.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/literal/valid_name.hpp>
#include <we/type/signature.hpp>

#include <list>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>
#include <boost/variant.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct structure_type
      {
        ID_SIGNATURES(structure);
        PARENT_SIGNATURES(function);

      public:
        structure_type ( ID_CONS_PARAM(structure)
                       , PARENT_CONS_PARAM(function)
                       , const std::string& name
                       , const signature::desc_t& sig
                       , const boost::filesystem::path& path
                       );

        const signature::desc_t& signature() const;
        signature::desc_t& signature();
        const signature::desc_t& signature (const signature::desc_t& sig);

        const std::string& name() const;
        const std::string& name (const std::string& name);

        const boost::filesystem::path& path() const;

        id::ref::structure clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _name;
        signature::desc_t _sig;
        boost::filesystem::path _path;
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

    namespace structure_type
    {
      typedef boost::unordered_map< signature::field_name_t
                                  , type::structure_type
                                  > set_type;
      typedef boost::unordered_map< signature::field_name_t
                                  , std::string
                                  > forbidden_type;

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

      class get_literal_type_name
        : public boost::static_visitor<literal::type_name_t>
      {
      public:
        literal::type_name_t operator () (const literal::type_name_t & t) const;
        literal::type_name_t operator () (const signature::structured_t &) const;
      };

      class resolve : public boost::static_visitor<bool>
      {
      private:
        const boost::filesystem::path path;
        const set_type & sig_set;

      public:
        resolve ( const set_type & _sig_set
                , const boost::filesystem::path & _path
                );

        bool operator () (literal::type_name_t & t) const;
        bool operator () (signature::structured_t & map) const;
      };

      class specialize : public boost::static_visitor<signature::desc_t>
      {
      private:
        const type::type_map_type & map_in;

      public:
        specialize (const type::type_map_type & _map_in);

        signature::desc_t operator () (literal::type_name_t & t) const;
        signature::desc_t operator () (signature::structured_t & map) const;
      };
    }
  }
}

#endif
