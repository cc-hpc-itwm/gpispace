// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <xml/parse/id/mapper.fwd.hpp>
#include <xml/parse/id/types.hpp>
#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/use.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef xml::util::unique<connect_type,id::connect>::elements_type connections_type;

      struct transition_type
      {
      private:
        xml::util::unique<connect_type,id::connect> _in;
        xml::util::unique<connect_type,id::connect> _out;
        xml::util::unique<connect_type,id::connect> _read;
        xml::util::unique<place_map_type,id::place_map> _place_map;

        id::transition _id;
        id::net _parent;
        id::mapper* _id_mapper;

      public:
        id::mapper* id_mapper() const { return _id_mapper; }

        typedef boost::variant <id::ref::function, use_type>
                function_or_use_type;

        transition_type ( const id::transition& id
                        , const id::net& parent
                        , id::mapper* id_mapper
                        );
        transition_type ( const function_or_use_type& function_or_use
                        , const id::transition& id
                        , const id::net& parent
                        , id::mapper* id_mapper
                        );

        const id::transition& id() const;
        const id::net& parent() const;

        bool is_same (const transition_type& other) const;

      private:
        boost::optional<function_or_use_type> _function_or_use;

        std::string _name;

      public:
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

        boost::optional<function_type> get_function (const std::string&) const;

        // ***************************************************************** //

        const connections_type & in (void) const;
        const connections_type & out (void) const;
        const connections_type & read (void) const;
        const place_maps_type & place_map (void) const;

        connections_type & in (void);
        connections_type & out (void);
        connections_type & read (void);
        place_maps_type & place_map (void);

        // ***************************************************************** //

        void push_in (const connect_type & connect);
        void push_out (const connect_type & connect);
        void push_inout (const connect_type& connect);
        void push_read (const connect_type & connect);
        void push_place_map (const place_map_type & place_map);

        // ***************************************************************** //

        void clear_ports ();
        void clear_place_map ();

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     );
        void resolve ( const xml::parse::struct_t::set_type & global
                     , const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     );

        // ***************************************************************** //

        void specialize ( const type::type_map_type & map
                        , const type::type_get_type & get
                        , const xml::parse::struct_t::set_type & known_structs
                        , const state::type & state
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
      };

      // ******************************************************************* //

      using petri_net::connection_t;
      using petri_net::edge::PT;
      using petri_net::edge::PT_READ;
      using petri_net::edge::TP;

      void transition_synthesize
        ( const transition_type & trans
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
