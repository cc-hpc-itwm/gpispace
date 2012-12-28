// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <xml/parse/type/net.fwd.hpp>

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type_map_type.hpp>

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
      struct net_type
      {
        ID_SIGNATURES(net);
        PARENT_SIGNATURES(function);

      public:
        typedef xml::util::unique<function_type,id::ref::function> functions_type;
        typedef xml::util::unique<place_type,id::ref::place> places_type;
        typedef xml::util::unique<specialize_type,id::ref::specialize> specializes_type;
        typedef xml::util::unique<tmpl_type,id::ref::tmpl> templates_type;
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

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

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

      private:
        //! \todo Remove this and all other _functions related stuff.
        //! \note This only exists for specialization, which should be lazy.
        const id::ref::function& push_function (const id::ref::function&);

      public:
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

        void rename (const id::ref::place&, const std::string&);
        void rename (const id::ref::specialize&, const std::string&);
        void rename (const id::ref::tmpl&, const std::string&);
        void rename (const id::ref::transition&, const std::string&);

        // ***************************************************************** //

        signature::type type_of_place (const place_type&) const;

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

        void sanity_check (const state::type & state) const;

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

        xml::parse::structure_type::set_type structs_resolved;

      private:
        we::type::property::type _properties;

        boost::filesystem::path _path;
      };

      // ******************************************************************* //

      boost::unordered_map<std::string, petri_net::place_id_type>
      net_synthesize ( petri_net::net& we_net
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

#endif
