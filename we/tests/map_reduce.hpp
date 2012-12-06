#ifndef WE_TESTS_MAP_REDUCE_HPP
#define WE_TESTS_MAP_REDUCE_HPP 1

namespace we
{
  namespace tests
  {
    using petri_net::connection_t;
    using petri_net::edge::PT;
    using petri_net::edge::PT_READ;
    using petri_net::edge::TP;

    template <typename Activity>
    struct map_reduce
    {
      typedef typename Activity::transition_type transition_type;

      typedef typename transition_type::edge_type edge_type;

      typedef typename transition_type::net_type net_type;
      typedef typename transition_type::mod_type mod_type;
      typedef typename transition_type::expr_type expr_type;
      typedef typename transition_type::cond_type cond_type;

      typedef typename transition_type::pid_t pid_t;
      typedef typename petri_net::tid_t tid_t;
      typedef typename transition_type::port_id_t port_id_t;

      static transition_type generate ()
      {
        transition_type map_reduce
          ( "map"
          , mod_type ("map_red", "map")
          , cond_type("true")
          , transition_type::external
          );

        map_reduce.add_ports ()
          ("in", literal::STRING(), we::type::PORT_IN)
          ("out", literal::STRING(), we::type::PORT_OUT)
          ;

        return map_reduce;
      }
    };
  }
}

#endif
