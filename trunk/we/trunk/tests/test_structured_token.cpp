// control loop with multi-token condition, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/cond_exp.hpp>
#include <we/type/token.hpp>

#include <we/util/show.hpp>

#include "timer.hpp"

#include <string>

using std::cout;
using std::endl;

typedef std::string place_t;
typedef std::string transition_t;
typedef unsigned short edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

static edge_t mk_edge (const std::string & descr)
{
  static edge_cnt_t e (0);

  return edge_t (e++, descr);
}

typedef petri_net::net<place_t, transition_t, edge_t, we::token::type> pnet_t;

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "[" << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "]";
    }
  cout << endl;
}

using petri_net::connection_t;
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

static Function::Condition::Expression<we::token::type>
mk_cond (pnet_t & net, const std::string & exp)
{
  return Function::Condition::Expression<we::token::type>
    ( exp
    , boost::bind (&pnet_t::get_place, boost::ref (net), _1)
    );
}

int
main ()
{
  pnet_t net;

  petri_net::pid_t pid_NUM_SLICES (net.add_place ("NUM_SLICES"));
  petri_net::pid_t pid_MAX_DEPTH (net.add_place ("MAX_DEPTH"));
  petri_net::pid_t pid_splitted (net.add_place ("splitted"));
  petri_net::pid_t pid_slice_in (net.add_place ("slice_in"));
  petri_net::pid_t pid_slice_depth (net.add_place ("slice_depth"));
  petri_net::pid_t pid_slice_out (net.add_place ("slice_out"));
  petri_net::pid_t pid_joined (net.add_place ("joined"));
  petri_net::pid_t pid_done (net.add_place ("done"));

  petri_net::tid_t tid_split
    ( net.add_transition
      ("${slice_in} := ${splitted}; ${splitted} := ${splitted} + 1")
    );
  petri_net::tid_t tid_tag
    ( net.add_transition
      ("${slice_depth.slice} := ${slice_in}; ${slice_depth.depth} := 0")
    );
  petri_net::tid_t tid_work
    ( net.add_transition
      ("${slice_depth.slice} := ${slice_depth.slice};   \
        ${slice_depth.depth} := ${slice_depth.depth} + 1"
      )
    );

  petri_net::tid_t tid_untag
    (net.add_transition("${slice_out} := ${slice_depth.slice}"));

  petri_net::pid_t tid_join
    (net.add_transition ("${joined} := ${joined} + 1"));

  petri_net::tid_t tid_finalize (net.add_transition ("${done} := 1"));

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
  net.add_edge ( mk_edge ("put slice_out")
               , connection_t (PT, tid_untag, pid_slice_out)
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

  net.set_choice_condition_function 
    (tid_split, mk_cond (net, "${splitted} < ${NUM_SLICES}"));

  net.set_choice_condition_function 
    (tid_work, mk_cond (net, "${slice_depth.depth} < ${MAX_DEPTH}"));

  net.set_choice_condition_function 
    (tid_untag, mk_cond (net, "${slice_depth.depth} >= ${MAX_DEPTH}"));

  net.set_choice_condition_function 
    ( tid_finalize
    , mk_cond (net, "${joined} == ${NUM_SLICES} & ${splitted} == ${NUM_SLICES}")
    );

  net.put_token (pid_splitted, we::token::type(expr::variant::type(0L)));
  net.put_token (pid_joined, we::token::type(expr::variant::type(0L)));
  net.put_token (pid_NUM_SLICES, we::token::type(expr::variant::type(3L)));
  net.put_token (pid_MAX_DEPTH, we::token::type(expr::variant::type(3L)));

  we::token::type::map_t m;

  m["slice"] = expr::variant::type(1L);
  m["depth"] = expr::variant::type(2L);

  net.put_token (pid_slice_depth, we::token::type(m));

  marking (net);

  {
    boost::mt19937 engine;

    Timer_t timer ("fire");

    while (!net.enabled_transitions().empty())
      {
        net.fire(net.enabled_transitions().at(0));
        marking (net);
      }
  }

  marking (net);

  return EXIT_SUCCESS;
}
