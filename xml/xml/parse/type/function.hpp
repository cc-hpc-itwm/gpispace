// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <xml/parse/id/mapper.fwd.hpp>
#include <xml/parse/id/types.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/util/mk_fstream.hpp>
#include <xml/parse/util/unique.hpp>

#include <we/we.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef xml::util::unique<port_type,id::port>::elements_type ports_type;
      typedef std::list<std::string> conditions_type;
      typedef xml::util::unique<function_type,id::function>::elements_type functions_type;
      typedef xml::util::unique<template_type,id::tmpl>::elements_type templates_type;
      typedef xml::util::unique<specialize_type,id::specialize>::elements_type specializes_type;

      struct function_type
      {
      private:
        typedef xml::util::unique<port_type,id::port> unique_port_type;

        unique_port_type _in;
        unique_port_type _out;
        unique_port_type _tunnel;

        // ***************************************************************** //

        void push ( const port_type & p
                  , unique_port_type & ports
                  , const unique_port_type & others
                  , const std::string descr
                  );

        // ***************************************************************** //

        typedef boost::unordered_set<std::string> typenames_type;

        typenames_type _typenames;

        id::function _id;

      public:
        typedef boost::variant< id::transition
                              , id::tmpl
                              , boost::blank
                              > id_parent;
      public:
        id_parent _parent;
        id::mapper* _id_mapper;

        fhg::util::maybe<std::string> _name;

      public:
        typedef boost::variant < expression_type
                               , mod_type
                               , boost::recursive_wrapper<net_type>
                               > type;

        bool contains_a_module_call;
        structs_type structs;

        fhg::util::maybe<bool> internal;

        conditions_type cond;

        requirements_type requirements;

        we::type::property::type prop;

        type f;

        boost::filesystem::path path;

        xml::parse::struct_t::set_type structs_resolved;

        // ***************************************************************** //

        bool is_net() const;

        // ***************************************************************** //

        const fhg::util::maybe<std::string> name() const;
        const std::string& name (const std::string& name);
        const fhg::util::maybe<std::string>&
        name (const fhg::util::maybe<std::string>& name);

        function_type ( const type& _f
                      , const id::function& id
                      , const id_parent& parent
                      , id::mapper* id_mapper
                      );

        const id::function& id() const;
        const id_parent& parent() const;

        bool is_same (const function_type& other) const;

#ifdef BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
        function_type & operator= (function_type const &rhs);
#endif // BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND

        // ***************************************************************** //

        const typenames_type& typenames () const;
        void insert_typename (const std::string& tn);

        // ***************************************************************** //

        void add_expression (const expression_type & e);

        // ***************************************************************** //

        const ports_type& in (void) const;
        const ports_type& out (void) const;
        const ports_type& tunnel (void) const;

        boost::optional<port_type> get_port_in (const std::string & name) const;
        boost::optional<port_type> get_port_out (const std::string & name) const;

        bool is_known_port_in (const std::string & name) const;
        bool is_known_port_out (const std::string & name) const;
        bool is_known_port (const std::string & name) const;
        bool is_known_port_inout (const std::string & name) const;
        bool is_known_tunnel (const std::string& name) const;

        // ***************************************************************** //

        std::string condition (void) const;

        // ***************************************************************** //

        void push_in (const port_type & p);
        void push_out (const port_type & p);
        void push_inout (const port_type & p);
        void push_tunnel (const port_type& p);

        // ***************************************************************** //

        xml::parse::struct_t::forbidden_type forbidden_below (void) const;

        // ***************************************************************** //

        void distribute_function (const state::type& state);
        void distribute_function ( const state::type& state
                                 , const functions_type& functions
                                 , const templates_type& templates
                                 , const specializes_type& specializes
                                 );

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     );
        void resolve
          ( const xml::parse::struct_t::set_type & global
          , const state::type & state
          , const xml::parse::struct_t::forbidden_type & forbidden
          );

        // ***************************************************************** //

        signature::type type_of_port ( const we::type::PortDirection & dir
                                     , const port_type & port
                                     ) const;

        // ***************************************************************** //

        void sanity_check (const state::type & state) const;

        // ***************************************************************** //

        void type_check (const state::type & state) const;

        // ***************************************************************** //

        we::activity_t::transition_type
        synthesize (const state::type & state);

        // ***************************************************************** //

        void specialize (const state::type & state);

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::struct_t::set_type & known_structs
                        , const state::type & state
                        );
      };

      // ***************************************************************** //

      struct fun_info_type
      {
        std::string name;
        std::string code;
        flags_type ldflags;
        flags_type cxxflags;
        links_type links;
        boost::filesystem::path path;

        fun_info_type ( const std::string & _name
                      , const std::string & _code
                      , const flags_type & _ldflags
                      , const flags_type & _cxxflags
                      , const links_type & _links
                      , const boost::filesystem::path & _path
                      );

        bool operator== (const fun_info_type & other) const;

        friend std::size_t hash_value (const fun_info_type &);
      };

      std::size_t hash_value (const fun_info_type & fi);

      typedef boost::unordered_set<fun_info_type> fun_infos_type;

      typedef boost::unordered_map<std::string,fun_infos_type> fun_info_map;

      typedef boost::filesystem::path path_t;

      void mk_wrapper ( const state::type & state
                      , const fun_info_map & m
                      );

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       );

      bool find_module_calls ( const state::type &
                             , function_type &
                             , fun_info_map &
                             , mcs_type &
                             );

      bool find_module_calls ( const state::type & state
                             , net_type & n
                             , fun_info_map & m
                             , mcs_type& mcs
                             );

      bool find_module_calls ( const state::type & state
                             , function_type & f
                             , fun_info_map & m
                             );

      bool find_module_calls ( const state::type & state
                             , function_type & f
                             , fun_info_map & m
                             , mcs_type& mcs
                             );

      // ***************************************************************** //

      void struct_to_cpp ( const struct_t & s
                         , const state::type & state
                         );

      void structs_to_cpp ( const structs_type & structs
                          , const state::type & state
                          );

      // ***************************************************************** //

      void struct_to_cpp (const state::type &, const function_type &);

      // ***************************************************************** //

      void struct_to_cpp ( const state::type & state
                         , const function_type & f
                         );

      // ******************************************************************* //

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const function_type & f
                  , const state::type & state
                  );

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const function_type & f
                  );
      } // namespace dump
    }
  }
}

#endif
