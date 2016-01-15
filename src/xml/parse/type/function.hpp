// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/id/generic.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/memory_buffer.hpp>
#include <xml/parse/type/memory_transfer.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/place_map.hpp>
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

#include <string>
#include <unordered_set>

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
        typedef std::unordered_set<std::string> typenames_type;

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

        typedef boost::variant < expression_type
                               , type::module_type
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
                      , const boost::optional<std::string>& name
                      , const content_type& content
                      );

        function_type ( ID_CONS_PARAM(function)
                      , const boost::optional<parent_id_type>& parent
                      , const util::position_type&
                      , const boost::optional<std::string>& name
                      , const ports_type& ports
                      , fhg::pnet::util::unique<memory_buffer_type> const&
                      , std::list<memory_get> const&
                      , std::list<memory_put> const&
                      , std::list<memory_getput> const&
                      , const typenames_type& typenames
                      , const bool& contains_a_module_call
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

        void push_memory_get (memory_get const&);
        void push_memory_put (memory_put const&);
        void push_memory_getput (memory_getput const&p);

        std::list<memory_get> const& memory_gets() const;
        std::list<memory_put> const& memory_puts() const;
        std::list<memory_getput> const& memory_getputs() const;

        void push_memory_buffer (const memory_buffer_type&);
        fhg::pnet::util::unique<memory_buffer_type>
          const& memory_buffers() const;
        bool is_known_memory_buffer (std::string const&) const;

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

        void type_check (const state::type & state) const;

        // ***************************************************************** //

        we::type::transition_t synthesize
          ( const std::string&
          , const state::type&
          , std::unordered_map<std::string, we::port_id_type>& port_id_in
          , std::unordered_map<std::string, we::port_id_type>& port_id_out
          , const conditions_type&
          , const we::type::property::type&
          , const requirements_type&
          , we::priority_type
          , xml::util::range_type<place_map_type const>
          , std::unordered_map<we::port_id_type, std::string>& real_place_names
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
        fhg::pnet::util::unique<memory_buffer_type> _memory_buffers;

        std::list<memory_get> _memory_gets;
        std::list<memory_put> _memory_puts;
        std::list<memory_getput> _memory_getputs;

        typenames_type _typenames;

      public:
        bool contains_a_module_call;

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
        boost::filesystem::path path;

        fun_info_type ( const std::string & _name
                      , const std::string & _code
                      , const std::list<std::string>& _ldflags
                      , const std::list<std::string>& _cxxflags
                      , const boost::filesystem::path & _path
                      );

        bool operator== (const fun_info_type & other) const;
      };

      typedef std::unordered_set<fun_info_type> fun_infos_type;

      typedef std::unordered_map<std::string,fun_infos_type> fun_info_map;

      typedef boost::filesystem::path path_t;

      void mk_wrapper ( const state::type & state
                      , const fun_info_map & m
                      );

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       , const std::unordered_set<std::string>& structnames
                       );

      bool find_module_calls ( const state::type & state
                             , const id::ref::function&
                             , fun_info_map & m
                             );

      // ***************************************************************** //

      void struct_to_cpp ( const state::type &
                         , const id::ref::function &
                         , std::unordered_set<std::string>&
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

namespace std
{
  template<> struct hash<xml::parse::type::fun_info_type>
  {
    size_t operator()(xml::parse::type::fun_info_type const&) const;
  };
}
