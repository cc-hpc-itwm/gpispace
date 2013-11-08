// {mirko.rahn}@itwm.fraunhofer.de

#include <we/type/transition.hpp>

#include <we/type/net.hpp>

namespace we
{
  namespace type
  {
    namespace
    {
      class visitor_expression
        : public boost::static_visitor<boost::optional<const expression_t&> >
      {
      public:
        boost::optional<const expression_t&>
          operator() (const expression_t& e) const
        {
          return e;
        }
        boost::optional<const expression_t&>
          operator() (const module_call_t&) const
        {
          return boost::none;
        }
        boost::optional<const expression_t&>
          operator() (const petri_net::net&) const
        {
          return boost::none;
        }
      };
    }

    boost::optional<const expression_t&> transition_t::expression() const
    {
      return boost::apply_visitor (visitor_expression(), data());
    }

    // ********************************************************************* //

    transition_t::port_id_with_prop_t
    output_port_by_pid ( transition_t const& trans
                       , const petri_net::place_id_type& pid
                       )
    {
      BOOST_FOREACH ( transition_t::inner_to_outer_t::value_type const& p
                    , trans.inner_to_outer()
                    )
      {
        if (p.second.first == pid)
        {
          return std::make_pair (p.first, p.second.second);
        }
      }

      throw exception::not_connected<petri_net::place_id_type>
        ("trans: "+trans.name()+": output port not connected by pid: "+ fhg::util::show (pid), pid);
    }

    // ********************************************************************* //
  }
}
