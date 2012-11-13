// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/net.fwd.hpp>

#include <xml/parse/id/generic.hpp>
#include <xml/parse/id/types.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct transition_type
      {
        ID_SIGNATURES(transition);
        PARENT_SIGNATURES(net);

      public:
        typedef std::string unique_key_type;

        typedef xml::util::unique<connect_type,id::ref::connect>
          connections_type;
        typedef xml::util::unique<place_map_type,id::ref::place_map>
          place_maps_type;

        typedef boost::variant <id::ref::function, id::ref::use>
          function_or_use_type;

        transition_type ( ID_CONS_PARAM(transition)
                        , PARENT_CONS_PARAM(net)
                        );

        transition_type ( ID_CONS_PARAM(transition)
                        , PARENT_CONS_PARAM(net)
                        , const function_or_use_type&
                        );

        transition_type ( ID_CONS_PARAM(transition)
                        , PARENT_CONS_PARAM(net)
                        , const boost::optional<function_or_use_type>&
                        , const std::string& name
                        , const connections_type& in
                        , const connections_type& out
                        , const connections_type& read
                        , const place_maps_type& place_map
                        , const structs_type& structs
                        , const conditions_type& cond
                        , const requirements_type& requirements
                        , const boost::optional<petri_net::prio_t>& priority
                        , const boost::optional<bool>& finline
                        , const boost::optional<bool>& internal
                        , const we::type::property::type& prop
                        , const boost::filesystem::path& path
                        );

        const function_or_use_type& function_or_use() const;
        function_or_use_type& function_or_use();
        const function_or_use_type& function_or_use
          (const function_or_use_type& function_or_use_);

        const std::string& name() const;
        const std::string& name(const std::string& name);

        // ***************************************************************** //

        boost::optional<const id::ref::function&>
        get_function (const std::string&) const;

        // ***************************************************************** //

        const connections_type& in() const;
        const connections_type& out() const;
        const connections_type& read() const;
        const place_maps_type& place_map() const;

        // ***************************************************************** //

        void push_in (const id::ref::connect&);
        void push_out (const id::ref::connect&);
        void push_read (const id::ref::connect&);
        void push_place_map (const id::ref::place_map&);

        // ***************************************************************** //

        void clear_ports ();
        void clear_place_map ();

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::structure_type::forbidden_type & forbidden
                     );
        void resolve ( const xml::parse::structure_type::set_type & global
                     , const state::type & state
                     , const xml::parse::structure_type::forbidden_type & forbidden
                     );

        // ***************************************************************** //

        void specialize ( const type::type_map_type & map
                        , const type::type_get_type & get
                        , const xml::parse::structure_type::set_type & known_structs
                        , state::type & state
                        );

        // ***************************************************************** //

        void sanity_check (const state::type & state) const;

        // ***************************************************************** //

        void type_check ( const std::string & direction
                        , const connect_type & connect
                        , const state::type & state
                        ) const;

        void type_check (const state::type & state) const;

        const unique_key_type& unique_key() const;

        id::ref::transition clone
          (boost::optional<parent_id_type> parent = boost::none) const;

      private:
        boost::optional<function_or_use_type> _function_or_use;

        std::string _name;

        connections_type _in;
        connections_type _out;
        connections_type _read;
        place_maps_type _place_map;

        //! \todo All below should be private with accessors.
      public:
        structs_type structs;
        conditions_type cond;
        requirements_type requirements;

        boost::optional<petri_net::prio_t> priority;
        boost::optional<bool> finline;
        boost::optional<bool> internal;

        we::type::property::type prop;

        boost::filesystem::path path;
      };

      // ******************************************************************* //

      void transition_synthesize
        ( const id::ref::transition & id_transition
        , const state::type & state
        , const net_type & net
        , we::activity_t::transition_type::net_type & we_net
        , const place_map_map_type & pids
        , we::activity_t::transition_type::edge_type & e
        );

      // ******************************************************************* //

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const transition_type & t
                  );
      }
    }
  }
}

#endif
