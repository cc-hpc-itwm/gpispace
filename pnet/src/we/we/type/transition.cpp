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
  }
}
