// structured token, expression transition, mirko.rahn@itwm.fraunhofer.de

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

// ************************************************************************* //

typedef unsigned int transition_cnt_t;
typedef std::pair<transition_cnt_t,std::string> transition_t;
typedef unsigned int edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

static edge_cnt_t e (0);
static transition_cnt_t t (0);

static edge_t mk_edge (const std::string & descr)
{
  return edge_t (e++, descr);
}

static transition_t mk_trans (const std::string & descr)
{
  return transition_t (t++, descr);
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

template<typename NET>
class Transition
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
  explicit Transition ( const std::string & _name
                      , const std::string & _expression
                      , const NET & net
                      )
    : name (_name)
    , expression (_expression)
    , parser (expression)
    , context ()
    , translate (boost::bind(&place::name<NET>, boost::ref(net), _1))
    , signature (boost::bind(&place::signature<NET>, boost::ref(net), _1))
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
    ( mk_trans (name)
    , Transition<pnet_t> (name, expression, net)
    , condition::type<pnet_t> (condition, net)
    );
}

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      )
{
  return net.add_transition 
    ( mk_trans (name)
    , Transition<pnet_t> (name, expression, net)
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
  long NUM_SLICES (3);
  long MAX_DEPTH (4);
  long CAP_IN_PROGRESS (0);
  bool PRINT_MARKING (true);

  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ("slices", po::value<long>(&NUM_SLICES)->default_value(3), "num slices")
    ("depth", po::value<long>(&MAX_DEPTH)->default_value(4), "max depth")
    ("cap", po::value<long>(&CAP_IN_PROGRESS)->default_value(0), "capacity in place 'in_progress'")
    ("print", po::value<bool>(&PRINT_MARKING)->default_value(true), "print after each fire")
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

  petri_net::pid_t pid_NUM_SLICES (net.add_place (place::type("NUM_SLICES","long")));
  petri_net::pid_t pid_MAX_DEPTH (net.add_place (place::type("MAX_DEPTH","long")));
  petri_net::pid_t pid_splitted (net.add_place (place::type("splitted","long")));
  petri_net::pid_t pid_slice_in (net.add_place (place::type("slice_in","long")));
  petri_net::pid_t pid_slice_out (net.add_place (place::type("slice_out","long")));
  petri_net::pid_t pid_joined (net.add_place (place::type("joined","long")));

  signature::structured_t sig;

  sig["slice"] = "long";
  sig["depth"] = "long";

  petri_net::pid_t pid_slice_depth (net.add_place (place::type("slice_depth", sig)));

  petri_net::pid_t pid_done (net.add_place (place::type("done"))); // control
  petri_net::pid_t pid_in_progress (net.add_place (place::type("in_progress"))); // control

  petri_net::tid_t tid_split
    ( mk_transition 
      ( net
      , "split"
      , "${slice_in}    := ${splitted};     \
         ${splitted}    := ${splitted} + 1; \
         ${in_progress} := [];              "
      , "${splitted} < ${NUM_SLICES}"
      )
    );
  petri_net::tid_t tid_tag
    ( mk_transition
      ( net
      , "tag"
      , "${slice_depth.slice} := ${slice_in}; \
         ${slice_depth.depth} := 0            "
      )
    );
  petri_net::tid_t tid_work
    ( mk_transition
      ( net
      , "work"
      , "${slice_depth.slice} := ${slice_depth.slice};   \
         ${slice_depth.depth} := ${slice_depth.depth} + 1"
      , "${slice_depth.depth} < ${MAX_DEPTH}"
      )
    );

  petri_net::tid_t tid_untag
    ( mk_transition
      ( net
      , "untag"
      , "${slice_out} := ${slice_depth.slice}"
      , "${slice_depth.depth} >= ${MAX_DEPTH}"
      )
    );

  petri_net::pid_t tid_join
    ( mk_transition
      ( net
      , "join"
      , "${joined} := ${joined} + 1"
      )
    );

  petri_net::tid_t tid_finalize 
    ( mk_transition
      ( net
      , "finalize"
      , "${done} := []"
      , "${joined} == ${NUM_SLICES} & ${splitted} == ${NUM_SLICES}"
      )
    );

  net.add_edge ( mk_edge ("get splitted")
               , connection_t (PT, tid_split, pid_splitted)
               );
  net.add_edge ( mk_edge ("read NUM_SLICES")
               , connection_t (PT_READ, tid_split, pid_NUM_SLICES)
               );
  net.add_edge ( mk_edge ("put slice_in")
               , connection_t (TP, tid_split, pid_slice_in)
               );
  net.add_edge ( mk_edge ("put splitted")
               , connection_t (TP, tid_split, pid_splitted)
               );
  net.add_edge ( mk_edge ("put in_progress")
               , connection_t (TP, tid_split, pid_in_progress)
               );

  net.add_edge ( mk_edge ("get slice_in")
               , connection_t (PT, tid_tag, pid_slice_in)
               );
  net.add_edge ( mk_edge ("put slice_depth")
               , connection_t (TP, tid_tag, pid_slice_depth)
               );

  net.add_edge ( mk_edge ("get slice_depth")
               , connection_t (PT, tid_work, pid_slice_depth)
               );
  net.add_edge ( mk_edge ("read MAX_DEPTH")
               , connection_t (PT_READ, tid_work, pid_MAX_DEPTH)
               );
  net.add_edge ( mk_edge ("put slice_depth")
               , connection_t (TP, tid_work, pid_slice_depth)
               );

  net.add_edge ( mk_edge ("get slice_depth")
               , connection_t (PT, tid_untag, pid_slice_depth)
               );
  net.add_edge ( mk_edge ("read MAX_DEPTH")
               , connection_t (PT_READ, tid_untag, pid_MAX_DEPTH)
               );
  net.add_edge ( mk_edge ("put slice_out")
               , connection_t (TP, tid_untag, pid_slice_out)
               );

  net.add_edge ( mk_edge ("get slice_out")
               , connection_t (PT, tid_join, pid_slice_out)
               );
  net.add_edge ( mk_edge ("get joined")
               , connection_t (PT, tid_join, pid_joined)
               );
  net.add_edge ( mk_edge ("put joined")
               , connection_t (TP, tid_join, pid_joined)
               );
  net.add_edge ( mk_edge ("get in_progress")
               , connection_t (PT, tid_join, pid_in_progress)
               );

  net.add_edge ( mk_edge ("get splitted")
               , connection_t (PT, tid_finalize, pid_splitted)
               );
  net.add_edge ( mk_edge ("get joined")
               , connection_t (PT, tid_finalize, pid_joined)
               );
  net.add_edge ( mk_edge ("read NUM_SLICES")
               , connection_t (PT_READ, tid_finalize, pid_NUM_SLICES)
               );
  net.add_edge ( mk_edge ("put done")
               , connection_t (TP, tid_finalize, pid_done)
               );

  if (CAP_IN_PROGRESS > 0)
    net.set_capacity (pid_in_progress, CAP_IN_PROGRESS);

  // WORK HERE: not type safe at the moment
  net.put_token (pid_splitted, token::type(literal::type(0L)));
  net.put_token (pid_joined, token::type(literal::type(0L)));
  net.put_token (pid_NUM_SLICES, token::type(literal::type(NUM_SLICES)));
  net.put_token (pid_MAX_DEPTH, token::type(literal::type(MAX_DEPTH)));

  marking (net);

  {
    boost::mt19937 engine;

    Timer_t timer ("fire", NUM_SLICES * MAX_DEPTH + 4 * NUM_SLICES + 1);

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
