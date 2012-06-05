// mirko.rahn@itwm.fraunhofer.de

#include <we/net_with_transition_function.hpp>
#include <we/function/trans.hpp>
#include <we/type/token.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/signature.hpp>
#include <we/type/control.hpp>
#include <we/type/place.hpp>
#include <we/type/condition.hpp>
#include <we/type/literal/name.hpp>

#include <we/util/stat.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <boost/serialization/nvp.hpp>

#include <iomanip>

// ************************************************************************* //

static statistic::loud<std::string> stat;

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
    stat.start (name, cond.expression());

    const bool ret (cond (choices));

    stat.stop (name, cond.expression());

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

typedef petri_net::net_with_transition_function< place::type
                                               , transition_t
                                               , edge_t
                                               , token::type
                                               > pnet_t;

// ************************************************************************* //

using std::cout;
using std::endl;

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "{"
           << *p << ": "
           << n.num_token (*p) << ": "
           << n.get_place (*p) << ":"
        ;

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
  const expr::parse::parser parser;
  expr::eval::context context;

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
    stat.start (name, "bind");

    for ( pnet_t::input_t::const_iterator top (input.begin())
        ; top != input.end()
        ; ++top
        )
      {
        const token::type
          token (Function::Transition::get_token<token::type> (*top));

        const petri_net::pid_t
          pid (Function::Transition::get_pid<token::type> (*top));

        context.bind (translate (pid), token.value);
      }

    stat.stop (name, "bind");

    if (PRINT_FIRE)
      {
        cout << "*** " << name;

        if (name == "tmp")
          cout << " package = " << context.value ("bunch_to_work");

        cout << endl;
      }

    stat.start (name, "eval");

    parser.eval_all (context);

    stat.stop (name, "eval");

    stat.start (name, "unbind");

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

    stat.stop (name, "unbind");

    return output;
  }
};

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      , const std::string & condition = "true"
                                      )
{
  const petri_net::tid_t tid
    ( net.add_transition
      ( transition_t ( name
                     , condition::type
                       ( condition
                       , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
                       )
                     )
      )
    );

  net.set_transition_function
    ( tid
    , TransitionFunction
      ( name
      , expression
      , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
      , boost::bind(&place::signature<pnet_t>, boost::ref(net), _1)
      )
    );

  return tid;
}

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::PT_READ;
using petri_net::edge::TP;

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
    << "PRINT_MARKING         = " << PRINT_FIRE << endl
    ;

  pnet_t net;

  // *********************************************************************** //
  // global constants

  petri_net::pid_t pid_OFFSETS
    (net.add_place (place::type ("OFFSETS", literal::LONG())));
  token::put (net, pid_OFFSETS, OFFSETS);

  petri_net::pid_t pid_PACKAGES_PER_OFFSET
    (net.add_place (place::type ("PACKAGES_PER_OFFSET", literal::LONG())));
  token::put (net, pid_PACKAGES_PER_OFFSET, PACKAGES_PER_OFFSET);

  petri_net::pid_t pid_BUNCHES_PER_PACKAGE
    (net.add_place (place::type ("BUNCHES_PER_PACKAGE", literal::LONG())));
  token::put (net, pid_BUNCHES_PER_PACKAGE, BUNCHES_PER_PACKAGE);

#if 0
  petri_net::pid_t pid_SUBVOLUMES_PER_OFFSET
    (net.add_place (place::type ("SUBVOLUMES_PER_OFFSET", literal::LONG())));
  token::put (net, pid_SUBVOLUMES_PER_OFFSET, SUBVOLUMES_PER_OFFSET);

  petri_net::pid_t pid_STORES
    (net.add_place (place::type ("STORES", literal::LONG())));
  token::put (net, pid_STORES, STORES);

  petri_net::pid_t pid_BUFFER_IN_SUBVOLUMEN
    (net.add_place (place::type ("BUFFER_IN_SUBVOLUMEN", literal::LONG())));
  token::put (net, pid_BUFFER_IN_SUBVOLUMEN, BUFFER_IN_SUBVOLUMEN);
#endif

  // *********************************************************************** //
  // generate offsets

  signature::structured_t sig_state;

  sig_state["num"] = literal::LONG();
  sig_state["state"] = literal::LONG();

  petri_net::pid_t pid_off_state
    (net.add_place (place::type ("off_state", sig_state)));

  petri_net::pid_t pid_off_try
    (net.add_place (place::type ("off_try", sig_state)));

  petri_net::pid_t pid_off_to_work
    (net.add_place (place::type ("off_to_work", literal::LONG())));

  petri_net::tid_t tid_off_init
    ( mk_transition
      ( net
      , "off_init"
      , "${off_state.num}   := ${OFFSETS} ;\
         ${off_state.state} := 0L          "
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_init, pid_OFFSETS));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_init, pid_off_state));

  petri_net::tid_t tid_off_try
    ( mk_transition
      ( net
      , "off_try"
      , "${off_try} := ${off_state}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_try, pid_off_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_try, pid_off_try));

  petri_net::tid_t tid_off_break
    ( mk_transition
      ( net
      , "off_break"
      , ""
      , "${off_try.state} >= ${off_try.num}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_break, pid_off_try));

  petri_net::tid_t tid_off_step
    ( mk_transition
      ( net
      , "off_step"
      , "${off_to_work}     := ${off_try.state}     ;\
         ${off_state}       := ${off_try}           ;\
         ${off_state.state} := ${off_try.state} + 1  "
      , "${off_try.state} < ${off_try.num}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_off_step, pid_off_try));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_step, pid_off_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_off_step, pid_off_state));

  // *********************************************************************** //
  // generate packages

  signature::structured_t sig_package;

  sig_package["offset"] = literal::LONG();
  sig_package["package"] = literal::LONG();

  signature::structured_t sig_offset_with_state;

  sig_offset_with_state["offset"] = literal::LONG();
  sig_offset_with_state["state"] = sig_state;

  petri_net::pid_t pid_pack_state
    (net.add_place (place::type ("pack_state", sig_offset_with_state)));

  petri_net::pid_t pid_pack_try
    (net.add_place (place::type ("pack_try", sig_offset_with_state)));

  petri_net::pid_t pid_pack_to_work
    (net.add_place (place::type ("pack_to_work", sig_package)));

  petri_net::tid_t tid_pack_init
    ( mk_transition
      ( net
      , "pack_init"
      , "${pack_state.offset}      := ${off_to_work}         ;\
         ${pack_state.state.num}   := ${PACKAGES_PER_OFFSET} ;\
         ${pack_state.state.state} := 0L                      "
      )
    );

  net.add_edge (mk_edge(), connection_t (PT_READ, tid_pack_init, pid_PACKAGES_PER_OFFSET));
  net.add_edge (mk_edge(), connection_t (PT, tid_pack_init, pid_off_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_init, pid_pack_state));

  petri_net::tid_t tid_pack_try
    ( mk_transition
      ( net
      , "pack_try"
      , "${pack_try} := ${pack_state}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_pack_try, pid_pack_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_try, pid_pack_try));

  petri_net::tid_t tid_pack_break
    ( mk_transition
      ( net
      , "pack_break"
      , ""
      , "${pack_try.state.state} >= ${pack_try.state.num}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_pack_break, pid_pack_try));

  petri_net::tid_t tid_pack_step
    ( mk_transition
      ( net
      , "pack_step"
      , "${pack_to_work.offset}    := ${pack_try.offset}          ;\
         ${pack_to_work.package}   := ${pack_try.state.state}     ;\
         ${pack_state}             := ${pack_try}                 ;\
         ${pack_state.state.state} := ${pack_try.state.state} + 1  "
      , "${pack_try.state.state} < ${pack_try.state.num}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_pack_step, pid_pack_try));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_step, pid_pack_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_pack_step, pid_pack_state));

  // *********************************************************************** //
  // generate bunches

  signature::structured_t sig_bunch;

  sig_bunch["package"] = sig_package;
  sig_bunch["bunch"] = literal::LONG();

  signature::structured_t sig_package_with_state;

  sig_package_with_state["package"] = sig_package;
  sig_package_with_state["state"] = sig_state;

  petri_net::pid_t pid_bunch_state
    (net.add_place (place::type ("bunch_state", sig_package_with_state)));

  petri_net::pid_t pid_bunch_try
    (net.add_place (place::type ("bunch_try", sig_package_with_state)));

  petri_net::pid_t pid_bunch_to_work
    (net.add_place (place::type ("bunch_to_work", sig_bunch)));

  petri_net::tid_t tid_bunch_init
    ( mk_transition
      ( net
      , "bunch_init"
      , "${bunch_state.package}     := ${pack_to_work}        ;\
         ${bunch_state.state.num}   := ${BUNCHES_PER_PACKAGE} ;\
         ${bunch_state.state.state} := 0L                      "
      )
    );

  net.add_edge (mk_edge(), connection_t (PT_READ, tid_bunch_init, pid_BUNCHES_PER_PACKAGE));
  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_init, pid_pack_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_init, pid_bunch_state));

  petri_net::tid_t tid_bunch_try
    ( mk_transition
      ( net
      , "bunch_try"
      , "${bunch_try} := ${bunch_state}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_try, pid_bunch_state));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_try, pid_bunch_try));

  petri_net::tid_t tid_bunch_break
    ( mk_transition
      ( net
      , "bunch_break"
      , ""
      , "${bunch_try.state.state} >= ${bunch_try.state.num}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_break, pid_bunch_try));

  petri_net::tid_t tid_bunch_step
    ( mk_transition
      ( net
      , "bunch_step"
      , "${bunch_to_work.package}   := ${bunch_try.package}         ;\
         ${bunch_to_work.bunch}     := ${bunch_try.state.state}     ;\
         ${bunch_state}             := ${bunch_try}                 ;\
         ${bunch_state.state.state} := ${bunch_try.state.state} + 1  "
      , "${bunch_try.state.state} < ${bunch_try.state.num}"
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_bunch_step, pid_bunch_try));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_step, pid_bunch_to_work));
  net.add_edge (mk_edge(), connection_t (TP, tid_bunch_step, pid_bunch_state));

  // *********************************************************************** //

  petri_net::tid_t tid_tmp
    ( mk_transition
      ( net
      , "tmp"
      , ""
      )
    );

  net.add_edge (mk_edge(), connection_t (PT, tid_tmp, pid_bunch_to_work));

  // *********************************************************************** //

  marking (net);

  {
    boost::mt19937 engine (SEED);

    Timer_t timer ("fire", OFFSETS * SUBVOLUMES_PER_OFFSET * BUNCHES_PER_PACKAGE * PACKAGES_PER_OFFSET);

    while (!net.enabled_transitions().empty())
      {
        net.fire_random (engine); // (net.enabled_transitions().first());

        if (PRINT_MARKING)
          marking (net);
      }
  }

  marking (net);

  stat.out ("Kirchhoff");

  return EXIT_SUCCESS;
}
