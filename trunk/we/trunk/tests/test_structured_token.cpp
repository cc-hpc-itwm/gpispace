// structured token, expression transition, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/cond_exp.hpp>
#include <we/type/token.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/util/show.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/program_options.hpp>

// ************************************************************************* //

typedef std::string place_t;
typedef unsigned int transition_t;
typedef unsigned int edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

static edge_cnt_t e (0);
static transition_t t (0);

static edge_t mk_edge (const std::string & descr)
{
  return edge_t (e++, descr);
}

typedef petri_net::net<place_t, transition_t, edge_t, we::token::type> pnet_t;

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

static Function::Condition::Expression<we::token::type>
mk_cond (const pnet_t & net, const std::string & exp)
{
  return Function::Condition::Expression<we::token::type>
    ( exp
    , boost::bind (&pnet_t::get_place, boost::ref (net), _1)
    );
}

// ************************************************************************* //

class prefix_not_unique : public std::runtime_error
{
public:
  prefix_not_unique (const std::string & pref) 
    : std::runtime_error ("prefix not unique: " + pref)
  {}
};

class no_value_given : public std::runtime_error
{
public:
  no_value_given (const std::string & pref) 
    : std::runtime_error ("no_value_given for: " + pref)
  {}
};

typedef expr::eval::context<std::string> context_t;

// construct token from context

// WORK HERE: If there is a description of the target place available,
// then this function can be implemented by a series of lookups
// instead of a linear search

static we::token::type mk_token ( const std::string & pref
                                , const context_t & context
                                )
{
  // first try for single valued token (e.g. no fields)
  try
    {
      const expr::variant::type & v (context.value (pref));

      return we::token::type (v);
    }
  catch (expr::exception::eval::missing_binding<std::string> &)
    {
      /* check the complete context */

      we::token::type::map_t m;

      for ( context_t::const_iterator c (context.begin())
          ; c != context.end()
          ; ++c
          )
        {
          const std::string name (c->first);

          if (pref == name.substr (0, pref.length()))
            {
              const std::string rest (name.substr (pref.length()));

              if (rest[0] != '.')
                throw prefix_not_unique(pref);

              m[rest.substr(1)] = c->second;
            }
        }

      if (m.size() == 0)
        throw no_value_given (pref);

      return we::token::type (m);
    }

  throw no_value_given (pref);
}

class Transition
{
private:
  const std::string expression;
  const expr::parse::parser<std::string> parser;
  expr::eval::context<std::string> context;
  typedef boost::function<std::string (const pid_t &)> translate_t;
  translate_t translate;
  
public:
  explicit Transition ( const std::string & _expression
                      , const translate_t & _translate
                      )
    : expression (_expression)
    , parser (expression)
    , context ()
    , translate (_translate)
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
        const we::token::type 
          token (Function::Transition::get_token<we::token::type> (*top));

        const petri_net::pid_t
          pid (Function::Transition::get_pid<we::token::type> (*top));

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
          Function::Transition::Traits<we::token::type>::token_on_place_t top_t;

        output.push_back (top_t (mk_token (translate (pid), context), pid));
      }

    return output;
  }
};

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & expression
                                      , const std::string & condition
                                      )
{
  return net.add_transition 
    ( t++
    , Transition ( expression
                 , boost::bind (&pnet_t::get_place, boost::ref(net), _1)
                 )
    , mk_cond (net, condition)
    );
}

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & expression
                                      )
{
  return net.add_transition 
    ( t++
    , Transition ( expression
                 , boost::bind (&pnet_t::get_place, boost::ref(net), _1)
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

  petri_net::pid_t pid_NUM_SLICES (net.add_place ("NUM_SLICES"));
  petri_net::pid_t pid_MAX_DEPTH (net.add_place ("MAX_DEPTH"));
  petri_net::pid_t pid_splitted (net.add_place ("splitted"));
  petri_net::pid_t pid_slice_in (net.add_place ("slice_in"));
  petri_net::pid_t pid_slice_depth (net.add_place ("slice_depth"));
  petri_net::pid_t pid_slice_out (net.add_place ("slice_out"));
  petri_net::pid_t pid_joined (net.add_place ("joined"));
  petri_net::pid_t pid_done (net.add_place ("done"));
  petri_net::pid_t pid_in_progress (net.add_place ("in_progress"));

  petri_net::tid_t tid_split
    ( mk_transition 
      ( net
      , "${slice_in}    := ${splitted};     \
         ${splitted}    := ${splitted} + 1; \
         ${in_progress} := [];              "
      , "${splitted} < ${NUM_SLICES}"
      )
    );
  petri_net::tid_t tid_tag
    ( mk_transition
      ( net
      , "${slice_depth.slice} := ${slice_in}; ${slice_depth.depth} := 0"
      )
    );
  petri_net::tid_t tid_work
    ( mk_transition
      ( net
      , "${slice_depth.slice} := ${slice_depth.slice};   \
         ${slice_depth.depth} := ${slice_depth.depth} + 1"
      , "${slice_depth.depth} < ${MAX_DEPTH}"
      )
    );

  petri_net::tid_t tid_untag
    ( mk_transition
      ( net
      , "${slice_out} := ${slice_depth.slice}"
      , "${slice_depth.depth} >= ${MAX_DEPTH}"
      )
    );

  petri_net::pid_t tid_join
    ( mk_transition
      ( net
      , "${joined} := ${joined} + 1"
      )
    );

  petri_net::tid_t tid_finalize 
    ( mk_transition
      ( net
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

  net.put_token (pid_splitted, we::token::type(expr::variant::type(0L)));
  net.put_token (pid_joined, we::token::type(expr::variant::type(0L)));
  net.put_token (pid_NUM_SLICES, we::token::type(expr::variant::type(NUM_SLICES)));
  net.put_token (pid_MAX_DEPTH, we::token::type(expr::variant::type(MAX_DEPTH)));

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
