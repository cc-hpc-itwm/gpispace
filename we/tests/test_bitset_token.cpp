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

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <boost/serialization/nvp.hpp>

// ************************************************************************* //

typedef unsigned int edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

static edge_cnt_t e (0);

static edge_t mk_edge (const std::string & descr)
{
  return edge_t (e++, descr);
}

// ************************************************************************* //

typedef unsigned int transition_cnt_t;

struct transition_t
{
public:
  transition_cnt_t t;
  std::string name;
  mutable condition::type cond;

  transition_t ( const transition_cnt_t & _t
               , const std::string & _name
               , const condition::type & _cond
               )
    : t(_t), name(_name), cond(_cond)
  {}

  bool condition (Function::Condition::Traits<token::type>::choices_t & choices)
    const
  {
    return cond (choices);
  }

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(t);
    ar & BOOST_SERIALIZATION_NVP(name);
    ar & BOOST_SERIALIZATION_NVP(cond);
  }
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<transition_cnt_t> h;

  return h (t.t);
}

inline bool operator == (const transition_t & x, const transition_t & y)
{
  return x.t == y.t;
}

static transition_t mk_trans ( const std::string & name
                             , const condition::type & cond
                             )
{
  static transition_cnt_t t (0);

  return transition_t (t++, name, cond);
}

typedef petri_net::net<place::type, transition_t, edge_t, token::type> pnet_t;

// ************************************************************************* //

using std::cout;
using std::endl;

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "{" << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "}";
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

    parser.eval_all (context);

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

    return output;
  }
};

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      , const std::string & condition
                                      )
{
  const petri_net::tid_t tid
    ( net.add_transition
      ( mk_trans ( name
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
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  long NUM_VID (4);
  long NUM_BID (2);
  bool PRINT_MARKING (true);

  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ("num_vid", po::value<long>(&NUM_VID)->default_value(NUM_VID), "number of vid")
    ("num_bid", po::value<long>(&NUM_BID)->default_value(NUM_BID), "number of bid")
    ("print", po::value<bool>(&PRINT_MARKING)->default_value(PRINT_MARKING), "print after each fire")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      cout << desc << "\n";
      return EXIT_SUCCESS;
    }

  pnet_t net;

  // simple
  petri_net::pid_t pid_vid (net.add_place (place::type("vid","long")));

  // structured
  signature::structured_t sig;

  sig["bid"] = "long";
  sig["seen"] = "bitset";

  petri_net::pid_t pid_store (net.add_place (place::type("store", sig)));

  petri_net::tid_t tid
    ( mk_transition
      ( net
      , "trans"
      , "${store.seen} := bitset_insert (${store.seen}, ${vid}); \
         ${store.bid}  := ${store.bid}                         ; "
      , "!bitset_is_element (${store.seen}, ${vid})"
      )
    );

  net.add_edge (mk_edge ("get pair"), connection_t (PT, tid, pid_store));
  net.add_edge (mk_edge ("set pair"), connection_t (TP, tid, pid_store));
  net.add_edge (mk_edge ("read vid"), connection_t (PT_READ, tid, pid_vid));


  for (long i (0); i < NUM_VID; ++i)
    token::put (net, pid_vid, literal::type(i));

  for (long i (0); i < NUM_BID; ++i)
    {
      value::structured_t m;

      m["bid"] = i;
      m["seen"] = bitsetofint::type(0);

      token::put (net, pid_store, m);
    }

  marking (net);

  {
    boost::mt19937 engine;

    Timer_t timer ("fire", NUM_VID * NUM_BID);

    while (!net.enabled_transitions().empty())
      {
        net.fire_random(engine);

        if (PRINT_MARKING)
          marking (net);
      }
  }

  marking (net);

  return EXIT_SUCCESS;
}
