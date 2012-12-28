// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

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

#include <we/type/property.hpp>
#include <we/type/transition.hpp>
#include <we/type/port.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> conditions_type;

      struct function_type
      {
        ID_SIGNATURES(function);

      private:
        typedef boost::unordered_set<std::string> typenames_type;

      public:
        typedef std::string unique_key_type;

        typedef xml::util::unique<port_type,id::ref::port> ports_type;

        //! \todo net is only in this list as specialize not yet
        //! reparents the function to the transition requesting it, as
        //! specialization is not yet lazy. See net_type::specialize()
        typedef boost::variant< id::transition
                              , id::tmpl
                              , id::net
                              > parent_id_type;

        typedef boost::variant < id::ref::expression
                               , id::ref::module
                               , id::ref::net
                               > content_type;

        // ***************************************************************** //

        template<typename T>
          static boost::optional<parent_id_type> make_parent (const T& id)
        {
          return boost::make_optional (parent_id_type (id));
        }

        // ***************************************************************** //

        function_type ( ID_CONS_PARAM(function)
                      , const boost::optional<parent_id_type>& parent
                      , const content_type& content
                      );

        function_type ( ID_CONS_PARAM(function)
                      , const boost::optional<parent_id_type>& parent
                      , const boost::optional<std::string>& name
                      , const boost::optional<bool>& internal
                      , const content_type& content
                      , const boost::filesystem::path& path
                      );

        function_type ( ID_CONS_PARAM(function)
                      , const boost::optional<parent_id_type>& parent
                      , const boost::optional<std::string>& name
                      , const ports_type& ports
                      , const typenames_type& typenames
                      , const bool& contains_a_module_call
                      , const boost::optional<bool>& internal
                      , const structs_type& structs
                      , const conditions_type& cond
                      , const requirements_type& requirements
                      , const content_type& content
                      , const xml::parse::structure_type::set_type& resolved
                      , const we::type::property::type& properties
                      , const boost::filesystem::path& path
                      );

        const content_type& content() const;
        content_type& content();
        const content_type& content (const content_type&);

        // ***************************************************************** //

        bool is_net() const;
        boost::optional<const id::ref::net&> get_net() const;

        // ***************************************************************** //

        const boost::optional<std::string>& name() const;
        const std::string& name (const std::string& name);
        const boost::optional<std::string>&
        name (const boost::optional<std::string>& name);

        const boost::optional<parent_id_type>& parent() const;

        bool has_parent() const;
        void unparent();
        void parent (const parent_id_type& parent);

        boost::optional<id::ref::transition> parent_transition() const;
        boost::optional<id::ref::tmpl> parent_tmpl() const;
        //! \todo This should be removed soon (i.e. when specialization is lazy).
        boost::optional<id::ref::net> parent_net() const;

        boost::optional<const id::ref::function&>
        get_function (const std::string& name) const;

        // ***************************************************************** //

        void push_port (const id::ref::port&);
        void remove_port (const id::ref::port&);

        const ports_type& ports() const;

        boost::optional<const id::ref::port&> get_port_in (const std::string & name) const;
        boost::optional<const id::ref::port&> get_port_out (const std::string & name) const;

        bool is_known_port_in (const std::string & name) const;
        bool is_known_port_out (const std::string & name) const;
        bool is_known_port (const std::string & name) const;
        bool is_known_port_inout (const std::string & name) const;
        bool is_known_tunnel (const std::string& name) const;

        void rename (const id::ref::port&, const std::string&);
        void port_direction
          (const id::ref::port&, const we::type::PortDirection&);

        // ***************************************************************** //

        const typenames_type& typenames () const;
        void insert_typename (const std::string& tn);

        // ***************************************************************** //

        void add_expression (const expressions_type & e);

        // ***************************************************************** //

        std::string condition (void) const;

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

        we::type::transition_t
        synthesize (const state::type & state);

        // ***************************************************************** //

        void specialize (state::type & state);

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::structure_type::set_type & known_structs
                        , state::type & state
                        );

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        const unique_key_type& unique_key() const;

        id::ref::function clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        boost::optional<parent_id_type> _parent;

        boost::optional<std::string> _name;

        ports_type _ports;

        typenames_type _typenames;

      public:
        bool contains_a_module_call;

        boost::optional<bool> internal;

        structs_type structs;
        conditions_type cond;
        requirements_type requirements;

      private:
        content_type _content;

      public:
        xml::parse::structure_type::set_type structs_resolved;

      private:
        we::type::property::type _properties;

      public:
        boost::filesystem::path path;
      };

      // ***************************************************************** //

      struct fun_info_type
      {
        std::string name;
        std::string code;
        module_type::flags_type ldflags;
        module_type::flags_type cxxflags;
        module_type::links_type links;
        boost::filesystem::path path;

        fun_info_type ( const std::string & _name
                      , const std::string & _code
                      , const module_type::flags_type & _ldflags
                      , const module_type::flags_type & _cxxflags
                      , const module_type::links_type & _links
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

      void struct_to_cpp (const state::type &, const id::ref::function &);

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
