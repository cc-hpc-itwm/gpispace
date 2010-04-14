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

#include <we/util/show.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <boost/serialization/nvp.hpp>

// ************************************************************************* //

static long NUM_VID (4);
static long NUM_BID (4);
static long NUM_STORE (3);
static long NUM_BUF (2);
static unsigned int SEED (3141);
static bool PRINT_MARKING (true);
static bool PRINT_MODCALL (true);

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

    if (PRINT_MODCALL)
      {
        if (name == "load")
          {
            std::cout << "LOAD"
                      << " bid " << context.value ("bid_in")
                      << " sid " << context.value ("sid")
                      << std::endl;
          }
        else if (name == "prefetch")
          {
            std::cout << "PREFETCH"
                      << " bid " << context.value ("store.bid")
                      << " from sid " << context.value ("store.sid")
                      << " to vid " << context.value ("vid_buf_empty.vid")
                      << " into buf " << context.value ("vid_buf_empty.bufid")
                      << std::endl;
          }
        else if (name == "process")
          {
            std::cout << "PROCESS"
                      << " vid " << context.value ("vid_buf_filled.vid")
                      << " bid " << context.value ("vid_buf_filled.bid")
                      << " buf " << context.value ("vid_buf_filled.bufid")
                      << std::endl;
          }
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
  return net.add_transition 
    ( mk_trans ( name
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
    ("subvolumes", po::value<long>(&NUM_VID)->default_value(NUM_VID), "number of subvolumes")
    ("bunches", po::value<long>(&NUM_BID)->default_value(NUM_BID), "number of bunches")
    ("stores", po::value<long>(&NUM_STORE)->default_value(NUM_STORE), "number of bunch stores")
    ("buffers", po::value<long>(&NUM_BUF)->default_value(NUM_BUF), "number of bunch buffers per subvolume")
    ("seed", po::value<unsigned int>(&SEED)->default_value(SEED), "seed for random number generator")
    ("print_marking", po::value<bool>(&PRINT_MARKING)->default_value(PRINT_MARKING), "print marking after each fire")
    ("print_modcall", po::value<bool>(&PRINT_MODCALL)->default_value(PRINT_MODCALL), "print modcalls")
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

  petri_net::pid_t pid_NUM_VID (net.add_place (place::type("NUM_VID","long")));
  petri_net::pid_t pid_NUM_BID (net.add_place (place::type("NUM_BID","long")));
  petri_net::pid_t pid_NUM_STORE (net.add_place (place::type("NUM_STORE","long")));
  petri_net::pid_t pid_NUM_BUF (net.add_place (place::type("NUM_BUF","long")));

  petri_net::pid_t pid_vid_in (net.add_place (place::type("vid_in","long")));
  petri_net::pid_t pid_bid_in (net.add_place (place::type("bid_in","long")));
  petri_net::pid_t pid_vid_out (net.add_place (place::type("vid_out","long")));
  petri_net::pid_t pid_bid_out (net.add_place (place::type("bid_out","long")));
  petri_net::pid_t pid_sid (net.add_place (place::type("sid","long")));
  petri_net::pid_t pid_bufid (net.add_place (place::type("bufid","long")));

  signature::structured_t sig_store;

  sig_store["bid"] = "long";
  sig_store["seen"] = "bitset";
  sig_store["sid"] = "long";
  sig_store["numvid"] = "long";

  petri_net::pid_t pid_store (net.add_place (place::type("store", sig_store)));

  signature::structured_t sig_vid_buffer_empty;

  sig_vid_buffer_empty["vid"] = "long";
  sig_vid_buffer_empty["bufid"] = "long";
  sig_vid_buffer_empty["numbid"] = "long";

  petri_net::pid_t pid_vid_buf_empty
    (net.add_place (place::type("vid_buf_empty", sig_vid_buffer_empty)));
  petri_net::pid_t pid_vid_buf_processed
    (net.add_place (place::type("vid_buf_processed", sig_vid_buffer_empty)));

  signature::structured_t sig_vid_buffer_filled;

  sig_vid_buffer_filled["vid"] = "long";
  sig_vid_buffer_filled["bufid"] = "long";
  sig_vid_buffer_filled["numbid"] = "long";
  sig_vid_buffer_filled["bid"] = "long";

  petri_net::pid_t pid_vid_buf_filled
    (net.add_place (place::type("vid_buf_filled", sig_vid_buffer_filled)));

  petri_net::tid_t tid_load
    ( mk_transition 
      ( net
      , "load"
      , "${store.bid}    := ${bid_in} ;\
         ${store.seen}   := {}        ;\
         ${store.sid}    := ${sid}    ;\
         ${store.numvid} := 0;         "
      , "true"
      )
    );

  net.add_edge (mk_edge ("get bid_in"), connection_t (PT, tid_load, pid_bid_in));
  net.add_edge (mk_edge ("get sid"), connection_t (PT, tid_load, pid_sid));
  net.add_edge (mk_edge ("set store"), connection_t (TP, tid_load, pid_store));

  petri_net::tid_t tid_done_bid
    ( mk_transition 
      ( net
      , "done_bid"
      , "${bid_out} := ${store.bid} ;\
         ${sid}     := ${store.sid}  "
      , "${store.numvid} == ${NUM_VID}"
      )
    );

  net.add_edge (mk_edge ("read NUM_VID"), connection_t (PT_READ, tid_done_bid, pid_NUM_VID));
  net.add_edge (mk_edge ("get store"), connection_t (PT, tid_done_bid, pid_store));
  net.add_edge (mk_edge ("set sid"), connection_t (TP, tid_done_bid, pid_sid));
  net.add_edge (mk_edge ("set bid_out"), connection_t (TP, tid_done_bid, pid_bid_out));

  petri_net::tid_t tid_tag
    ( mk_transition 
      ( net
      , "tag"
      , "${vid_buf_empty.vid}    := ${vid_in} ;\
         ${vid_buf_empty.bufid}  := ${bufid}  ;\
         ${vid_buf_empty.numbid} := 0          "
      , "true"
      )
    );

  net.add_edge (mk_edge ("get vid"), connection_t (PT, tid_tag, pid_vid_in));
  net.add_edge (mk_edge ("read bufid"), connection_t (PT_READ, tid_tag, pid_bufid));
  net.add_edge (mk_edge ("set vid_buf_empty"), connection_t (TP, tid_tag, pid_vid_buf_empty));

  petri_net::tid_t tid_prefetch
    ( mk_transition 
      ( net
      , "prefetch"
      , "${store.bid}    := ${store.bid} ;                        ;\
         ${store.seen}   := bitset_insert ( ${store.seen}          \
                                          , ${vid_buf_empty.vid}   \
                                          )                       ;\
         ${store.sid}    := ${store.sid};                         ;\
         ${store.numvid} := ${store.numvid} + 1                   ;\
                                                                   \
         ${vid_buf_filled.vid}    := ${vid_buf_empty.vid}         ;\
         ${vid_buf_filled.bufid}  := ${vid_buf_empty.bufid}       ;\
         ${vid_buf_filled.bid}    := ${store.bid}                 ;\
         ${vid_buf_filled.numbid} := ${vid_buf_empty.numbid}       "
      , "!bitset_is_element (${store.seen}, ${vid_buf_empty.vid})"
      )
    );

  net.add_edge (mk_edge ("get vid_buf_empty"), connection_t (PT, tid_prefetch, pid_vid_buf_empty));
  net.add_edge (mk_edge ("get store"), connection_t (PT, tid_prefetch, pid_store));
  net.add_edge (mk_edge ("set store"), connection_t (TP, tid_prefetch, pid_store));
  net.add_edge (mk_edge ("set vid_buf_filled"), connection_t (TP, tid_prefetch, pid_vid_buf_filled));

  petri_net::tid_t tid_process
    ( mk_transition 
      ( net
      , "process"
      , "${vid_buf_processed.vid}    := ${vid_buf_filled.vid}        ;\
         ${vid_buf_processed.bufid}  := ${vid_buf_filled.bufid}      ;\
         ${vid_buf_processed.numbid} := ${vid_buf_filled.numbid} + 1  "
      , "true"
      )
    );

  net.add_edge (mk_edge ("get vid_buf_filled"), connection_t (PT, tid_process, pid_vid_buf_filled));
  net.add_edge (mk_edge ("set vid_buf_processed"), connection_t (TP, tid_process, pid_vid_buf_processed));

  petri_net::tid_t tid_done_vid
    ( mk_transition 
      ( net
      , "done_vid"
      , "${vid_out} := ${vid_buf_processed.vid}"
      , "${vid_buf_processed.numbid} == ${NUM_BID}"
      )
    );

  net.add_edge (mk_edge ("get vid_buf_processed"), connection_t (PT, tid_done_vid, pid_vid_buf_processed));
  net.add_edge (mk_edge ("read NUM_BID"), connection_t (PT_READ, tid_done_vid, pid_NUM_BID));
  net.add_edge (mk_edge ("set vid_out"), connection_t (TP, tid_done_vid, pid_vid_out));

  petri_net::tid_t tid_next_vid
    ( mk_transition 
      ( net
      , "next_vid"
      , "${vid_buf_empty.vid}    := ${vid_buf_processed.vid}    ;\
         ${vid_buf_empty.bufid}  := ${vid_buf_processed.bufid}  ;\
         ${vid_buf_empty.numbid} := ${vid_buf_processed.numbid}  "
      , "${vid_buf_processed.numbid} != ${NUM_BID}"
      )
    );

  net.add_edge (mk_edge ("get vid_buf_processed"), connection_t (PT, tid_next_vid, pid_vid_buf_processed));
  net.add_edge (mk_edge ("read NUM_BID"), connection_t (PT_READ, tid_next_vid, pid_NUM_BID));
  net.add_edge (mk_edge ("set vid_buf_empty"), connection_t (TP, tid_next_vid, pid_vid_buf_empty));

  token::put (net, pid_NUM_VID, NUM_VID);
  token::put (net, pid_NUM_BID, NUM_BID);
  token::put (net, pid_NUM_BUF, NUM_BUF);
  token::put (net, pid_NUM_STORE, NUM_STORE);

  for (long i (0); i < NUM_VID; ++i)
    token::put (net, pid_vid_in, literal::type(i));

  for (long i (0); i < NUM_BID; ++i)
    token::put (net, pid_bid_in, literal::type(i));

  for (long i (0); i < NUM_STORE; ++i)
    token::put (net, pid_sid, literal::type(i));

  for (long i (0); i < NUM_BUF; ++i)
    token::put (net, pid_bufid, literal::type(i));

  marking (net);

  {
    boost::mt19937 engine (SEED);

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
