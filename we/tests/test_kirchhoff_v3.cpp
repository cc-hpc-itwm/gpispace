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

static statistic::muted<std::string> stat;

// ************************************************************************* //

static long BUNCHES_PER_PACKAGE (4);
static long NUMBER_OF_NODES (8);
static long OFFSETS (2);
static long PACKAGES_PER_OFFSET (3);
static long STORES (8);
static long SUBVOLUMES_PER_OFFSET (3);
static long SUBVOLUME_MULTIPLICITY (2);

static bool PRINT_MARKING (true);
static bool PRINT_FIRE (true);

static unsigned int SEED (3141);

// ************************************************************************* //

typedef unsigned int edge_t;

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
           << endl
        ;

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp << endl;

      cout << "}" << endl;
    }
  cout << endl;
}

// ************************************************************************* //

typedef expr::eval::context context_t;

class TransitionFunction
{
private:
  const std::string name;
  const std::string expression;
  const expr::parse::parser parser;
  context_t context;

  typedef boost::function<std::string (const petri_net::pid_t &)> translate_t;
  typedef boost::function<signature::type (const petri_net::pid_t &)> sig_t;
  const translate_t translate;
  const sig_t signature;

public:
  explicit TransitionFunction ( const std::string & _name
                              , const std::string & _expression
                              , const translate_t & _translate
                              , const sig_t & _sig
                              , const context_t & _context
                              )
    : name (_name)
    , expression (_expression)
    , parser (expression)
    , context (_context)
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

static std::string strip (const std::string & s)
{
  std::string r;

  for (std::string::const_iterator pos (s.begin()); pos != s.end(); ++pos)
    if (!isspace(*pos))
      r.push_back (*pos);

  return r;
}

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      , const std::string & condition = "true"
                                      , const context_t & context = context_t()
                                      )
{
  const petri_net::tid_t tid
    ( net.add_transition
      ( transition_t ( name
                     , condition::type
                       ( strip (condition)
                       , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
                       )
                     )
      )
    );

  net.set_transition_function
    ( tid
    , TransitionFunction
      ( name
      , strip (expression)
      , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
      , boost::bind(&place::signature<pnet_t>, boost::ref(net), _1)
      , context
      )
    );

  return tid;
}

static petri_net::pid_t
mk_place ( pnet_t & net
         , const std::string & name
         , const literal::type_name_t & sig = literal::CONTROL()
         )
{
  return net.add_place (place::type (name, sig));
}

static petri_net::pid_t
mk_place ( pnet_t & net
         , const std::string & name
         , const signature::structured_t & sig
         )
{
  return net.add_place (place::type (name, sig));
}

static void
mk_edge (pnet_t & net, const petri_net::connection_t & c)
{
  static edge_t e (0); net.add_edge (e++, c);
}

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::PT_READ;
using petri_net::edge::TP;

namespace po = boost::program_options;

// *********************************************************************** //

namespace signature
{
  static structured_t config;
  static structured_t state;
  static structured_t copy;
  static structured_t volume;
  static structured_t volume_with_count;
  static structured_t offset_with_state;
  static structured_t package;
  static structured_t package_with_state;
  static structured_t bunch;
  static structured_t loaded_bunch;
  static structured_t buffer;

  static void init (void)
  {
    config["BUNCHES_PER_PACKAGE"] = literal::LONG();
    config["NUMBER_OF_NODES"] = literal::LONG();
    config["OFFSETS"] = literal::LONG();
    config["PACKAGES_PER_OFFSET"] = literal::LONG();
    config["STORES"] = literal::LONG();
    config["SUBVOLUMES_PER_OFFSET"] = literal::LONG();
    config["SUBVOLUME_MULTIPLICITY"] = literal::LONG();

    state["num"] = literal::LONG();
    state["state"] = literal::LONG();

    copy["id"] = literal::LONG();
    copy["of"] = literal::LONG();

    package["id"] = literal::LONG();
    package["offset"] = literal::LONG();

    bunch["id"] = literal::LONG();
    bunch["package"] = package;

    buffer["filled"] = literal::BOOL();
    buffer["assigned"] = literal::BOOL();
    buffer["processed"] = literal::BOOL();
    buffer["bunch"] = bunch;
    buffer["store"] = literal::LONG();

    volume["id"] = literal::LONG();
    volume["copy"] = copy;
    volume["offset"] = literal::LONG();
    volume["buffer_0"] = buffer;
    volume["buffer_1"] = buffer;

    volume_with_count["volume"] = literal::LONG();
    volume_with_count["offset"] = literal::LONG();
    volume_with_count["wait_bunch"] = literal::LONG();
    volume_with_count["wait_copy"] = literal::LONG();

    offset_with_state["offset"] = literal::LONG();
    offset_with_state["state"] = state;

    package_with_state["package"] = package;
    package_with_state["state"] = state;

    loaded_bunch["bunch"] = bunch;
    loaded_bunch["store"] = literal::LONG();
    loaded_bunch["seen"] = literal::BITSET();
    loaded_bunch["wait"] = literal::LONG();
  }
}

// *********************************************************************** //

int
main (int argc, char ** argv)
{
  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ("bunches", po::value<long>(&BUNCHES_PER_PACKAGE)->default_value(BUNCHES_PER_PACKAGE), "number of bunches per package")
    ("nodes", po::value<long>(&NUMBER_OF_NODES)->default_value(NUMBER_OF_NODES), "number of nodes")
    ("offsets", po::value<long>(&OFFSETS)->default_value(OFFSETS), "number of offsets")
    ("packages", po::value<long>(&PACKAGES_PER_OFFSET)->default_value(PACKAGES_PER_OFFSET), "number of packages per offset")
    ("stores", po::value<long>(&STORES)->default_value(STORES), "number of bunch stores")
    ("subvolumes", po::value<long>(&SUBVOLUMES_PER_OFFSET)->default_value(SUBVOLUMES_PER_OFFSET), "number of subvolumes per offset")
    ("multiplicity", po::value<long>(&SUBVOLUME_MULTIPLICITY)->default_value(SUBVOLUME_MULTIPLICITY), "multiplicity of subvolumes")

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
    << "BUNCHES_PER_PACKAGE    = " << BUNCHES_PER_PACKAGE << endl
    << "NUMBER_OF_NODES        = " << NUMBER_OF_NODES << endl
    << "OFFSETS                = " << OFFSETS << endl
    << "PACKAGES_PER_OFFSET    = " << PACKAGES_PER_OFFSET << endl
    << "STORES                 = " << STORES << endl
    << "SUBVOLUMES_PER_OFFSET  = " << SUBVOLUMES_PER_OFFSET << endl
    << "SUBVOLUME_MULTIPLICITY = " << SUBVOLUME_MULTIPLICITY << endl

    << "SEED                   = " << SEED << endl

    << "PRINT_MARKING          = " << PRINT_MARKING << endl
    << "PRINT_FIRE             = " << PRINT_FIRE << endl
    ;

  pnet_t net;

  signature::init();

  using petri_net::pid_t;
  using petri_net::tid_t;

  // *********************************************************************** //
  // place

  pid_t pid_config_file (mk_place (net, "config_file", literal::STRING()));
  pid_t pid_config (mk_place (net, "config", signature::config));

  pid_t pid_trigger_gen_store (mk_place (net, "trigger_gen_store"));
  pid_t pid_gen_store_state
    (mk_place (net, "gen_store_state", signature::state));
  pid_t pid_empty_store (mk_place (net, "empty_store", literal::LONG()));

  pid_t pid_trigger_gen_offset (mk_place (net, "trigger_gen_offset"));
  pid_t pid_gen_offset_state
    (mk_place (net, "gen_offset_state", signature::state));
  pid_t pid_offset (mk_place (net, "offset", literal::LONG()));

  pid_t pid_wanted_offset (mk_place (net, "wanted_offset", literal::BITSET()));

  pid_t pid_trigger_gen_volume (mk_place (net, "trigger_gen_volume"));

  pid_t pid_gen_volume_state
    (mk_place (net, "gen_volume_state", signature::state));

  pid_t pid_buffer_empty (mk_place (net, "buffer_empty", signature::buffer));
  pid_t pid_volume_state (mk_place (net, "volume_state", signature::volume));
  pid_t pid_volume (mk_place (net, "volume", signature::volume));
  pid_t pid_volume_count (mk_place (net, "volume_count", signature::volume_with_count));
  pid_t pid_gen_volume_wait (mk_place (net, "gen_volume_wait", literal::LONG()));

  pid_t pid_gen_package_state
    (mk_place (net, "gen_package_state", signature::offset_with_state));

  pid_t pid_package (mk_place (net, "package", signature::package));

  pid_t pid_gen_bunch_state
    (mk_place (net, "gen_bunch_state", signature::package_with_state));

  pid_t pid_bunch (mk_place (net, "bunch", signature::bunch));

  pid_t pid_loaded_bunch (mk_place (net, "loaded_bunch", signature::loaded_bunch));

  // *********************************************************************** //
  // transition

  tid_t tid_gen_config
    ( mk_transition
      ( net
      , "gen_config"
      , "${config.BUNCHES_PER_PACKAGE} := " + fhg::util::show(BUNCHES_PER_PACKAGE) + ";"
      + "${config.NUMBER_OF_NODES} := " + fhg::util::show(NUMBER_OF_NODES) + ";"
      + "${config.OFFSETS} := " + fhg::util::show (OFFSETS) + ";"
      + "${config.PACKAGES_PER_OFFSET} := " + fhg::util::show(PACKAGES_PER_OFFSET) + ";"
      + "${config.STORES} := " + fhg::util::show (STORES) + ";"
      + "${config.SUBVOLUMES_PER_OFFSET} := " + fhg::util::show(SUBVOLUMES_PER_OFFSET) + ";"
      + "${config.SUBVOLUME_MULTIPLICITY} := " + fhg::util::show(SUBVOLUME_MULTIPLICITY) +";"
      + "${trigger_gen_store} := []"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_config, pid_config_file));
  mk_edge (net, connection_t (TP, tid_gen_config, pid_config));
  mk_edge (net, connection_t (TP, tid_gen_config, pid_trigger_gen_store));

  // *********************************************************************** //

  tid_t tid_gen_store
    ( mk_transition
      ( net
      , "gen_store"
      , "${gen_store_state.num} := ${config.STORES};\
         ${gen_store_state.state} := 0L"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_store, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_store, pid_trigger_gen_store));

  mk_edge (net, connection_t (TP, tid_gen_store, pid_gen_store_state));

  tid_t tid_gen_store_step
    ( mk_transition
      ( net
      , "gen_store_step"
      , "${empty_store} := ${gen_store_state.state};\
         ${gen_store_state.state} := ${gen_store_state.state} + 1;"
      , "${gen_store_state.state} < ${gen_store_state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_store_step, pid_gen_store_state));
  mk_edge (net, connection_t (TP, tid_gen_store_step, pid_gen_store_state));
  mk_edge (net, connection_t (TP, tid_gen_store_step, pid_empty_store));

  tid_t tid_gen_store_break
    ( mk_transition
      ( net
      , "gen_store_break"
      , "${trigger_gen_volume} := []"
      , "${gen_store_state.state} >= ${gen_store_state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_store_break, pid_gen_store_state));
  mk_edge (net, connection_t (TP, tid_gen_store_break, pid_trigger_gen_volume));

  // *********************************************************************** //

  tid_t tid_gen_volume
    ( mk_transition
      ( net
      , "gen_volume"
      , "${gen_volume_state.num} := ${config.SUBVOLUMES_PER_OFFSET};\
         ${gen_volume_state.state} := 0L;\
         ${wanted_offset} := {};\
         ${gen_volume_wait} := ${config.SUBVOLUMES_PER_OFFSET}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_volume, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_volume, pid_trigger_gen_volume));
  mk_edge (net, connection_t (TP, tid_gen_volume, pid_gen_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume, pid_gen_volume_wait));
  mk_edge (net, connection_t (TP, tid_gen_volume, pid_wanted_offset));

  value::structured_t buffer_empty;
  value::structured_t nobunch;
  value::structured_t nopackage;

  nopackage["id"] = -1L;
  nopackage["offset"] = -1L;
  nobunch["id"] = -1L;
  nobunch["package"] = nopackage;
  buffer_empty["assigned"] = false;
  buffer_empty["filled"] = false;
  buffer_empty["processed"] = false;
  buffer_empty["bunch"] = nobunch;
  buffer_empty["store"] = -1L;

  token::put (net, pid_buffer_empty, buffer_empty);

  tid_t tid_gen_volume_step
    ( mk_transition
      ( net
      , "gen_volume_step"
      , "${volume_state.id} := ${gen_volume_state.state};\
         ${volume_state.copy.id} := 0L;\
         ${volume_state.copy.of} := ${config.SUBVOLUME_MULTIPLICITY};\
         ${volume_state.offset} := 0L;\
         ${volume_state.buffer_0} := ${buffer_empty};\
         ${volume_state.buffer_1} := ${buffer_empty};\
         ${volume_count.volume} := ${gen_volume_state.state};\
         ${volume_count.offset} := 0L;\
         ${volume_count.wait_bunch} := ${config.BUNCHES_PER_PACKAGE} * ${config.PACKAGES_PER_OFFSET};\
         ${volume_count.wait_copy} := ${config.SUBVOLUME_MULTIPLICITY};\
         ${gen_volume_state.state} := ${gen_volume_state.state} + 1;\
         ${wanted_offset} := bitset_insert (${wanted_offset}, 0L);"
      , "${gen_volume_state.state} < ${gen_volume_state.num}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_volume_step, pid_config));
  mk_edge (net, connection_t (PT_READ, tid_gen_volume_step, pid_buffer_empty));
  mk_edge (net, connection_t (PT, tid_gen_volume_step, pid_gen_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume_step, pid_gen_volume_state));
  mk_edge (net, connection_t (PT, tid_gen_volume_step, pid_wanted_offset));
  mk_edge (net, connection_t (TP, tid_gen_volume_step, pid_wanted_offset));
  mk_edge (net, connection_t (TP, tid_gen_volume_step, pid_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume_step, pid_volume_count));

  tid_t tid_gen_volume_break
    ( mk_transition
      ( net
      , "gen_volume_break"
      , ""
      , "${gen_volume_state.state} >= ${gen_volume_state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_volume_break, pid_gen_volume_state));

  tid_t tid_gen_volume_copy_step
    ( mk_transition
      ( net
      , "gen_volume_copy_step"
      , "${volume} := ${volume_state};\
         ${volume_state.copy.id} := ${volume_state.copy.id} + 1"
      , "${volume_state.copy.id} < ${volume_state.copy.of}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_volume_copy_step, pid_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume_copy_step, pid_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume_copy_step, pid_volume));

  tid_t tid_gen_volume_copy_break
    ( mk_transition
      ( net
      , "gen_volume_copy_break"
      , "${gen_volume_wait} := ${gen_volume_wait} - 1"
      , "${volume_state.copy.id} >= ${volume_state.copy.of}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_volume_copy_break, pid_volume_state));
  mk_edge (net, connection_t (PT, tid_gen_volume_copy_break, pid_gen_volume_wait));
  mk_edge (net, connection_t (TP, tid_gen_volume_copy_break, pid_gen_volume_wait));

  tid_t tid_trigger_gen_offset
    ( mk_transition
      ( net
      , "trigger_gen_offset"
      , "${trigger_gen_offset} := []"
      , "${gen_volume_wait} == 0L"
      )
    );

  mk_edge (net, connection_t (PT, tid_trigger_gen_offset, pid_gen_volume_wait));
  mk_edge (net, connection_t (TP, tid_trigger_gen_offset, pid_trigger_gen_offset));

  // *********************************************************************** //

  tid_t tid_gen_offset
    ( mk_transition
      ( net
      , "gen_offset"
      , "${gen_offset_state.num} := ${config.OFFSETS};\
         ${gen_offset_state.state} := 0L;"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_offset, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_offset, pid_trigger_gen_offset));
  mk_edge (net, connection_t (TP, tid_gen_offset, pid_gen_offset_state));

  tid_t tid_gen_offset_step
    ( mk_transition
      ( net
      , "gen_offset_step"
      , "${offset} := ${gen_offset_state.state};\
         ${gen_offset_state.state} := ${gen_offset_state.state} + 1;"
      , "${gen_offset_state.state} < ${gen_offset_state.num} &&\
         bitset_is_element (${wanted_offset},${gen_offset_state.state})"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_offset_step, pid_wanted_offset));
  mk_edge (net, connection_t (PT, tid_gen_offset_step, pid_gen_offset_state));
  mk_edge (net, connection_t (TP, tid_gen_offset_step, pid_gen_offset_state));
  mk_edge (net, connection_t (TP, tid_gen_offset_step, pid_offset));

  tid_t tid_gen_offset_break
    ( mk_transition
      ( net
      , "gen_offset_break"
      , ""
      , "${gen_offset_state.state} >= ${gen_offset_state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_offset_break, pid_gen_offset_state));

  // *********************************************************************** //

  tid_t tid_gen_package
    ( mk_transition
      ( net
      , "gen_package"
      , "${gen_package_state.offset} := ${offset};\
         ${gen_package_state.state.num} := ${config.PACKAGES_PER_OFFSET};\
         ${gen_package_state.state.state} := 0L;"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_package, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_package, pid_offset));
  mk_edge (net, connection_t (TP, tid_gen_package, pid_gen_package_state));

  tid_t tid_gen_package_step
    ( mk_transition
      ( net
      , "gen_package_step"
      , "${package.id} := ${gen_package_state.state.state};\
         ${package.offset} := ${gen_package_state.offset};\
         ${gen_package_state.state.state} := ${gen_package_state.state.state} + 1"
      , "${gen_package_state.state.state} < ${gen_package_state.state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_package_step, pid_gen_package_state));
  mk_edge (net, connection_t (TP, tid_gen_package_step, pid_gen_package_state));
  mk_edge (net, connection_t (TP, tid_gen_package_step, pid_package));

  tid_t tid_gen_package_break
    ( mk_transition
      ( net
      , "gen_package_break"
      , ""
      , "${gen_package_state.state.state} >= ${gen_package_state.state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_package_break, pid_gen_package_state));

  // *********************************************************************** //

  tid_t tid_gen_bunch
    ( mk_transition
      ( net
      , "gen_bunch"
      , "${gen_bunch_state.state.num} := ${config.BUNCHES_PER_PACKAGE};\
         ${gen_bunch_state.state.state} := 0L;\
         ${gen_bunch_state.package} := ${package}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_bunch, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_bunch, pid_package));
  mk_edge (net, connection_t (TP, tid_gen_bunch, pid_gen_bunch_state));

  tid_t tid_gen_bunch_step
    ( mk_transition
      ( net
      , "gen_bunch_step"
      , "${bunch.package} := ${gen_bunch_state.package};\
         ${bunch.id} := ${gen_bunch_state.state.state};\
         ${gen_bunch_state.state.state} := ${gen_bunch_state.state.state} + 1"
      , "${gen_bunch_state.state.state} < ${gen_bunch_state.state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_bunch_step, pid_gen_bunch_state));
  mk_edge (net, connection_t (TP, tid_gen_bunch_step, pid_gen_bunch_state));
  mk_edge (net, connection_t (TP, tid_gen_bunch_step, pid_bunch));

  tid_t tid_gen_bunch_break
    ( mk_transition
      ( net
      , "gen_bunch_break"
      , ""
      , "${gen_bunch_state.state.state} >= ${gen_bunch_state.state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_bunch_break, pid_gen_bunch_state));

  // *********************************************************************** //

  tid_t tid_load
    ( mk_transition
      ( net
      , "load"
      , "${loaded_bunch.bunch} := ${bunch};\
         ${loaded_bunch.store} := ${empty_store};\
         ${loaded_bunch.seen} := {};\
         ${loaded_bunch.wait} := ${config.SUBVOLUMES_PER_OFFSET}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_load, pid_config));
  mk_edge (net, connection_t (PT, tid_load, pid_bunch));
  mk_edge (net, connection_t (PT, tid_load, pid_empty_store));
  mk_edge (net, connection_t (TP, tid_load, pid_loaded_bunch));

  tid_t tid_reuse_store
    ( mk_transition
      ( net
      , "reuse store"
      , "${empty_store} := ${loaded_bunch.store}"
      , "${loaded_bunch.wait} == 0L"
      )
    );

  mk_edge (net, connection_t (PT, tid_reuse_store, pid_loaded_bunch));
  mk_edge (net, connection_t (TP, tid_reuse_store, pid_empty_store));

  // *********************************************************************** //

  tid_t tid_assign0
    ( mk_transition
      ( net
      , "assign0"
      , "${volume.buffer_0.assigned} := true;\
         ${volume.buffer_0.bunch} := ${loaded_bunch.bunch} ;\
         ${loaded_bunch.seen} := bitset_insert ( ${loaded_bunch.seen} \
                                               , ${volume.id}         \
                                               );\
         ${volume.buffer_0.store} := ${loaded_bunch.store}\
        "
      , "(!${volume.buffer_0.assigned}) &&\
         (${loaded_bunch.bunch.package.offset} == ${volume.offset}) &&\
         (!bitset_is_element (${loaded_bunch.seen}, ${volume.id})) \
        "
      )
    );

  mk_edge (net, connection_t (PT, tid_assign0, pid_loaded_bunch));
  mk_edge (net, connection_t (TP, tid_assign0, pid_loaded_bunch));
  mk_edge (net, connection_t (PT, tid_assign0, pid_volume));
  mk_edge (net, connection_t (TP, tid_assign0, pid_volume));

  tid_t tid_assign1
    ( mk_transition
      ( net
      , "assign1"
      , "${volume.buffer_1.assigned} := true;\
         ${volume.buffer_1.bunch} := ${loaded_bunch.bunch} ;\
         ${loaded_bunch.seen} := bitset_insert ( ${loaded_bunch.seen} \
                                               , ${volume.id}         \
                                               );\
         ${volume.buffer_1.store} := ${loaded_bunch.store}\
        "
      , "(!${volume.buffer_1.assigned}) &&\
         (${loaded_bunch.bunch.package.offset} == ${volume.offset}) &&\
         (!bitset_is_element (${loaded_bunch.seen}, ${volume.id})) \
        "
      )
    );

  mk_edge (net, connection_t (PT, tid_assign1, pid_loaded_bunch));
  mk_edge (net, connection_t (TP, tid_assign1, pid_loaded_bunch));
  mk_edge (net, connection_t (PT, tid_assign1, pid_volume));
  mk_edge (net, connection_t (TP, tid_assign1, pid_volume));

  net.set_transition_priority (tid_assign0, 1);
  net.set_transition_priority (tid_assign1, 1);

  // *********************************************************************** //
  // token

  token::put (net, pid_config_file, std::string("/scratch/KDM.conf"));

  // *********************************************************************** //

  marking (net);

  {
    boost::mt19937 engine (SEED);

    Timer_t timer ("fire", OFFSETS * SUBVOLUMES_PER_OFFSET * BUNCHES_PER_PACKAGE * PACKAGES_PER_OFFSET);

    while (net.can_fire())
      {
        net.fire_random (engine);

        if (PRINT_MARKING)
          marking (net);
      }
  }

  marking (net);

  stat.out ("Kirchhoff");

  return EXIT_SUCCESS;
}
