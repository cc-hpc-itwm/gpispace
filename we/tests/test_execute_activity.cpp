#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/type/port.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/mgmt/context.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::PT_READ;
using petri_net::edge::TP;

typedef we::type::transition_t transition_t;
typedef petri_net::net pnet_t;
typedef we::mgmt::type::activity_t activity_t;
typedef activity_t::input_t input_t;

namespace dummy
{
  static long dummy (const long input)
  {
    return input + 42;
  }
}

namespace module
{
  template <typename T>
  class visitor_get_value_of_literal : public boost::static_visitor<T>
  {
  public:
    T operator () (const literal::type & literal) const
    {
      return boost::get<T> (literal);
    }

    T operator () (const value::structured_t & o) const
    {
      throw std::runtime_error ("bad_get: expected literal, got: " + fhg::util::show (o));
    }
  };

  template <typename T, typename V>
  T get_value_of_literal (const V & v)
  {
    return boost::apply_visitor (visitor_get_value_of_literal<T>(), v);
  }

  template <typename ModuleCall, typename Context, typename OutputList>
  void eval (const ModuleCall & mf, const Context & ctxt, OutputList & output)
  {
    if ( mf.module() == "dummy" && mf.function() == "dummy" )
    {
      const long result
        (dummy::dummy (get_value_of_literal<long> (ctxt.value("input"))));
      output.push_back (std::make_pair (result, "output"));
    }
  }

  static void call (activity_t & act, const we::type::module_call_t & module_call)
  {
    // construct context
    typedef expr::eval::context context_t;
    typedef activity_t::input_t input_t;
    typedef activity_t::output_t output_t;
    typedef we::type::port_t port_t;
    typedef we::type::transition_t::const_iterator port_iterator;

    context_t context;
    for ( input_t::const_iterator top (act.input().begin())
        ; top != act.input().end()
        ; ++top
        )
    {
      const token::type token = top->first;
      const petri_net::port_id_type port_id = top->second;

      context.bind (act.transition().name_of_port (port_id), token.value);
    }

    typedef std::vector <std::pair<value::type, std::string> > mod_output_t;

    mod_output_t mod_output;
    module::eval ( module_call, context, mod_output );

    for ( mod_output_t::const_iterator ton (mod_output.begin())
        ; ton != mod_output.end()
        ; ++ton
        )
    {
      const petri_net::port_id_type& port_id
        (act.transition().output_port_by_name (ton->second));

      const port_t & port (act.transition().get_port (port_id));

      act.add_output
        ( output_t::value_type
          ( token::type ( port.name()
                       , port.signature()
                       , ton->first
                       )
          , port_id
          )
        );
    }
  }
}

struct exec_context : public we::mgmt::context<>
{
  typedef petri_net::net net_t;
  typedef we::type::module_call_t mod_t;
  typedef we::type::expression_t expr_t;

  void handle_internally ( activity_t & act, net_t &)
  {
    act.inject_input ();

    // submit to self
    while (act.can_fire())
    {
      std::cout << "***** act (pre-extract):"
                << std::endl
                << act
                << std::endl;

      activity_t sub = act.extract ();

      std::cout << "***** sub-act (pre-execute):"
                << std::endl
                << sub
                << std::endl;

      sub.execute (*this);

      std::cout << "***** sub-act (post-execute):"
                << std::endl
                << sub
                << std::endl;

      act.inject (sub);

      std::cout << "***** act (post-inject):"
                << std::endl
                << act
                << std::endl;
    }

    act.collect_output();
  }

  void handle_internally ( activity_t & , const mod_t & )
  {
    throw std::runtime_error ("cannot handle module calls internally");
  }

  void handle_internally ( activity_t & , const expr_t & )
  {
    // nothing to do
  }

  std::string fake_external ( const std::string & act_enc, net_t & n )
  {
    activity_t act = we::util::text_codec::decode<activity_t> (act_enc);
    handle_internally ( act, n );
    return we::util::text_codec::encode (act);
  }

  void handle_externally ( activity_t & act, net_t & n)
  {
    activity_t result ( we::util::text_codec::decode<activity_t> (fake_external (we::util::text_codec::encode(act), n)));
    act.set_output(result.output());
  }

  std::string fake_external ( const std::string & act_enc, const mod_t & mod )
  {
    activity_t act = we::util::text_codec::decode<activity_t> (act_enc);
    module::call ( act, mod );
    return we::util::text_codec::encode (act);
  }

  void handle_externally ( activity_t & act, const mod_t & module_call )
  {
    activity_t result ( we::util::text_codec::decode<activity_t> (fake_external (we::util::text_codec::encode(act), module_call)));
    act.set_output(result.output());
  }

  void handle_externally ( activity_t & act, const expr_t & e)
  {
    handle_internally ( act, e );
  }
};

int main (int ac, char ** av)
{
  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " pnet-file" << std::endl;
    return EXIT_SUCCESS;
  }

  std::ifstream ifs (av[1]);
  if (! ifs)
  {
    std::cerr << "could not open \""<< av[1] <<"\"" << std::endl;
    return EXIT_FAILURE;
  }

  activity_t act ( we::util::text_codec::decode<activity_t> (ifs) );

  std::cout << "act (initial):"
            << std::endl
            << act
            << std::endl;

  struct exec_context ctxt;
  act.execute (ctxt);

  std::cout << "act (final):"
            << std::endl
            << act
            << std::endl;

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
