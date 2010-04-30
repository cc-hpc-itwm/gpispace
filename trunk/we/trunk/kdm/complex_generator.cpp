#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/mgmt/type/activity.hpp>

#include "complex_generator.hpp"

using petri_net::connection_t;
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

typedef place::type place_t;
typedef token::type token_t;
typedef unsigned int edge_t;
typedef we::type::transition_t<place_t, edge_t, token_t> transition_t;
typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;
typedef we::mgmt::type::activity_t<transition_t, token_t> activity_t;
typedef activity_t::input_t input_t;

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
    throw std::runtime_error ("bad_get: expected literal, got: " + util::show (o));
  }
};

template <typename T, typename V>
T get_value_of_literal (const V & v)
{
  return boost::apply_visitor (visitor_get_value_of_literal<T>(), v);
}

namespace kdm
{
  static ::value::type initialize (const std::string & filename, long & wait)
  {
    ::value::structured_t config;

    std::cout << "initialize: use file " << filename << std::endl;

    std::ifstream file (filename.c_str());

    if (!file)
      throw std::runtime_error ("Lurcks, config file not good!?");

    while (!file.eof())
      {
        std::string s;
        file >> s;
        long v;
        file >> v;
        if (s.size())
          config[s] = v;
      }

    std::cout << "initialize: got config " << config << std::endl;

    wait = get_value_of_literal<long> 
      (::value::get_field ("SUBVOLUMES_PER_OFFSET", config))
      ;

    std::cout << "initialize: wait = " << wait << std::endl;

    return config;
  }

  static void loadTT (const ::value::type & v)
  {
    std::cout << "loadTT: got config " << v << std::endl;
  }

  static void finalize (const ::value::type & v)
  {
    std::cout << "finalize: got config " << v << std::endl;
  }

  static ::value::type load ( const ::value::type & config
                            , const ::value::type & bunch
                            , const long & store
                            )
  {
    std::cout << "load: got config " << config << std::endl;
    std::cout << "load: got bunch " << bunch << std::endl;
    std::cout << "load: got store " << store << std::endl;

    ::value::structured_t loaded_bunch;

    loaded_bunch["bunch"] = bunch;
    loaded_bunch["store"] = store;
    loaded_bunch["seen"] = literal::type (bitsetofint::type());
    loaded_bunch["wait"] = get_value_of_literal<long> 
      (::value::get_field ("SUBVOLUMES_PER_OFFSET", config))
      ;

    std::cout << "load: loaded_bunch " << loaded_bunch << std::endl;

    return loaded_bunch;
  }

  static void write (const ::value::type & config, const ::value::type & volume)
  {
    std::cout << "write: got config " << config << std::endl;
    std::cout << "write: got volume " << volume << std::endl;
  }

  static ::value::type
  process ( const ::value::type & config
          , const ::value::type & volume
          )
  {
    std::cout << "process: got config " << config << std::endl;
    std::cout << "process: got volume " << volume << std::endl;

    ::value::type volume_processed (volume);
    
    const ::value::type buffer0 (::value::get_field ("buffer0", volume));
    const bool assigned0
      (get_value_of_literal<bool>(::value::get_field ("assigned", buffer0)));
    const bool filled0
      (get_value_of_literal<bool>(::value::get_field ("filled", buffer0)));

    const ::value::type buffer1 (::value::get_field ("buffer1", volume));
    const bool assigned1
      (get_value_of_literal<bool>(::value::get_field ("assigned", buffer1)));
    const bool filled1
      (get_value_of_literal<bool>(::value::get_field ("filled", buffer1)));

    // die hier implementierte Logik ist noch nicht optimal: es wird
    // einfach nur jeder bufffer geladen, der assigned aber nicht
    // gefüllt ist und die buffer, die geladen sind, werden verarbeitet.

    const long wait
      (get_value_of_literal<long>(::value::get_field ("wait", volume)));

    ::value::field("assigned", ::value::field("buffer0", volume_processed)) = assigned0 && !filled0;
    ::value::field("filled", ::value::field("buffer0", volume_processed)) = assigned0 && !filled0;
    ::value::field("free", ::value::field("buffer0", volume_processed)) = assigned0 && !filled0;
    ::value::field("assigned", ::value::field("buffer1", volume_processed)) = assigned1 && !filled1;
    ::value::field("filled", ::value::field("buffer1", volume_processed)) = assigned1 && !filled1;
    ::value::field("free", ::value::field("buffer1", volume_processed)) = assigned1 && !filled1;
    ::value::field("wait", volume_processed) = wait - ((filled0) ? 1 : 0) - ((filled1) ? 1 : 0);

    return volume_processed;
  }
}

namespace module
{
  template <typename ModuleCall, typename Context, typename OutputList>
  void eval (const ModuleCall & mf, const Context & ctxt, OutputList & output)
  {
    if (mf.module() == "kdm")
      {
   
        if (mf.function() == "loadTT")
          {
            const value::type & v (ctxt.value("config"));
            kdm::loadTT (v);
            output.push_back (std::make_pair (control(), "trigger"));
          }

        else if (mf.function() == "initialize")
          {
            const std::string filename 
              (get_value_of_literal<std::string> (ctxt.value("config_file")));
            long wait (0);
            const value::type config (kdm::initialize (filename, wait));
            output.push_back (std::make_pair (config, "config"));
            output.push_back (std::make_pair (literal::type(wait), "wait"));
            output.push_back (std::make_pair (control(), "trigger"));
            bitsetofint::type bs;
            bs.ins (0);
            output.push_back (std::make_pair (literal::type(bs), "wanted"));
          }

        else if (mf.function() == "finalize")
          {
            const value::type & v (ctxt.value("config"));
            kdm::finalize (v);
            output.push_back (std::make_pair (control(), "done"));
          }

        else if (mf.function() == "load")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & bunch (ctxt.value("bunch"));
            const long & store
              (get_value_of_literal<long> (ctxt.value("empty_store")));
            const value::type loaded_bunch (kdm::load (config, bunch, store));
            output.push_back (std::make_pair (loaded_bunch, "loaded_bunch"));
          }

        else if (mf.function() == "write")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & volume (ctxt.value("volume_to_be_written"));
            kdm::write (config, volume);
            output.push_back (std::make_pair (volume, "volume_written"));
          }

        else if (mf.function() == "process")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & volume (ctxt.value("volume"));
            const value::type volume_processed (kdm::process (config, volume));

            output.push_back ( std::make_pair ( volume_processed
                                              , "volume_processed"
                                              )
                             );
          }
      }
  }

  static void call (activity_t & act, const transition_t::mod_type & module_call)
  {
    we::mgmt::type::detail::printer<activity_t, std::ostream> printer (act, std::cout);

    // construct context
    typedef expr::eval::context <signature::field_name_t> context_t;
    typedef activity_t::input_t input_t;
    typedef activity_t::output_t output_t;
    typedef activity_t::token_type token_type;
    typedef activity_t::transition_type::port_id_t port_id_t;
    typedef activity_t::transition_type::port_t port_t;
    typedef activity_t::transition_type::const_iterator port_iterator;

    context_t context;
    for ( input_t::const_iterator top (act.input().begin())
        ; top != act.input().end()
        ; ++top
        )
    {
      const token_type token   = top->first;
      const port_id_t  port_id = top->second;

      context.bind
        ( we::type::detail::translate_port_to_name ( act.transition()
                                                   , port_id
                                                   )
        , token.value
        );
    }

    typedef std::vector <std::pair<value::type, std::string> > mod_output_t;

    mod_output_t mod_output;

    module::eval ( module_call, context, mod_output );

    for ( mod_output_t::const_iterator ton (mod_output.begin())
        ; ton != mod_output.end()
        ; ++ton
        )
    {
      try
      {
        const port_id_t port_id =
          we::type::detail::translate_name_to_output_port (act.transition(), ton->second);

        const port_t & port =
          act.transition().get_port (port_id);

        act.output().push_back
          ( std::make_pair
            ( token_type ( port.name()
                         , port.signature()
                         , ton->first
                         )
            , port_id
            )
          );
      }
      catch (const std::exception & e)
      {
        std::cout << "During collect output: " << e.what() << std::endl;

        throw;
      }
    }
  }
}

struct exec_context
{
  typedef transition_t::net_type net_t;
  typedef transition_t::mod_type mod_t;
  typedef transition_t::expr_type expr_t;

  void handle_internally ( activity_t & act, net_t &)
  {
    act.inject_input ();

    // submit to self
    while (act.has_enabled())
    {
//       std::cout << "***** act (pre-extract):"
//                 << std::endl
//                 << act
//                 << std::endl;

      activity_t sub = act.extract ();

//       std::cout << "***** sub-act (pre-execute):"
//                 << std::endl
//                 << sub
//                 << std::endl;

      sub.execute (*this);

//       std::cout << "***** sub-act (post-execute):"
//                 << std::endl
//                 << sub
//                 << std::endl;

      act.inject (sub);

//       std::cout << "***** act (post-inject):"
//                 << std::endl
//                 << act
//                 << std::endl;
    }

    we::mgmt::visitor::output_collector<activity_t> collect_output (act);
    boost::apply_visitor ( collect_output
                         , act.transition().data()
                         );
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
    act = result;
    //    act.output().swap (result.output());
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
    act = result;
    //    act.output().swap (result.output());
  }

  void handle_externally ( activity_t & act, const expr_t & e)
  {
    handle_internally ( act, e );
  }
};

int main (int argc, char ** argv)
{
  transition_t kdm_complex (kdm::kdm<activity_t>::generate());

  activity_t act (kdm_complex);

  act.input().push_back 
    ( input_t::value_type
      ( token_t ( "config_file"
                , literal::STRING
                , std::string ((argc > 1) ? argv[1] : "/scratch/KDM.conf")
                )
      , kdm_complex.input_port_by_name ("config_file")
      )
    );

  // dump activity for test purposes
  {
    std::ofstream ofs ("kdm_complex.pnet");
    ofs << we::util::text_codec::encode (act);
  }

//   std::cout << "act (initial):"
//             << std::endl
//             << act
//             << std::endl;

  struct exec_context ctxt;
  act.execute (ctxt);

  we::mgmt::type::detail::printer<activity_t, std::ostream> printer (act, std::cout);
  printer << act.output();
  std::cout << std::endl;

//   std::cout << "act (final):"
//             << std::endl
//             << act
//             << std::endl;

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
