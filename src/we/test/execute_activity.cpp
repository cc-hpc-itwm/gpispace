#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/port.hpp>
#include <we/type/activity.hpp>
#include <we/context.hpp>

#include <we/type/value.hpp>

#include <we/expr/eval/context.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include <util-generic/print_exception.hpp>

using we::edge::PT;
using we::edge::PT_READ;
using we::edge::TP;

typedef we::type::transition_t transition_t;
typedef we::type::net_type pnet_t;
typedef we::type::activity_t activity_t;
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

    context_t context;
    for ( input_t::const_iterator top (act.input().begin())
        ; top != act.input().end()
        ; ++top
        )
    {
      const pnet::type::value::value_type& token (top->first);
      const we::port_id_type& port_id (top->second);

      context.bind ( act.transition().ports_input().at (port_id).name()
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
      const we::port_id_type& port_id
        (act.transition().output_port_by_name (ton->second));

      act.add_output (port_id, ton->first);
    }
  }
}

struct exec_context : public we::context
{
  boost::mt19937 _engine;

  virtual void handle_internally (activity_t& act, net_t const&) override
  {
    // submit to self
    if (act.transition().net())
    {
      while ( boost::optional<we::type::activity_t> sub
            = boost::get<we::type::net_type> (act.transition().data())
            . fire_expressions_and_extract_activity_random
                ( _engine
                , [] (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
                  {
                    throw std::logic_error ("got unexpected workflow_response");
                  }
                )
            )
      {
        exec_context ctxt;
        sub->execute (&ctxt);
        act.inject (*sub);
      }
    }
  }

  virtual void handle_externally (activity_t& act, net_t const& n) override
  {
    handle_internally (act, n);
  }

  virtual void handle_externally (activity_t& act, mod_t const& module_call) override
  {
    module::call (act, module_call);
  }
};

int main (int ac, char ** av)
try
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
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return EXIT_FAILURE;
}
