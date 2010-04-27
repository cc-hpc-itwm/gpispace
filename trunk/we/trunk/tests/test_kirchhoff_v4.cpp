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

static long NUMBER_OF_NODES (8);
static long OFFSETS (2);
static long BUNCHES_PER_OFFSET (3);
static long STORES (8);
static long SUBVOLUMES_PER_OFFSET (3);

static bool PRINT_MARKING (true);
static bool PRINT_FIRE (true);

static unsigned int SEED (3141);

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

// ************************************************************************* //

typedef unsigned int edge_t;

// ************************************************************************* //

typedef petri_net::net<place::type, transition_t, edge_t, token::type> pnet_t;

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

static std::vector<std::string>
mk_vec (const std::string & s)
{
  std::vector<std::string> v;
  std::string e;

  std::string::const_iterator pos (s.begin());
  const std::string::const_iterator end (s.end());

  while (pos != end)
    {
      switch (*pos)
        {
        case '.':
          v.push_back (e);
          e.clear();
          break;
        default:
          e.push_back (*pos);
          break;
        }
      ++pos;
    }

  v.push_back (e);

  return v;
}

typedef expr::eval::context<signature::field_name_t> context_t;

class TransitionFunction
{
private:
  const std::string name;
  const std::string expression;
  const expr::parse::parser<signature::field_name_t> parser;
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
        if (name == "process")
          {
            if (value::function::is_true (context.value (mk_vec ("volume.buffer0.filled"))))
              {
                cout << "*** " << name;
                cout << " vol " << context.value (mk_vec ("volume.offset"))
                     << "." << context.value (mk_vec ("volume.id"));
                cout << " " << context.value (mk_vec ("volume.buffer0.bunch"));
                cout << endl;
              }
            if (value::function::is_true (context.value (mk_vec ("volume.buffer1.filled"))))
              {
                cout << "*** " << name;
                cout << " vol " << context.value (mk_vec ("volume.offset"))
                     << "." << context.value (mk_vec ("volume.id"));
                cout << " " << context.value (mk_vec ("volume.buffer1.bunch"));
                cout << endl;
              }
          }
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
  return net.add_transition 
    ( transition_t ( name
                   , condition::type 
                     ( strip (condition)
                     , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
                     )
               )
    , TransitionFunction
      ( name
      , strip (expression)
      , boost::bind(&place::name<pnet_t>, boost::ref(net), _1)
      , boost::bind(&place::signature<pnet_t>, boost::ref(net), _1)
      , context
      )
    );
}

// ************************************************************************* //

static void
mk_edge (pnet_t & net, const petri_net::connection_t & c)
{
  static edge_t e (0); net.add_edge (e++, c);
}

// ************************************************************************* //

static petri_net::pid_t
mk_place ( pnet_t & net
         , const std::string & name
         , const literal::type_name_t & sig = literal::CONTROL
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

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

namespace po = boost::program_options;

// *********************************************************************** //

namespace signature
{
  static structured_t config;
  static structured_t state;
  static structured_t offset_with_state;
  static structured_t bunch;
  static structured_t loaded_bunch;
  static structured_t buffer;
  static structured_t volume;

  static void init (void)
  {
    config["NUMBER_OF_NODES"] = literal::LONG;
    config["OFFSETS"] = literal::LONG;
    config["BUNCHES_PER_OFFSET"] = literal::LONG;
    config["STORES"] = literal::LONG;
    config["SUBVOLUMES_PER_OFFSET"] = literal::LONG;

    state["num"] = literal::LONG;
    state["state"] = literal::LONG;

    offset_with_state["offset"] = literal::LONG;
    offset_with_state["state"] = state;

    bunch["id"] = literal::LONG;
    bunch["offset"] = literal::LONG;

    loaded_bunch["bunch"] = bunch;
    loaded_bunch["store"] = literal::LONG;
    loaded_bunch["seen"] = literal::BITSET;
    loaded_bunch["wait"] = literal::LONG;

    buffer["filled"] = literal::BOOL;
    buffer["assigned"] = literal::BOOL;
    buffer["prefetched"] = literal::BOOL;
    buffer["bunch"] = bunch;
    buffer["store"] = literal::LONG;

    volume["id"] = literal::LONG;
    volume["offset"] = literal::LONG;
    volume["wait"] = literal::LONG;
    volume["buffer0"] = buffer;
    volume["buffer1"] = buffer;
  }
}

namespace value
{
  namespace bunch
  {
    static structured_t invalid;

    static void init (void)
    {
      invalid["id"] = -1L;
      invalid["offset"] = -1L;
    }
  }

  namespace buffer
  {
    static structured_t empty;

    static void init (void)
    {
      empty["assigned"] = false;
      empty["filled"] = false;
      empty["prefetched"] = false;
      empty["store"] = -1L;
      empty["bunch"] = bunch::invalid;
    }
  }

  static void init (void)
  {
    bunch::init();
    buffer::init();
  }
}

// *********************************************************************** //

int
main (int argc, char ** argv)
{
  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ("nodes", po::value<long>(&NUMBER_OF_NODES)->default_value(NUMBER_OF_NODES), "number of nodes")
    ("offsets", po::value<long>(&OFFSETS)->default_value(OFFSETS), "number of offsets")
    ("bunches", po::value<long>(&BUNCHES_PER_OFFSET)->default_value(BUNCHES_PER_OFFSET), "number of bunches per offset")
    ("stores", po::value<long>(&STORES)->default_value(STORES), "number of bunch stores")
    ("subvolumes", po::value<long>(&SUBVOLUMES_PER_OFFSET)->default_value(SUBVOLUMES_PER_OFFSET), "number of subvolumes per offset")

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
    << "NUMBER_OF_NODES        = " << NUMBER_OF_NODES << endl
    << "OFFSETS                = " << OFFSETS << endl
    << "BUNCHES_PER_OFFSET     = " << BUNCHES_PER_OFFSET << endl
    << "STORES                 = " << STORES << endl
    << "SUBVOLUMES_PER_OFFSET  = " << SUBVOLUMES_PER_OFFSET << endl

    << "SEED                   = " << SEED << endl

    << "PRINT_MARKING          = " << PRINT_MARKING << endl
    << "PRINT_FIRE             = " << PRINT_FIRE << endl
    ;

  pnet_t net;

  signature::init();
  value::init();

  using petri_net::pid_t;
  using petri_net::tid_t;

  // *********************************************************************** //
  // place

  pid_t pid_config_file (mk_place (net, "config_file", literal::STRING));
  pid_t pid_config (mk_place (net, "config", signature::config));
  pid_t pid_trigger_gen_store (mk_place (net, "trigger_gen_store"));
  pid_t pid_gen_store_state (mk_place (net, "gen_store_state", signature::state));
  pid_t pid_empty_store (mk_place (net, "empty_store", literal::LONG));
  pid_t pid_trigger_gen_offset (mk_place (net, "trigger_gen_offset"));
  pid_t pid_wanted_offset (mk_place (net, "wanted_offset", literal::BITSET));
  pid_t pid_gen_offset_state (mk_place (net, "gen_offset_state", signature::state));
  pid_t pid_offset_bunch (mk_place (net, "offset_bunch", literal::LONG));
  pid_t pid_offset_volume (mk_place (net, "offset_volume", literal::LONG));
  pid_t pid_gen_bunch_state (mk_place (net, "gen_bunch_state", signature::offset_with_state));
  pid_t pid_bunch (mk_place (net, "bunch", signature::bunch));
  pid_t pid_loaded_bunch (mk_place (net, "loaded_bunch", signature::loaded_bunch));
  pid_t pid_gen_volume_state (mk_place (net, "gen_volume_state", signature::offset_with_state));
  pid_t pid_volume (mk_place (net, "volume", signature::volume));
  pid_t pid_volume_processed (mk_place (net, "volume_processed", signature::volume));
  net.set_capacity (pid_volume_processed, 1);
  pid_t pid_buffer_empty (mk_place (net, "buffer_empty", signature::buffer));
  token::put (net, pid_buffer_empty, value::buffer::empty);

  // *********************************************************************** //
  // transition

  tid_t tid_gen_config
    ( mk_transition
      ( net
      , "gen_config"
      , "${config.NUMBER_OF_NODES} := " + util::show(NUMBER_OF_NODES) + ";"
      + "${config.OFFSETS} := " + util::show (OFFSETS) + ";"
      + "${config.BUNCHES_PER_OFFSET} := " + util::show(BUNCHES_PER_OFFSET) + ";"
      + "${config.STORES} := " + util::show (STORES) + ";"
      + "${config.SUBVOLUMES_PER_OFFSET} := " + util::show(SUBVOLUMES_PER_OFFSET) + ";"
      + "${trigger_gen_store} := [];"
      + "${wanted_offset} := bitset_insert ({}, 0L);"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_config, pid_config_file));
  mk_edge (net, connection_t (TP, tid_gen_config, pid_config));
  mk_edge (net, connection_t (TP, tid_gen_config, pid_trigger_gen_store));
  mk_edge (net, connection_t (TP, tid_gen_config, pid_wanted_offset));

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
         ${gen_store_state.state} := ${gen_store_state.state} + 1"
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
      , "${trigger_gen_offset} := []"
      , "${gen_store_state.state} >= ${gen_store_state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_store_break, pid_gen_store_state));
  mk_edge (net, connection_t (TP, tid_gen_store_break, pid_trigger_gen_offset));

  // *********************************************************************** //

  tid_t tid_gen_offset
    ( mk_transition
      ( net
      , "gen_offset"
      , "${gen_offset_state.num} := ${config.OFFSETS};\
         ${gen_offset_state.state} := 0L"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_offset, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_offset, pid_trigger_gen_offset));
  mk_edge (net, connection_t (TP, tid_gen_offset, pid_gen_offset_state));

  tid_t tid_gen_offset_step
    ( mk_transition
      ( net
      , "gen_offset_step"
      , "${offset_bunch} := ${gen_offset_state.state};\
         ${offset_volume} := ${gen_offset_state.state};\
         ${gen_offset_state.state} := ${gen_offset_state.state} + 1"
      , "${gen_offset_state.state} < ${gen_offset_state.num} &\
         bitset_is_element (${wanted_offset}, ${gen_offset_state.state})"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_offset_step, pid_wanted_offset));
  mk_edge (net, connection_t (PT, tid_gen_offset_step, pid_gen_offset_state));
  mk_edge (net, connection_t (TP, tid_gen_offset_step, pid_gen_offset_state));
  mk_edge (net, connection_t (TP, tid_gen_offset_step, pid_offset_bunch));
  mk_edge (net, connection_t (TP, tid_gen_offset_step, pid_offset_volume));

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

  tid_t tid_gen_bunch
    ( mk_transition
      ( net
      , "gen_bunch"
      , "${gen_bunch_state.state.num} := ${config.BUNCHES_PER_OFFSET} ;\
         ${gen_bunch_state.state.state} := 0L ;\
         ${gen_bunch_state.offset} := ${offset_bunch}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_bunch, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_bunch, pid_offset_bunch));
  mk_edge (net, connection_t (TP, tid_gen_bunch, pid_gen_bunch_state));
  
  tid_t tid_gen_bunch_step
    ( mk_transition
      ( net
      , "gen_bunch_step"
      , "${bunch.id} := ${gen_bunch_state.state.state} ;\
         ${bunch.offset} := ${gen_bunch_state.offset};\
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

  tid_t tid_gen_volume
    ( mk_transition
      ( net
      , "gen_volume"
      , "${gen_volume_state.state.state} := 0L;\
         ${gen_volume_state.state.num} := ${config.SUBVOLUMES_PER_OFFSET};\
         ${gen_volume_state.offset} := ${offset_volume}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_volume, pid_config));
  mk_edge (net, connection_t (PT, tid_gen_volume, pid_offset_volume));
  mk_edge (net, connection_t (TP, tid_gen_volume, pid_gen_volume_state));

  tid_t tid_gen_volume_step
    ( mk_transition
      ( net
      , "gen_volume_step"
      , "${volume.id} := ${gen_volume_state.state.state};\
         ${volume.offset} := ${gen_volume_state.offset};\
         ${volume.wait} := ${config.BUNCHES_PER_OFFSET};\
         ${volume.buffer0} := ${buffer_empty};\
         ${volume.buffer1} := ${buffer_empty};\
         ${gen_volume_state.state.state} := ${gen_volume_state.state.state} + 1"
      , "${gen_volume_state.state.state} < ${gen_volume_state.state.num}"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_gen_volume_step, pid_config));
  mk_edge (net, connection_t (PT_READ, tid_gen_volume_step, pid_buffer_empty));
  mk_edge (net, connection_t (PT, tid_gen_volume_step, pid_gen_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume_step, pid_gen_volume_state));
  mk_edge (net, connection_t (TP, tid_gen_volume_step, pid_volume));

  tid_t tid_gen_volume_break
    ( mk_transition
      ( net
      , "gen_volume_break"
      , ""
      , "${gen_volume_state.state.state} >= ${gen_volume_state.state.num}"
      )
    );

  mk_edge (net, connection_t (PT, tid_gen_volume_break, pid_gen_volume_state));

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
  mk_edge (net, connection_t (PT, tid_load, pid_empty_store));
  mk_edge (net, connection_t (PT, tid_load, pid_bunch));
  mk_edge (net, connection_t (TP, tid_load, pid_loaded_bunch));

  tid_t tid_reuse_store
    ( mk_transition
      ( net
      , "reuse_store"
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
      , "${loaded_bunch.seen} := bitset_insert (${loaded_bunch.seen}, ${volume.id});\
         ${volume.buffer0.assigned} := true;\
         ${volume.buffer0.bunch} := ${loaded_bunch.bunch};\
         ${volume.buffer0.store} := ${loaded_bunch.store}"
      , "(!${volume.buffer0.assigned}) &\
         (${volume.offset} == ${loaded_bunch.bunch.offset}) &\
         (!bitset_is_element (${loaded_bunch.seen}, ${volume.id}))"
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
      , "${loaded_bunch.seen} := bitset_insert (${loaded_bunch.seen}, ${volume.id});\
         ${volume.buffer1.assigned} := true;\
         ${volume.buffer1.bunch} := ${loaded_bunch.bunch};\
         ${volume.buffer1.store} := ${loaded_bunch.store}"
      , "(!${volume.buffer1.assigned}) &\
         (${volume.offset} == ${loaded_bunch.bunch.offset}) &\
         (!bitset_is_element (${loaded_bunch.seen}, ${volume.id}))"
      )
    );

  mk_edge (net, connection_t (PT, tid_assign1, pid_loaded_bunch));
  mk_edge (net, connection_t (TP, tid_assign1, pid_loaded_bunch));
  mk_edge (net, connection_t (PT, tid_assign1, pid_volume));
  mk_edge (net, connection_t (TP, tid_assign1, pid_volume));

  net.set_transition_priority (tid_assign0, 1);
  net.set_transition_priority (tid_assign1, 1);

  // *********************************************************************** //

  tid_t tid_process
    ( mk_transition
      ( net
      , "process"
      , "${volume_processed} := ${volume};\
         ${volume_processed.buffer0.assigned} := (${volume.buffer0.assigned} & !${volume.buffer0.filled});\
         ${volume_processed.buffer0.filled} := (${volume.buffer0.assigned} & !${volume.buffer0.filled});\
         ${volume_processed.buffer0.prefetched} := (${volume.buffer0.assigned} & !${volume.buffer0.filled});\
         ${volume_processed.buffer1.assigned} := (${volume.buffer1.assigned} & !${volume.buffer1.filled});\
         ${volume_processed.buffer1.filled} := (${volume.buffer1.assigned} & !${volume.buffer1.filled});\
         ${volume_processed.buffer1.prefetched} := (${volume.buffer1.assigned} & !${volume.buffer1.filled});\
         ${volume_processed.wait} := ${volume.wait}\
                                  - (if ${volume.buffer0.filled} then 1L else 0L endif)\
                                  - (if ${volume.buffer1.filled} then 1L else 0L endif)"
      , "${volume.buffer0.assigned} | ${volume.buffer1.assigned}"
      )
    );

  mk_edge (net, connection_t (PT, tid_process, pid_volume));
  mk_edge (net, connection_t (TP, tid_process, pid_volume_processed));

  // *********************************************************************** //

  tid_t tid_reuse_store0
    ( mk_transition
      ( net
      , "reuse_store0"
      , "${loaded_bunch.wait} := ${loaded_bunch.wait} - 1;\
         ${volume_processed.buffer0.prefetched} := false;"
      , "(${volume_processed.buffer0.bunch} == ${loaded_bunch.bunch}) &\
         (${volume_processed.buffer0.prefetched})"
      )
    );

  mk_edge (net, connection_t (PT, tid_reuse_store0, pid_loaded_bunch));
  mk_edge (net, connection_t (TP, tid_reuse_store0, pid_loaded_bunch));
  mk_edge (net, connection_t (PT, tid_reuse_store0, pid_volume_processed));
  mk_edge (net, connection_t (TP, tid_reuse_store0, pid_volume_processed));

  tid_t tid_reuse_store1
    ( mk_transition
      ( net
      , "reuse_store1"
      , "${loaded_bunch.wait} := ${loaded_bunch.wait} - 1;\
         ${volume_processed.buffer1.prefetched} := false;"
      , "(${volume_processed.buffer1.bunch} == ${loaded_bunch.bunch}) &\
         (${volume_processed.buffer1.prefetched})"
      )
    );

  mk_edge (net, connection_t (PT, tid_reuse_store1, pid_loaded_bunch));
  mk_edge (net, connection_t (TP, tid_reuse_store1, pid_loaded_bunch));
  mk_edge (net, connection_t (PT, tid_reuse_store1, pid_volume_processed));
  mk_edge (net, connection_t (TP, tid_reuse_store1, pid_volume_processed));

  net.set_transition_priority (tid_reuse_store0, 1);
  net.set_transition_priority (tid_reuse_store1, 1);

  // *********************************************************************** //

  tid_t tid_volume_step
    ( mk_transition
      ( net
      , "volume_step"
      , "${volume} := ${volume_processed}"
      , "${volume_processed.wait} > 0L"
      )
    );

  mk_edge (net, connection_t (PT, tid_volume_step, pid_volume_processed));
  mk_edge (net, connection_t (TP, tid_volume_step, pid_volume));

  tid_t tid_volume_next_offset
    ( mk_transition
      ( net
      , "volume_next_offset"
      , "${wanted_offset} := bitset_insert ( ${wanted_offset} \
                                           , ${volume_processed.offset} + 1 \
                                           );"
      , "(${volume_processed.wait} == 0L) &\
         (${volume_processed.offset} + 1 < ${config.OFFSETS})"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_volume_next_offset, pid_config));
  mk_edge (net, connection_t (PT, tid_volume_next_offset, pid_wanted_offset));
  mk_edge (net, connection_t (TP, tid_volume_next_offset, pid_wanted_offset));
  mk_edge (net, connection_t (PT, tid_volume_next_offset, pid_volume_processed));

  tid_t tid_volume_done
    ( mk_transition
      ( net
      , "volume_done"
      , ""
      , "(${volume_processed.wait} == 0L) &\
         (${volume_processed.offset} + 1 >= ${config.OFFSETS})"
      )
    );

  mk_edge (net, connection_t (PT_READ, tid_volume_done, pid_config));
  mk_edge (net, connection_t (PT, tid_volume_done, pid_volume_processed));

  // *********************************************************************** //
  // token

  token::put (net, pid_config_file, std::string("/scratch/KDM.conf"));
  
  // *********************************************************************** //

  marking (net);

  {
    boost::mt19937 engine (SEED);

    Timer_t timer ("fire");

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
