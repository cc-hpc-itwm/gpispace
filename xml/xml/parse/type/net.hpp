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
        ID_SIGNATURES(net);
        PARENT_SIGNATURES(function);

      private:
        typedef fhg::util::maybe<std::string> maybe_string_type;

      public:
        typedef xml::util::unique<function_type,id::ref::function,maybe_string_type> functions_type;
        typedef xml::util::unique<place_type,id::ref::place> places_type;
        typedef xml::util::unique<specialize_type,id::ref::specialize> specializes_type;
        typedef xml::util::unique<tmpl_type,id::ref::tmpl,maybe_string_type> templates_type;
        typedef xml::util::unique<transition_type,id::ref::transition> transitions_type;


        net_type ( ID_CONS_PARAM(net)
                 , PARENT_CONS_PARAM(function)
                 , const boost::filesystem::path& path
                       = boost::filesystem::path()
                 );

        net_type ( ID_CONS_PARAM(net)
                 , PARENT_CONS_PARAM(function)
                 , const functions_type& functions
                 , const places_type& places
                 , const specializes_type& specializes
                 , const templates_type& templates
                 , const transitions_type& transitions
                 , const structs_type& structs
                 , const bool& contains_a_module_call
                 , const xml::parse::structure_type::set_type& resol
                 , const we::type::property::type& properties
                 , const boost::filesystem::path& path
                 );

        const boost::filesystem::path& path() const;

        // ***************************************************************** //

        const functions_type& functions() const;
        const places_type& places() const;
        const specializes_type& specializes() const;
        const templates_type& templates() const;
        const transitions_type& transitions() const;

        // ***************************************************************** //

        boost::optional<const id::ref::function&>
        get_function (const std::string& name) const;

        boost::optional<const id::ref::tmpl&>
        get_template (const std::string& name) const;

        // ***************************************************************** //

        const id::ref::function& push_function (const id::ref::function&);
        const id::ref::place& push_place (const id::ref::place&);
        const id::ref::specialize& push_specialize (const id::ref::specialize&);
        const id::ref::tmpl& push_template (const id::ref::tmpl&);
        const id::ref::transition& push_transition (const id::ref::transition&);

        // ***************************************************************** //

        bool has_function (const std::string& name) const;
        bool has_place (const std::string& name) const;
        bool has_specialize (const std::string& name) const;
        bool has_template (const std::string& name) const;
        bool has_transition (const std::string& name) const;

        // ***************************************************************** //

        void erase_function (const id::ref::function&);
        void erase_place (const id::ref::place&);
        void erase_specialize (const id::ref::specialize&);
        void erase_template (const id::ref::tmpl&);
        void erase_transition (const id::ref::transition&);

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

        id::ref::net clone
          (boost::optional<parent_id_type> parent = boost::none) const;

      private:
        functions_type _functions;
        places_type _places;
        specializes_type _specializes;
        templates_type _templates;
        transitions_type _transitions;

        //! \todo Everything below should be private with accessors.
      public:
        structs_type structs;
        bool contains_a_module_call;

        xml::parse::structure_type::set_type structs_resolved;

        we::type::property::type prop;

      private:
        boost::filesystem::path _path;
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
