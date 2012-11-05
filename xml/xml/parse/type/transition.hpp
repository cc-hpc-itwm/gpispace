// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/net.fwd.hpp>

#include <xml/parse/id/generic.hpp>
#include <xml/parse/util/parent.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct transition_type
      {
        ID_SIGNATURES(transition)
        PARENT_SIGNATURES(net)

      private:
        typedef xml::util::unique<connect_type,id::ref::connect>
          connections_type;
        typedef xml::util::unique<place_map_type,id::ref::place_map>
          place_maps_type;

      public:
        typedef boost::variant <id::ref::function, use_type>
                function_or_use_type;

        transition_type ( ID_CONS_PARAM(transition)
                        , PARENT_CONS_PARAM(net)
                        );
        transition_type ( ID_CONS_PARAM(transition)
                        , PARENT_CONS_PARAM(net)
                        , const function_or_use_type& function_or_use
                        );

        const function_or_use_type& function_or_use() const;
        function_or_use_type& function_or_use();
        const function_or_use_type& function_or_use
          (const function_or_use_type& function_or_use_);

        const std::string& name() const;
        const std::string& name(const std::string& name);

        boost::filesystem::path path;

        we::type::property::type prop;

        structs_type structs;

        conditions_type cond;

        requirements_type requirements;

        fhg::util::maybe<petri_net::prio_t> priority;

        fhg::util::maybe<bool> finline;

        fhg::util::maybe<bool> internal;

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
                        , const net_type & net
                        , const state::type & state
                        ) const;

        void type_check (const net_type & net, const state::type & state) const;

      private:
        boost::optional<function_or_use_type> _function_or_use;

        std::string _name;

        connections_type _in;
        connections_type _out;
        connections_type _read;
        place_maps_type _place_map;
      };

      // ******************************************************************* //

      using petri_net::connection_t;
      using petri_net::edge::PT;
      using petri_net::edge::PT_READ;
      using petri_net::edge::TP;

      void transition_synthesize
        ( const id::transition & id_transition
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
