// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <xml/parse/id/generic.hpp>
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
      typedef std::list<std::string> conditions_type;
      typedef xml::util::unique<function_type,id::function>::elements_type functions_type;
      typedef xml::util::unique<tmpl_type,id::tmpl>::elements_type templates_type;
      typedef xml::util::unique<specialize_type,id::specialize>::elements_type specializes_type;

      struct function_type
      {
        ID_SIGNATURES(function)

      public:
        typedef xml::util::uniqueID<port_type,id::ref::port> unique_port_type;

      private:
        unique_port_type _in;
        unique_port_type _out;
        unique_port_type _tunnel;

        // ***************************************************************** //

        void push ( const id::ref::port & p
                  , unique_port_type & ports
                  , const unique_port_type & others
                  , const std::string descr
                  );

        // ***************************************************************** //

        typedef boost::unordered_set<std::string> typenames_type;

        typenames_type _typenames;

      public:
        typedef boost::variant< id::transition
                              , id::tmpl
                              > id_parent;
      public:
        boost::optional<id_parent> _parent;

        fhg::util::maybe<std::string> _name;

      public:
        typedef boost::variant < expression_type
                               , module_type
                               , id::ref::net
                               > type;

        bool contains_a_module_call;
        structs_type structs;

        fhg::util::maybe<bool> internal;

        conditions_type cond;

        requirements_type requirements;

        we::type::property::type prop;

        type f;

        boost::filesystem::path path;

        xml::parse::structure_type::set_type structs_resolved;

        // ***************************************************************** //

        bool is_net() const;

        // ***************************************************************** //

        const fhg::util::maybe<std::string> name() const;
        const std::string& name (const std::string& name);
        const fhg::util::maybe<std::string>&
        name (const fhg::util::maybe<std::string>& name);

        function_type ( ID_CONS_PARAM(function)
                      , const type& _f
                      , const boost::optional<id_parent>& parent
                      );

        const boost::optional<id_parent>& parent() const;

        bool has_parent() const;

        boost::optional<const id::ref::function&>
        get_function (const std::string& name) const;

        // ***************************************************************** //

        const unique_port_type& in() const;
        const unique_port_type& out() const;
        const unique_port_type& tunnel() const;

        // ***************************************************************** //

        const typenames_type& typenames () const;
        void insert_typename (const std::string& tn);

        // ***************************************************************** //

        void add_expression (const expression_type & e);

        // ***************************************************************** //

        boost::optional<const id::ref::port&> get_port_in (const std::string & name) const;
        boost::optional<const id::ref::port&> get_port_out (const std::string & name) const;

        bool is_known_port_in (const std::string & name) const;
        bool is_known_port_out (const std::string & name) const;
        bool is_known_port (const std::string & name) const;
        bool is_known_port_inout (const std::string & name) const;
        bool is_known_tunnel (const std::string& name) const;

        // ***************************************************************** //

        std::string condition (void) const;

        // ***************************************************************** //

        void push_in (const id::ref::port&);
        void push_out (const id::ref::port&);
        void push_inout (const id::ref::port&);
        void push_tunnel (const id::ref::port&);

        // ***************************************************************** //

        xml::parse::structure_type::forbidden_type forbidden_below (void) const;

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::structure_type::forbidden_type & forbidden
                     );
        void resolve
          ( const xml::parse::structure_type::set_type & global
          , const state::type & state
          , const xml::parse::structure_type::forbidden_type & forbidden
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

        void specialize (state::type & state);

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::structure_type::set_type & known_structs
                        , state::type & state
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

      bool find_module_calls ( const state::type & state
                             , const id::ref::function&
                             , fun_info_map & m
                             );

      // ***************************************************************** //

      void struct_to_cpp ( const structure_type & s
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
