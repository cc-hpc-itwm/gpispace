// mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/trans.hpp>
#include <we/type/token.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/signature.hpp>
#include <we/type/control.hpp>
#include <we/type/place.hpp>
#include <we/type/condition.hpp>
#include <we/type/literal/name.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <boost/serialization/nvp.hpp>

#include <iomanip>

// ************************************************************************* //

static long OFFSETS (2);
static long SUBVOLUMES_PER_OFFSET (3);
static long PACKAGES_PER_OFFSET (4);
static long BUNCHES_PER_PACKAGE (5);
static long STORES (3);
static long BUFFER_PER_SUBVOLUMEN (2);

static bool PRINT_MARKING (true);
static bool PRINT_FIRE (true);

static unsigned int SEED (3141);

// ************************************************************************* //

typedef unsigned int edge_t;

static edge_t mk_edge (void)
{
  static edge_t e (0);

  return e++;
}

// ************************************************************************* //

typedef std::pair<std::string,std::string> pair_t;
typedef std::map<pair_t,unsigned long> cnt_map_t;
typedef std::map<std::string,double> time_map_t;

static cnt_map_t cnt_map;
static time_map_t time_map;

struct transition_t
{
public:
  std::string name;
  mutable condition::type cond;

  transition_t ( const std::string & _name
               , const condition::type & _cond
               )
    : name(_name), cond(_cond)
  {}

  bool condition (Function::Condition::Traits<token::type>::choices_t & choices)
    const
  {
    cnt_map[pair_t(name, cond.expression())]++;

    time_map[cond.expression()] -= current_time();

    const bool ret (cond (choices));

    time_map[cond.expression()] += current_time();

    return ret;
  }

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(name);
    ar & BOOST_SERIALIZATION_NVP(cond);
  }
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<std::string> h;

  return h (t.name);
}

inline bool operator == (const transition_t & x, const transition_t & y)
{
  return x.name == y.name;
}

typedef petri_net::net<place::type, transition_t, edge_t, token::type> pnet_t;

// ************************************************************************* //

using std::cout;
using std::endl;

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "{" << *p << ": " << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "}" << endl;
    }
  cout << endl;
}

// ************************************************************************* //

class TransitionFunction
{
private:
  const std::string name;
  const std::string expression;
  const expr::parse::parser<signature::field_name_t> parser;
  expr::eval::context<signature::field_name_t> context;

  typedef boost::function<std::string (const petri_net::pid_t &)> translate_t;
  typedef boost::function<signature::type (const petri_net::pid_t &)> sig_t;
  const translate_t translate;
  const sig_t signature;

public:
  explicit TransitionFunction ( const std::string & _name
                              , const std::string & _expression
                              , const translate_t & _translate
                              , const sig_t & _sig
                              )
    : name (_name)
    , expression (_expression)
    , parser (expression)
    , context ()
    , translate (_translate)
    , signature (_sig)
  {}

  pnet_t::output_t operator () ( const pnet_t::input_t & input
                               , const pnet_t::output_descr_t & output_descr
                               )
  {
    cnt_map[pair_t(name,"")]++;

    time_map[name + ".bind"] -= current_time();

    for ( pnet_t::input_t::const_iterator top (input.begin())
        ; top != input.end()
        ; ++top
        )
      {
        const token::type 
          token (Function::Transition::get_token<token::type> (*top));

        const petri_net::pid_t
          pid (Function::Transition::get_pid<token::type> (*top));

        token.bind (translate (pid), context);
      }

    time_map[name + ".bind"] += current_time();

    if (PRINT_FIRE)
      {
        cout << "*** " << name;

        if (name == "tmp")
          cout << " package = " << context.value ("bunch_to_work");

        cout << endl;
      }

    time_map[name + ".eval"] -= current_time();

    parser.eval_all (context);

    time_map[name + ".eval"] += current_time();

    time_map[name + ".unbind"] -= current_time();

    pnet_t::output_t output;

    for ( pnet_t::output_descr_t::const_iterator out (output_descr.begin())
        ; out != output_descr.end()
        ; ++out
        )
      {
        const petri_net::pid_t pid (out->first);

        typedef 
          Function::Transition::Traits<token::type>::token_on_place_t top_t;

        output.push_back (top_t (token::type ( translate (pid)
                                             , signature (pid)
                                             , context
                                             )
                                , pid
                                )
                         );
      }

    time_map[name + ".unbind"] += current_time();

    return output;
  }
};

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      , const std::string & condition = "true"
                                      )
{
  return net.add_transition 
    ( transition_t ( name
                   , condition::type 
                     ( condition
                     , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
                     )
               )
    , TransitionFunction
      ( name
      , expression
      , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
      , boost::bind(&place::signature<pnet_t>, boost::ref(net), _1)
      )
    );
}

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ("offsets", po::value<long>(&OFFSETS)->default_value(OFFSETS), "number of offsets")
    ("subvolumes", po::value<long>(&SUBVOLUMES_PER_OFFSET)->default_value(SUBVOLUMES_PER_OFFSET), "number of subvolumes per offset")
    ("packages", po::value<long>(&PACKAGES_PER_OFFSET)->default_value(PACKAGES_PER_OFFSET), "number of packages per offset")
    ("bunches", po::value<long>(&BUNCHES_PER_PACKAGE)->default_value(BUNCHES_PER_PACKAGE), "number of bunches per package")
    ("buffers", po::value<long>(&BUFFER_PER_SUBVOLUMEN)->default_value(BUFFER_PER_SUBVOLUMEN), "number of bunch buffers per subvolume")
    ("stores", po::value<long>(&STORES)->default_value(STORES), "number of bunch stores")
    ("seed", po::value<unsigned int>(&SEED)->default_value(SEED), "seed for random number generator")
    ("print_marking", po::value<bool>(&PRINT_MARKING)->default_value(PRINT_MARKING), "print marking after each fire")
    ("print_fire", po::value<bool>(&PRINT_FIRE)->default_value(PRINT_FIRE), "print information about each firing")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      cout << desc << "\n";
      return EXIT_SUCCESS;
    }

  cout 
    << "OFFSETS               = " << OFFSETS << endl
    << "SUBVOLUMES_PER_OFFSET = " << SUBVOLUMES_PER_OFFSET << endl
    << "PACKAGES_PER_OFFSET   = " << PACKAGES_PER_OFFSET << endl
    << "BUNCHES_PER_PACKAGE   = " << BUNCHES_PER_PACKAGE << endl
    << "STORES                = " << STORES << endl
    << "BUFFER_PER_SUBVOLUMEN = " << BUFFER_PER_SUBVOLUMEN << endl
    << "SEED                  = " << SEED << endl
    << "PRINT_MARKING         = " << PRINT_MARKING << endl
    ;

  pnet_t net;

  // *********************************************************************** //
  // global constants

  petri_net::pid_t pid_OFFSETS
    (net.add_place (place::type ("OFFSETS", literal::LONG)));
  token::put (net, pid_OFFSETS, OFFSETS);

  petri_net::pid_t pid_PACKAGES_PER_OFFSET
    (net.add_place (place::type ("PACKAGES_PER_OFFSET", literal::LONG)));
  token::put (net, pid_PACKAGES_PER_OFFSET, PACKAGES_PER_OFFSET);

  petri_net::pid_t pid_BUNCHES_PER_PACKAGE
    (net.add_place (place::type ("BUNCHES_PER_PACKAGE", literal::LONG)));
  token::put (net, pid_BUNCHES_PER_PACKAGE, BUNCHES_PER_PACKAGE);

#if 0
  petri_net::pid_t pid_SUBVOLUMES_PER_OFFSET
    (net.add_place (place::type ("SUBVOLUMES_PER_OFFSET", literal::LONG)));
  token::put (net, pid_SUBVOLUMES_PER_OFFSET, SUBVOLUMES_PER_OFFSET);

  petri_net::pid_t pid_STORES
    (net.add_place (place::type ("STORES", literal::LONG)));
  token::put (net, pid_STORES, STORES);

  petri_net::pid_t pid_BUFFER_IN_SUBVOLUMEN
    (net.add_place (place::type ("BUFFER_IN_SUBVOLUMEN", literal::LONG)));
  token::put (net, pid_BUFFER_IN_SUBVOLUMEN, BUFFER_IN_SUBVOLUMEN);
#endif

  // *********************************************************************** //
  // generate offsets

  petri_net::pid_t pid_off_num
    (net.add_place (place::type ("off_num", literal::LONG)));
  petri_net::pid_t pid_off_state
    (net.add_place (place::type ("off_state", literal::LONG)));
  petri_net::pid_t pid_off_wait_for
    (net.add_place (place::type ("off_wait_for", literal::LONG)));

  signature::structured_t sig_pair_LL;
  
  sig_pair_LL["num"] = literal::LONG;
  sig_pair_LL["state"] = literal::LONG;

  petri_net::pid_t pid_off_pair
    (net.add_place (place::type ("off_pair", sig_pair_LL)));

  net.set_capacity (pid_off_pair, 1);

  petri_net::pid_t pid_off_to_work
    (net.add_place (place::type ("off_to_work", literal::LONG)));
  petri_net::pid_t pid_off_done
    (net.add_place (place::type ("off_done", literal::LONG)));
  petri_net::pid_t pid_off_all_done
    (net.add_place (place::type ("off_all_done")));

  petri_net::tid_t tid_off_init
    ( mk_transition
      ( net
      , "off_init"
      , "${off_num}      := ${OFFSETS} ;\
         ${off_state}    := 0L         ;\
         ${off_wait_for} := ${OFFSETS}  "
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_init, pid_OFFSETS));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_init, pid_off_num));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_init, pid_off_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_init, pid_off_wait_for));

  petri_net::tid_t tid_off_pair
    ( mk_transition
      ( net
      , "off_pair"
      , "${off_pair.num}      := ${off_num}   ;\
         ${off_pair.state}    := ${off_state} ;"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_pair, pid_off_num));
  net.add_edge (mk_edge(), connection_t (PT, tid_off_pair, pid_off_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_pair, pid_off_pair));

  petri_net::tid_t tid_off_break
    ( mk_transition
      ( net
      , "off_break"
      , ""
      , "${off_pair.state} >= ${off_pair.num}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_off_break, pid_off_pair));

  petri_net::tid_t tid_off_step
    ( mk_transition
      ( net
      , "off_step"
      , "${off_to_work} := ${off_pair.state}     ;\
         ${off_num}     := ${off_pair.num}       ;\
         ${off_state}   := ${off_pair.state} + 1  "
      , "${off_pair.state} < ${off_pair.num}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_off_step, pid_off_pair));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_step, pid_off_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_step, pid_off_num));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_step, pid_off_state));

  petri_net::tid_t tid_off_dec
    ( mk_transition
      ( net
      , "off_dec"
      , "${off_wait_for} := ${off_wait_for} - 1"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_off_dec, pid_off_done));
  net.add_edge (mk_edge(), connection_t (PT, tid_off_dec, pid_off_wait_for));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_dec, pid_off_wait_for));

  petri_net::tid_t tid_off_all_done
    ( mk_transition 
      ( net
      , "off_all_done"
      , "${off_all_done} := []"
      , "${off_wait_for} == 0L"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_all_done, pid_off_wait_for));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_all_done, pid_off_all_done));

  // *********************************************************************** //
  // generate packages

  signature::structured_t sig_package;

  sig_package["offset"] = literal::LONG;
  sig_package["package"] = literal::LONG;

  signature::structured_t sig_offset_count;

  sig_offset_count["offset"] = literal::LONG;
  sig_offset_count["count"] = literal::LONG;

  petri_net::pid_t pid_pack_num
    (net.add_place (place::type ("pack_num", sig_offset_count)));
  petri_net::pid_t pid_pack_state
    (net.add_place (place::type ("pack_state", sig_offset_count)));
  petri_net::pid_t pid_pack_wait_for
    (net.add_place (place::type ("pack_wait_for", sig_offset_count)));

  signature::structured_t sig_pair_ococ;
  
  sig_pair_ococ["num"] = sig_offset_count;
  sig_pair_ococ["state"] = sig_offset_count;

  petri_net::pid_t pid_pack_pair
    (net.add_place (place::type ("pack_pair", sig_pair_ococ)));

  net.set_capacity (pid_pack_pair, 1);

  petri_net::pid_t pid_pack_to_work
    (net.add_place (place::type ("pack_to_work", sig_package)));
  petri_net::pid_t pid_pack_done
    (net.add_place (place::type ("pack_done", sig_package)));

  petri_net::tid_t tid_pack_init
    ( mk_transition
      ( net
      , "pack_init"
      , "${pack_num.offset}      := ${off_to_work}         ;\
         ${pack_num.count}       := ${PACKAGES_PER_OFFSET} ;\
         ${pack_state.offset}    := ${off_to_work}         ;\
         ${pack_state.count}     := 0L                     ;\
         ${pack_wait_for.offset} := ${off_to_work}         ;\
         ${pack_wait_for.count}  := ${PACKAGES_PER_OFFSET}  "
      )
    );

  net.add_edge (mk_edge(), connection_t (PT_READ, tid_pack_init, pid_PACKAGES_PER_OFFSET));
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_init, pid_off_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_init, pid_pack_num));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_init, pid_pack_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_init, pid_pack_wait_for));

  petri_net::tid_t tid_pack_pair
    ( mk_transition
      ( net
      , "pack_pair"
      , "${pack_pair.num}      := ${pack_num}   ;\
         ${pack_pair.state}    := ${pack_state} ;"
      , "${pack_num.offset} == ${pack_state.offset}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_pack_pair, pid_pack_num));
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_pair, pid_pack_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_pair, pid_pack_pair));

  petri_net::tid_t tid_pack_break
    ( mk_transition
      ( net
      , "pack_break"
      , ""
      , "${pack_pair.state.count} >= ${pack_pair.num.count}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_break, pid_pack_pair));

  petri_net::tid_t tid_pack_step
    ( mk_transition
      ( net
      , "pack_step"
      , "${pack_to_work.offset}  := ${pack_pair.state.offset}   ;\
         ${pack_to_work.package} := ${pack_pair.state.count}    ;\
         ${pack_num}             := ${pack_pair.num}            ;\
         ${pack_state.offset}    := ${pack_pair.state.offset}   ;\
         ${pack_state.count}     := ${pack_pair.state.count} + 1 "
      , "${pack_pair.state.count} < ${pack_pair.num.count}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_step, pid_pack_pair));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_step, pid_pack_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_step, pid_pack_num));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_step, pid_pack_state));

  petri_net::tid_t tid_pack_dec
    ( mk_transition
      ( net
      , "pack_dec"
      , "${pack_wait_for.count} := ${pack_wait_for.count} - 1"
      , "${pack_wait_for.offset} == ${pack_done.offset}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_dec, pid_pack_done));
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_dec, pid_pack_wait_for));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_dec, pid_pack_wait_for));

  petri_net::tid_t tid_pack_all_done
    ( mk_transition 
      ( net
      , "pack_all_done"
      , "${off_done} := ${pack_wait_for.offset}"
      , "${pack_wait_for.count} == 0L"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_pack_all_done, pid_pack_wait_for));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_all_done, pid_off_done));

  // *********************************************************************** //
  // generate bunches

  signature::structured_t sig_bunch;

  sig_bunch["package"] = sig_package;
  sig_bunch["bunch"] = literal::LONG;

  signature::structured_t sig_package_count;

  sig_package_count["package"] = sig_package;
  sig_package_count["count"] = literal::LONG;

  petri_net::pid_t pid_bunch_num
    (net.add_place (place::type ("bunch_num", sig_package_count)));
  petri_net::pid_t pid_bunch_state
    (net.add_place (place::type ("bunch_state", sig_package_count)));
  petri_net::pid_t pid_bunch_wait_for
    (net.add_place (place::type ("bunch_wait_for", sig_package_count)));

  signature::structured_t sig_pair_pcpc;
  
  sig_pair_pcpc["num"] = sig_package_count;
  sig_pair_pcpc["state"] = sig_package_count;

  petri_net::pid_t pid_bunch_pair
    (net.add_place (place::type ("bunch_pair", sig_pair_pcpc)));

  net.set_capacity (pid_bunch_pair, 1);

  petri_net::pid_t pid_bunch_to_work
    (net.add_place (place::type ("bunch_to_work", sig_bunch)));
  petri_net::pid_t pid_bunch_done
    (net.add_place (place::type ("bunch_done", sig_bunch)));

  petri_net::tid_t tid_bunch_init
    ( mk_transition
      ( net
      , "bunch_init"
      , "${bunch_num.package}      := ${pack_to_work}        ;\
         ${bunch_num.count}        := ${BUNCHES_PER_PACKAGE} ;\
         ${bunch_state.package}    := ${pack_to_work}        ;\
         ${bunch_state.count}      := 0L                     ;\
         ${bunch_wait_for.package} := ${pack_to_work}        ;\
         ${bunch_wait_for.count}   := ${BUNCHES_PER_PACKAGE}  "
      )
    );

  net.add_edge (mk_edge(), connection_t (PT_READ, tid_bunch_init, pid_BUNCHES_PER_PACKAGE));
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_init, pid_pack_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_init, pid_bunch_num));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_init, pid_bunch_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_init, pid_bunch_wait_for));

  petri_net::tid_t tid_bunch_pair
    ( mk_transition
      ( net
      , "bunch_pair"
      , "${bunch_pair.num}      := ${bunch_num}   ;\
         ${bunch_pair.state}    := ${bunch_state} ;"
      , "${bunch_num.package} == ${bunch_state.package}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_pair, pid_bunch_num));
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_pair, pid_bunch_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_pair, pid_bunch_pair));

  petri_net::tid_t tid_bunch_break
    ( mk_transition
      ( net
      , "bunch_break"
      , ""
      , "${bunch_pair.state.count} >= ${bunch_pair.num.count}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_break, pid_bunch_pair));

  petri_net::tid_t tid_bunch_step
    ( mk_transition
      ( net
      , "bunch_step"
      , "${bunch_to_work.package} := ${bunch_pair.state.package}  ;\
         ${bunch_to_work.bunch}   := ${bunch_pair.state.count}    ;\
         ${bunch_num}             := ${bunch_pair.num}            ;\
         ${bunch_state.package}   := ${bunch_pair.state.package}  ;\
         ${bunch_state.count}     := ${bunch_pair.state.count} + 1 "
      , "${bunch_pair.state.count} < ${bunch_pair.num.count}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_step, pid_bunch_pair));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_step, pid_bunch_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_step, pid_bunch_num));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_step, pid_bunch_state));

  petri_net::tid_t tid_bunch_dec
    ( mk_transition
      ( net
      , "bunch_dec"
      , "${bunch_wait_for.count} := ${bunch_wait_for.count} - 1"
      , "${bunch_wait_for.package} == ${bunch_done.package}"
      )
    );
  
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_dec, pid_bunch_done));
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_dec, pid_bunch_wait_for));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_dec, pid_bunch_wait_for));

  petri_net::tid_t tid_bunch_all_done
    ( mk_transition 
      ( net
      , "bunch_all_done"
      , "${pack_done} := ${bunch_wait_for.package}"
      , "${bunch_wait_for.count} == 0L"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_all_done, pid_bunch_wait_for));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_all_done, pid_pack_done));

  // *********************************************************************** //

  petri_net::tid_t tid_tmp
    ( mk_transition
      ( net
      , "tmp"
      , "${bunch_done} := ${bunch_to_work}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_tmp, pid_bunch_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_tmp, pid_bunch_done));

  // *********************************************************************** //

  marking (net);

  {
    boost::mt19937 engine (SEED);

    Timer_t timer ("fire", OFFSETS * SUBVOLUMES_PER_OFFSET * BUNCHES_PER_PACKAGE * PACKAGES_PER_OFFSET);

    while (!net.enabled_transitions().empty())
      {
        net.fire_random(engine);

        if (PRINT_MARKING)
          marking (net);
      }
  }

  marking (net);

  cout << endl << "*** cnt_map:" << endl;

  for (cnt_map_t::const_iterator it (cnt_map.begin()); it != cnt_map.end(); ++it)
    cout << std::setw(8) << it->second
         << " [" << std::setw(15) << it->first.first << "]"
         << " " << it->first.second
         << endl;

  for (time_map_t::const_iterator it (time_map.begin()); it != time_map.end(); ++it)
    cout << std::setw(15) << it->second << " => " << it->first << endl;

  return EXIT_SUCCESS;
}
