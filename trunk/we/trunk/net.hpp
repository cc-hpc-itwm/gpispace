// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#ifndef _NET_HPP
#define _NET_HPP

#include <netfwd.hpp>

#include <adjacency.hpp>
#include <bijection.hpp>
#include <multirel.hpp>
#include <svector.hpp>
#include <transfun.hpp>

#include <map>

#include <boost/function.hpp>

namespace petri_net
{
namespace exception
{
  class transition_not_enabled : public std::runtime_error
  {
  public:
    transition_not_enabled (const std::string & msg) : std::runtime_error(msg) 
    {}
    ~transition_not_enabled() throw () {}
  };
}

// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{
public:
  enum edge_type {PT,TP};

  typedef svector<tid_t> enabled_t;

  typedef bijection::bi_const_it<Place,pid_t> place_const_it;
  typedef bijection::bi_const_it<Transition,tid_t> transition_const_it;
  typedef bijection::bi_const_it<Edge,eid_t> edge_const_it;

  typedef adjacency::const_it<pid_t,eid_t> adj_place_const_it;
  typedef adjacency::const_it<tid_t,eid_t> adj_transition_const_it;

  typedef multirel::right_const_it<Token, pid_t> token_place_it;

  typedef TransitionFunction::Traits<Token> tf_traits;
  typedef typename tf_traits::place_via_edge_t place_via_edge_t;
  typedef typename tf_traits::token_input_t token_input_t;
  typedef typename tf_traits::input_t input_t;

  typedef typename tf_traits::output_descr_t output_descr_t;
  typedef typename tf_traits::output_t output_t;

  typedef boost::function<output_t (input_t &, output_descr_t &)> transfun_t;

private:
  bijection::bijection<Place,pid_t> pmap; // Place <-> internal id
  bijection::bijection<Transition,tid_t> tmap; // Transition <-> internal id
  bijection::bijection<Edge,eid_t> emap; // Edge <-> internal id

  typedef std::map<eid_t, pid_t> map_pid_t;
  typedef typename map_pid_t::iterator map_pid_it_t;
  typedef typename map_pid_t::const_iterator map_pid_const_it_t;

  map_pid_t emap_in_p; // internal edge id -> internal place id
  map_pid_t emap_out_p; // internal edge id -> internal place id

  typedef std::map<eid_t, tid_t> map_tid_t;
  typedef typename map_tid_t::iterator map_tid_it_t;
  typedef typename map_tid_t::const_iterator map_tid_const_it_t;

  map_tid_t emap_in_t; // internal edge id -> internal transition id
  map_tid_t emap_out_t; // internal edge id -> internal transition id

  adjacency::table<pid_t,tid_t,eid_t> adj_pt;
  adjacency::table<tid_t,pid_t,eid_t> adj_tp;

  pid_t num_places;
  tid_t num_transitions;
  eid_t num_edges;

  typename multirel::multirel<Token,pid_t> token_place_rel;

  enabled_t enabled;

  std::map<tid_t, transfun_t> transfun;

public:
  net (const pid_t & _places = 10, const tid_t & _transitions = 10)
    throw (std::bad_alloc)
    : pmap ("place")
    , tmap ("transition")
    , emap ("edge name")
    , emap_in_p ()
    , emap_out_p ()
    , emap_in_t ()
    , emap_out_t ()
    , adj_pt (eid_invalid, _places, _transitions)
    , adj_tp (eid_invalid, _transitions, _places)
    , num_places (0)
    , num_transitions (0)
    , num_edges (0)
    , token_place_rel ()
    , enabled ()
  {};

  // numbers of elements
  pid_t get_num_places (void) const { return num_places; }
  tid_t get_num_transitions (void) const { return num_transitions; }
  eid_t get_num_edges (void) const { return num_edges; }

  // get id
  const pid_t & get_place_id (const Place & place) const
    throw (bijection::exception::no_such)
  {
    return pmap.get_id (place);
  }

  const tid_t & get_transition_id (const Transition & transition) const
    throw (bijection::exception::no_such)
  {
    return tmap.get_id (transition);
  }

  const eid_t & get_edge_id (const Edge & edge) const
    throw (bijection::exception::no_such)
  {
    return emap.get_id (edge);
  }

  // get element
  const Place & place (const pid_t & pid) const
    throw (bijection::exception::no_such)
  {
    return pmap.get_elem (pid);
  }

  const Transition & transition (const tid_t & tid) const
    throw (bijection::exception::no_such)
  {
    return tmap.get_elem (tid);
  }

  const Edge & edge (const eid_t & eid) const
    throw (bijection::exception::no_such)
  {
    return emap.get_elem (eid);
  }

  // add element
  pid_t add_place (const Place & place)
    throw (bijection::exception::already_there)
  {
    ++num_places;

    return pmap.add (place);
  }

  tid_t set_transition_function (const tid_t & tid, const transfun_t & f)
  {
    transfun[tid] = f;

    return tid;
  }

  tid_t add_transition 
  ( const Transition & transition
  , const transfun_t & f = TransitionFunction::Default<Token>()
  )
    throw (bijection::exception::already_there)
  {
    ++num_transitions;

    return set_transition_function (tmap.add (transition), f);
  }

private:
  template<typename ROW, typename COL>
  eid_t add_edge ( const Edge & edge
                 , const ROW & r
                 , const COL & c
                 , adjacency::table<ROW,COL,eid_t> & m
                 )
    throw (bijection::exception::already_there)
  {
    if (m.get_adjacent (r, c) != eid_invalid)
      throw bijection::exception::already_there ("adjacency");

    const eid_t eid (emap.add (edge));

    m.set_adjacent (r, c, eid);

    ++num_edges;

    return eid;
  }

public:
  eid_t add_edge_place_to_transition ( const Edge & edge
                                     , const pid_t & pid
                                     , const tid_t & tid
                                     )
    throw (bijection::exception::already_there)
  {
    const eid_t eid (add_edge<pid_t,tid_t> (edge, pid, tid, adj_pt));

    emap_out_p[eid] = pid;
    emap_in_t[eid] = tid;

    update_enabled_transitions (tid);

    return eid;
  }

  eid_t add_edge_transition_to_place ( const Edge & edge
                                     , const tid_t & tid
                                     , const pid_t & pid
                                     )
    throw (bijection::exception::already_there)
  {
    const eid_t eid (add_edge<tid_t,pid_t> (edge, tid, pid, adj_tp));

    emap_in_p[eid] = pid;
    emap_out_t[eid] = tid;

    update_enabled_transitions (tid);

    return eid;
  }

  // iterate through elements
  place_const_it places (void) const
  {
    return place_const_it (pmap);
  }

  transition_const_it transitions (void) const
  {
    return transition_const_it (tmap);
  }

  edge_const_it edges (void) const
  {
    return edge_const_it (emap);
  }

  // iterate through adjacencies
  adj_place_const_it out_of_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_tp.row_const_it (tid));
  }

  adj_place_const_it in_to_transition (const tid_t & tid) const
  {
    return adj_place_const_it (adj_pt.col_const_it (tid));
  }

  adj_transition_const_it out_of_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_pt.row_const_it (pid));
  }

  adj_transition_const_it in_to_place (const pid_t & pid) const
  {
    return adj_transition_const_it (adj_tp.col_const_it (pid));
  }

  // get edge info
  edge_type get_edge_info ( const eid_t & eid
                          , pid_t & pid
                          , tid_t & tid
                          ) const
  {
    const map_pid_const_it_t out_p (emap_out_p.find (eid));

    if (out_p != emap_out_p.end())
      {
        const map_tid_const_it_t in_t (emap_in_t.find (eid));

        pid = out_p->second;
        tid = in_t->second;

        return PT;
      }
    else
      {
        const map_pid_const_it_t in_p (emap_in_p.find (eid));
        const map_tid_const_it_t out_t (emap_out_t.find (eid));

        tid = out_t->second;
        pid = in_p->second;

        return TP;
      }
  }

  // delete elements
private:
  const eid_t & gen_delete_edge (const eid_t & eid, const tid_t & tid)
  {
    update_enabled_transitions (tid);

    emap.erase (eid);

    --num_edges;

    return eid;
  }
  

public:
  // WORK HERE: Make more efficient by avoided two searches in the emaps
  const eid_t & delete_edge_place_to_transition ( const eid_t & eid
                                                , const pid_t & pid
                                                , const tid_t & tid
                                                )
  {
    emap_out_p.erase (eid);
    emap_in_t.erase (eid);

    adj_pt.clear_adjacent (pid, tid);

    return gen_delete_edge (eid, tid);
  }

  const eid_t & delete_edge_transition_to_place ( const eid_t & eid
                                                , const tid_t & tid
                                                , const pid_t & pid
                                                )
  {
    emap_in_p.erase (eid);
    emap_out_t.erase (eid);

    adj_tp.clear_adjacent (tid, pid);

    return gen_delete_edge (eid, tid);
  }

  const eid_t & delete_edge (const eid_t & eid)
  {
    pid_t pid;
    tid_t tid;

    edge_type et (get_edge_info (eid, pid, tid));

    return (et == PT)
      ? delete_edge_place_to_transition (eid, pid, tid)
      : delete_edge_transition_to_place (eid, tid, pid)
      ;
  }

  const pid_t & delete_place (const pid_t & pid)
    throw (bijection::exception::no_such)
  {
    token_place_rel.delete_right (pid);

    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      delete_edge_place_to_transition (tit(), pid, *tit);

    for ( adj_transition_const_it tit (in_to_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      delete_edge_transition_to_place (tit(), *tit, pid);

    pmap.erase (pid);

    --num_places;

    return pid;
  }

  const tid_t & delete_transition (const tid_t & tid)
    throw (bijection::exception::no_such)
  {
    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      delete_edge_transition_to_place (pit(), tid, *pit);

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      delete_edge_place_to_transition (pit(), *pit, tid);

    tmap.erase (tid);

    enabled.erase (tid);

    --num_transitions;

    return tid;
  }

  // modify and replace
  // erased in case of conflict after modification
  pid_t modify_place (const pid_t & pid, const Place & place)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return pmap.modify (pid, place);
  }

  // kept old value in case of conflict after modification
  pid_t replace_place (const pid_t & pid, const Place & place)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return pmap.replace (pid, place);
  }

  tid_t modify_transition ( const tid_t & tid
                          , const Transition & transition
                          )
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return tmap.modify (tid, transition);
  }

  tid_t replace_transition ( const tid_t & tid
                           , const Transition & transition
                           )
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return tmap.replace (tid, transition);
  }

  eid_t modify_edge (const eid_t & eid, const Edge & edge)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return emap.modify (eid, edge);
  }

  eid_t replace_edge (const eid_t & eid, const Edge & edge)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    return emap.replace (eid, edge);
  }

  // deal with tokens
private:
  void add_enabled_transitions (const pid_t & pid)
  {
    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      if (can_fire (*tit))
        enabled.insert (*tit);
  }

  void del_enabled_transitions (const pid_t & pid)
  {
    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      if (!can_fire (*tit))
        enabled.erase (*tit);
  }

  void update_enabled_transitions (const tid_t & tid)
  {
    if (can_fire (tid))
      {
        enabled.insert (tid);
      }
    else
      {
        enabled.erase (tid);
      }
  }

public:  
  const enabled_t & enabled_transitions (void) const
  {
    return enabled;
  }

  void verify_enabled_transitions (void) const
  {
    enabled_t comp;

    for (transition_const_it t (transitions()); t.has_more(); ++t)
      if (can_fire (*t))
        comp.insert (*t);

    assert (comp == enabled);
  }

  bool put_token (const pid_t & pid, const Token & token)
  {
    const bool successful (token_place_rel.add (token, pid));

    if (successful)
      add_enabled_transitions (pid);

    return successful;
  }

  token_place_it get_token (const pid_t & pid) const
  {
    return token_place_rel.left_of (pid);
  }

  bool has_token (const pid_t & pid) const
  {
    return token_place_rel.contains_right (pid);
  }

  std::size_t delete_one_token (const pid_t & pid, const Token & token)
  {
    const std::size_t k (token_place_rel.delete_one (token, pid));

    if (k > 0)
      del_enabled_transitions (pid);

    return (k > 0) ? 1 : 0;
  }

  std::size_t delete_all_token (const pid_t & pid, const Token & token)
  {
    const std::size_t k (token_place_rel.delete_all (token, pid));

    if (k > 0)
      del_enabled_transitions (pid);

    return k;
  }

  // WORK HERE: implement more efficient
  std::size_t replace_one_token
  (const pid_t & pid, const Token & old_token, const Token & new_token)
  {
    const std::size_t k (delete_one_token (pid, old_token));

    if (k > 0)
      put_token (pid, new_token);

    return k;
  }

  // WORK HERE: implement more efficient
  std::size_t replace_all_token
  (const pid_t & pid, const Token & old_token, const Token & new_token)
  {
    const std::size_t k (delete_all_token (pid, old_token));

    for (std::size_t i (0); i < k; ++i)
      put_token (pid, new_token);

    return k;
  }

  // FIRE
  bool can_fire (const tid_t & tid) const
  {
    bool can_fire = true;

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more() && can_fire
        ; ++pit
        )
      can_fire = has_token (*pit);

    return can_fire;
  }

  void fire (const tid_t & tid) throw (exception::transition_not_enabled)
  {
    input_t input;
    output_descr_t output_descr;

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        const token_place_it tp (get_token (*pit));

        if (!tp.has_more())
          throw exception::transition_not_enabled ("during call of fire");

        input.push_back (token_input_t (*tp, place_via_edge_t(*pit, pit())));

        delete_one_token (*pit, *tp);
      }

    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      output_descr.push_back (place_via_edge_t (*pit, pit()));

    const output_t output (transfun[tid](input, output_descr));

    for ( typename output_t::const_iterator out (output.begin())
        ; out != output.end()
        ; ++out
        )
      put_token (out->second, out->first);
  }
};
} // namespace petri_net

#endif // _NET_HPP
