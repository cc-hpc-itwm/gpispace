#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/port.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/mgmt/context.hpp>

#include <we2/type/value.hpp>
#include <we2/require_type.hpp>

#include <we/expr/eval/context.hpp>

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
  template <typename ModuleCall, typename Context, typename OutputList>
  void eval (const ModuleCall & mf, const Context & ctxt, OutputList & output)
  {
    if ( mf.module() == "dummy" && mf.function() == "dummy" )
    {
      const long result
        (dummy::dummy (boost::get<long> (ctxt.value ("input"))));
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
      const pnet::type::value::value_type& token (top->first);
      const petri_net::port_id_type& port_id (top->second);

      context.bind ( act.transition().name_of_port (port_id)
                   , token
                   );
    }

    typedef std::vector <std::pair< pnet::type::value::value_type
                                  , std::string
                                  >
                        > mod_output_t;

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
          ( pnet::require_type_relaxed
            ( ton->first
            , port.signature()
            , port.name()
            )
          , port_id
          )
        );
    }
  }
}

struct exec_context : public we::mgmt::context
{
  virtual int handle_internally (activity_t& act, net_t&)
  {
    act.inject_input ();

    // submit to self
    while (act.can_fire())
    {
      activity_t sub = act.extract ();

      sub.execute (this);

      act.inject (sub);

    }

    act.collect_output();

    return 0;
  }

  virtual int handle_internally (activity_t&, mod_t&)
  {
    throw std::runtime_error ("cannot handle module calls internally");
  }

  virtual int handle_internally (activity_t&, expr_t&)
  {
    // nothing to do
    return 0;
  }

  std::string fake_external (const std::string& act_enc, net_t& n)
  {
    activity_t act (act_enc);
    handle_internally (act, n );
    return act.to_string();
  }

  virtual int handle_externally (activity_t& act, net_t& n)
  {
    activity_t result (fake_external (act.to_string(), n));
    act.set_output(result.output());
    return 0;
  }

  std::string fake_external (const std::string& act_enc, const mod_t& mod)
  {
    activity_t act (act_enc);
    module::call (act, mod );
    return act.to_string();
  }

  virtual int handle_externally (activity_t& act, mod_t& module_call)
  {
    activity_t result (fake_external (act.to_string(), module_call));
    act.set_output(result.output());
    return 0;
  }

  virtual int handle_externally (activity_t& act, expr_t& e)
  {
    return handle_internally (act, e );
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

  activity_t act (ifs);

  exec_context ctxt;
  act.execute (&ctxt);

  if (act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
