// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type_map_type.hpp>

#include <xml/parse/util/unique.hpp>

#include <xml/parse/type/dumps.hpp>

#include <xml/parse/type/id.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      class function_with_mapping_type
      {
      private:
        function_type& _function;
        boost::optional<type_map_type&> _type_map;

      public:
        explicit function_with_mapping_type
          ( function_type& function
          , boost::optional<type_map_type&> type_map = boost::none
          );

        function_type& function();
        boost::optional<type_map_type&> type_map();
      };

      struct net_type
      {
        ID_SIGNATURES(net)

      public:
        typedef xml::util::unique<place_type,id::place> places_type;

      private:
        typedef fhg::util::maybe<std::string> maybe_string_type;

        places_type _places;

        boost::unordered_set<id::ref::transition> _ids_transition;
        boost::unordered_map<std::string,id::ref::transition> _by_name_transition;

        xml::util::unique<function_type,id::function,maybe_string_type> _functions;
        xml::util::unique<tmpl_type,id::tmpl,maybe_string_type> _templates;
        xml::util::unique<specialize_type,id::specialize> _specializes;

        boost::optional<id::function> _parent;

        boost::filesystem::path _path;

      public:
        bool contains_a_module_call;
        structs_type structs;

        we::type::property::type prop;

        xml::parse::structure_type::set_type structs_resolved;

        net_type ( ID_CONS_PARAM(net)
                 , const id::function& parent
                 , const boost::filesystem::path& path
                       = boost::filesystem::path()
                 );

        boost::optional<const function_type&> parent() const;
        boost::optional<function_type&> parent();
        bool has_parent() const;

        const boost::filesystem::path& path() const;

        // ***************************************************************** //

#ifdef BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
        net_type & operator= (net_type const &rhs);
#endif // BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND

        // ***************************************************************** //

        bool has_place (const std::string& name) const;
        boost::optional<place_type> get_place (const std::string & name) const;
        boost::optional<place_type> place_by_id (const id::place& id) const;

        bool has_transition (const std::string& name) const;
        const boost::unordered_set<id::ref::transition>& ids_transition() const;
        boost::unordered_set<id::ref::transition>& ids_transition();

        boost::optional<function_type> get_function (const std::string & name) const;

        boost::optional<tmpl_type> get_template (const std::string & name) const;

        // ***************************************************************** //

        function_with_mapping_type get_function (const std::string & name);

        // ***************************************************************** //

        places_type & places (void);
        const places_type & places (void) const;

        const functions_type & functions (void) const;

        const specializes_type & specializes (void) const;

        const templates_type & templates (void) const;

        // ***************************************************************** //

        void push_place (const place_type & p);
        void erase_place (const place_type& t);

        const id::ref::transition& push_transition (const id::ref::transition&);

        void push_function (const function_type & f);

        void push_template (const tmpl_type & t);

        void push_specialize ( const specialize_type & s
                             , const state::type & state
                             );

        // ***************************************************************** //

        void clear_places (void);

        void clear_transitions (void);

        // ***************************************************************** //

        signature::type type_of_place (const place_type & place) const;

        // ***************************************************************** //

        void type_map_apply ( const type::type_map_type & outer_map
                            , type::type_map_type & inner_map
                            );

        void specialize ( const type::type_map_type & map
                        , const type::type_get_type & get
                        , const xml::parse::structure_type::set_type & known_structs
                        , state::type & state
                        );

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::structure_type::forbidden_type & forbidden
                     );

        void resolve ( const xml::parse::structure_type::set_type & global
                     , const state::type & state
                     , const xml::parse::structure_type::forbidden_type & forbidden
                     );

        // ***************************************************************** //

        void sanity_check (const state::type & state, const function_type& outerfun) const;

        // ***************************************************************** //

        void type_check (const state::type & state) const;

        void set_prefix (const std::string & prefix);
        void remove_prefix (const std::string & prefix);
      };

      // ******************************************************************* //

      boost::unordered_map< std::string
                          , we::activity_t::transition_type::pid_t
                          >
      net_synthesize ( we::activity_t::transition_type::net_type & we_net
                     , const place_map_map_type & place_map_map
                     , const net_type & net
                     , const state::type & state
                     , we::activity_t::transition_type::edge_type & e
                     );

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const net_type & net
                  );
      } // namespace dump
    }
  }
}

#endif
