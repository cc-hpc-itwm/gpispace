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
#include <xml/parse/util/parent.hpp>

#include <xml/parse/type/dumps.hpp>

#include <xml/parse/id/generic.hpp>

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
        PARENT_SIGNATURES(function)

      private:
        typedef fhg::util::maybe<std::string> maybe_string_type;

        typedef xml::util::unique<place_type,id::ref::place> places_type;
        typedef xml::util::unique<transition_type,id::ref::transition> transitions_type;
        typedef xml::util::unique<specialize_type,id::ref::specialize> specializes_type;
        typedef xml::util::unique<tmpl_type,id::ref::tmpl,maybe_string_type> templates_type;
        typedef xml::util::unique<function_type,id::ref::function,maybe_string_type> functions_type;

        places_type _places;
        transitions_type _transitions;
        specializes_type _specializes;
        templates_type _templates;
        functions_type _functions;

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

        const boost::filesystem::path& path() const;

        // ***************************************************************** //

        const places_type& places() const;
        const transitions_type& transitions() const;
        const specializes_type& specializes() const;
        const templates_type& templates() const;
        const functions_type& functions() const;

        // ***************************************************************** //

        boost::optional<const id::ref::function&>
        get_function (const std::string& name) const;

        boost::optional<const id::ref::tmpl&>
        get_template (const std::string& name) const;

        // ***************************************************************** //

        const id::ref::place& push_place (const id::ref::place &);
        const id::ref::transition& push_transition (const id::ref::transition&);
        const id::ref::specialize& push_specialize (const id::ref::specialize&);
        const id::ref::tmpl& push_template (const id::ref::tmpl&);
        const id::ref::function& push_function (const id::ref::function&);

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
