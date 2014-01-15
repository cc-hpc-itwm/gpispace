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
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <we/type/property.hpp>
#include <we/type/transition.hpp>
#include <we/type/port.hpp>

#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct conditions_type : public std::list<std::string>
      {
        std::string flatten() const;
      };

      conditions_type operator+ (conditions_type, const conditions_type&);

      struct function_type : with_position_of_definition
      {
        ID_SIGNATURES(function);

      private:
        typedef boost::unordered_set<std::string> typenames_type;

      public:
        typedef std::string unique_key_type;

        typedef xml::util::unique<port_type,id::ref::port> ports_type;

        //! \todo net is only in this list as specialize not yet
        //! reparents the function to the transition requesting it, as
        //! specialization is not yet lazy. If it is, also remove
        //! name_impl. See net_type::specialize() and function_type::name().
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
                      , const util::position_type&
                      , const content_type& content
                      );

        function_type ( ID_CONS_PARAM(function)
                      , const boost::optional<parent_id_type>& parent
                      , const util::position_type&
                      , const boost::optional<std::string>& name
                      , const boost::optional<bool>& internal
                      , const content_type& content
                      );

        function_type ( ID_CONS_PARAM(function)
                      , const boost::optional<parent_id_type>& parent
                      , const util::position_type&
                      , const boost::optional<std::string>& name
                      , const ports_type& ports
                      , const typenames_type& typenames
                      , const bool& contains_a_module_call
                      , const boost::optional<bool>& internal
                      , const structs_type& structs
                      , const conditions_type&
                      , const requirements_type& requirements
                      , const content_type& content
                      , const we::type::property::type& properties
                      );

        const content_type& content() const;
        content_type& content();
        const content_type& content (const content_type&);

        // ***************************************************************** //

        bool is_net() const;
        boost::optional<const id::ref::net&> get_net() const;

        // ***************************************************************** //

        //! \note function should not be something that can be in a
        //! unique. It only is in an unique, as net does not lazily
        //! specialize templates. It then stores them in a
        //! unique. Other parents (transition, tmpl) never have a
        //! unique, thus don't need to get notified on name
        //! change. This name() + name_impl() pattern can thus be
        //! removed as soon as net no longer can be a parent.
        const boost::optional<std::string>& name() const;
        const boost::optional<std::string>&
          name (const boost::optional<std::string>& name);

      private:
        friend struct net_type;
        const boost::optional<std::string>&
          name_impl (const boost::optional<std::string>& name);

      public:
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

        const conditions_type& conditions() const;
        void add_conditions (const std::list<std::string>&);

        // ***************************************************************** //

        xml::parse::structure_type_util::forbidden_type forbidden_below (void) const;

        // ***************************************************************** //

        boost::optional<pnet::type::signature::signature_type> signature (const std::string&) const;

        // ***************************************************************** //

        void sanity_check (const state::type & state) const;

        // ***************************************************************** //

        void type_check (const state::type & state) const;

        // ***************************************************************** //

        we::type::transition_t synthesize
          ( const std::string&
          , const state::type&
          , boost::unordered_map<std::string, we::port_id_type>& port_id_in
          , boost::unordered_map<std::string, we::port_id_type>& port_id_out
          , const boost::optional<bool>&
          , const conditions_type&
          , const we::type::property::type&
          , const requirements_type&
          ) const;

        // ***************************************************************** //

        void specialize (state::type & state);

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::structure_type_util::set_type & known_structs
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

      private:
        conditions_type _conditions;

      public:
        requirements_type requirements;

      private:
        content_type _content;
        we::type::property::type _properties;
      };

      // ***************************************************************** //

      struct fun_info_type
      {
        std::string name;
        std::string code;
        std::list<std::string> ldflags;
        std::list<std::string> cxxflags;
        std::list<link_type> links;
        boost::filesystem::path path;

        fun_info_type ( const std::string & _name
                      , const std::string & _code
                      , const std::list<std::string>& _ldflags
                      , const std::list<std::string>& _cxxflags
                      , const std::list<link_type>& _links
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
                       , const boost::unordered_set<std::string>& structnames
                       );

      bool find_module_calls ( const state::type & state
                             , const id::ref::function&
                             , fun_info_map & m
                             );

      // ***************************************************************** //

      void struct_to_cpp ( const state::type &
                         , const id::ref::function &
                         , boost::unordered_set<std::string>&
                         );

      // ******************************************************************* //

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const function_type & f
                  );
      } // namespace dump
    }
  }
}

#endif
