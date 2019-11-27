// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/memory_buffer.hpp>
#include <xml/parse/type/memory_transfer.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/preferences.hpp>
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
      struct function_type : with_position_of_definition
      {
      public:
        typedef std::string unique_key_type;

        typedef fhg::pnet::util::unique<port_type> ports_type;

        typedef boost::variant < expression_type
                               , module_type
                               , boost::recursive_wrapper<net_type>
                               > content_type;

        // ***************************************************************** //

        function_type ( const util::position_type&
                      , const boost::optional<std::string>& name
                      , const ports_type& ports
                      , fhg::pnet::util::unique<memory_buffer_type> const&
                      , std::list<memory_get> const&
                      , std::list<memory_put> const&
                      , std::list<memory_getput> const&
                      , const bool& contains_a_module_call
                      , const structs_type& structs
                      , const conditions_type&
                      , const requirements_type& requirements
                      , const preferences_type& preferences
                      , const content_type& content
                      , const we::type::property::type& properties
                      );

        //! \note explicitly defaulted in compilation unit to be able
        //! to forward declare content_type only
        function_type (function_type const&);
        function_type (function_type&&);
        function_type& operator= (function_type const&);
        function_type& operator= (function_type&&);
        ~function_type();

        function_type with_name (std::string) const;

        const content_type& content() const;
        content_type& content();

        // ***************************************************************** //

        bool is_net() const;
        boost::optional<net_type const&> get_net() const;
        net_type& get_net();

        // ***************************************************************** //

        const boost::optional<std::string>& name() const;

        // ***************************************************************** //

        std::list<memory_get> const& memory_gets() const;
        std::list<memory_put> const& memory_puts() const;
        std::list<memory_getput> const& memory_getputs() const;

        fhg::pnet::util::unique<memory_buffer_type>
          const& memory_buffers() const;

        const ports_type& ports() const;

        boost::optional<const port_type&> get_port_in (const std::string & name) const;
        boost::optional<const port_type&> get_port_out (const std::string & name) const;

        bool is_known_port_in (const std::string & name) const;
        bool is_known_port_out (const std::string & name) const;
        bool is_known_port (const std::string & name) const;
        bool is_known_port_inout (const std::string & name) const;
        bool is_known_tunnel (const std::string& name) const;

        // ***************************************************************** //

        const conditions_type& conditions() const;

        // ***************************************************************** //

        const preferences_type& preferences() const;

        // ***************************************************************** //

        void type_check (const state::type & state) const;
        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

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
          , fhg::pnet::util::unique<place_map_type> const&
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

        const unique_key_type& unique_key() const;

      private:
        //! \note should be const but can't be due to a massive amount
        //! of copy-assignments which should be copy-constructions but
        //! happen in optionals/variants and thus aren't. note that
        //! this can destroy the unique_key() when stored in a unique
        //! and directly modifying it (there is no setter, use
        //! with_name() instead)
        boost::optional<std::string> _name;

        ports_type _ports;
        fhg::pnet::util::unique<memory_buffer_type> _memory_buffers;

        std::list<memory_get> _memory_gets;
        std::list<memory_put> _memory_puts;
        std::list<memory_getput> _memory_getputs;

      public:
        bool contains_a_module_call;

        structs_type structs;

      private:
        conditions_type _conditions;
        preferences_type _preferences;

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

      void mk_wrapper ( const state::type & state
                      , const fun_info_map & m
                      );

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       , const std::unordered_set<std::string>& structnames
                       );

      bool find_module_calls ( const state::type & state
                             , function_type&
                             , fun_info_map & m
                             );

      // ***************************************************************** //

      void struct_to_cpp ( const state::type &
                         , function_type const&
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
