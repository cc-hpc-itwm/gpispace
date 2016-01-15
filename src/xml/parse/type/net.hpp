// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/type/net.fwd.hpp>

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <xml/parse/util/unique.hpp>

#include <xml/parse/type/dumps.hpp>

#include <xml/parse/id/generic.hpp>

#include <we/type/id.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct net_type : with_position_of_definition
      {
        ID_SIGNATURES(net);
        PARENT_SIGNATURES(function);

      public:
        typedef xml::util::unique<function_type,id::ref::function> functions_type;
        typedef fhg::pnet::util::unique<place_type> places_type;
        typedef xml::util::unique<specialize_type,id::ref::specialize> specializes_type;
        typedef xml::util::unique<tmpl_type,id::ref::tmpl> templates_type;
        typedef xml::util::unique<transition_type,id::ref::transition> transitions_type;


        net_type ( ID_CONS_PARAM(net)
                 , PARENT_CONS_PARAM(function)
                 , const util::position_type&
                 );

        net_type ( ID_CONS_PARAM(net)
                 , PARENT_CONS_PARAM(function)
                 , const util::position_type&
                 , const functions_type& functions
                 , const places_type& places
                 , const specializes_type& specializes
                 , const templates_type& templates
                 , const transitions_type& transitions
                 , const structs_type& structs
                 , const bool& contains_a_module_call
                 , const we::type::property::type& properties
                 );

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

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

        void push_place (place_type const&);
        const id::ref::specialize& push_specialize (const id::ref::specialize&);
        const id::ref::tmpl& push_template (const id::ref::tmpl&);
        const id::ref::transition& push_transition (const id::ref::transition&);

        // ***************************************************************** //

        void rename (const id::ref::function&, const std::string&);
        void rename (const id::ref::transition&, const std::string&);

        // ***************************************************************** //

        boost::optional<pnet::type::signature::signature_type> signature (const std::string&) const;

        // ***************************************************************** //

        void type_map_apply ( const type::type_map_type & outer_map
                            , type::type_map_type & inner_map
                            );

        void specialize ( const type::type_map_type & map
                        , const type::type_get_type & get
                        , const xml::parse::structure_type_util::set_type & known_structs
                        , state::type & state
                        );

        // ***************************************************************** //

        void type_check (const state::type & state) const;

        void set_prefix (const std::string & prefix);
        void remove_prefix (const std::string & prefix);

        id::ref::net clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

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

      private:
        we::type::property::type _properties;
      };

      // ******************************************************************* //

      std::unordered_map<std::string, we::place_id_type>
      net_synthesize ( we::type::net_type& we_net
                     , const place_map_map_type & place_map_map
                     , const net_type & net
                     , const state::type & state
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
