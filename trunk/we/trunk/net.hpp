// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#ifndef _NET_HPP
#define _NET_HPP

#include <netfwd.hpp>

#include <adjacency.hpp>
#include <bijection.hpp>
#include <cond.hpp>
#include <connection.hpp>
#include <multirel.hpp>
#include <svector.hpp>
#include <trans.hpp>

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include <boost/function.hpp>

namespace petri_net
{
  namespace exception
  {
    class transition_not_enabled : public std::runtime_error
    {
    public:
      transition_not_enabled (const std::string & msg)
        : std::runtime_error(msg)
      {}
      ~transition_not_enabled() throw () {}
    };

    class no_such : public std::runtime_error
    {
    public:
      no_such (const std::string & msg) : std::runtime_error (msg) {}
      ~no_such () throw () {}
    };
  }

  enum edge_type {PT,TP};

  typedef adjacency::const_it<pid_t,eid_t> adj_place_const_it;
  typedef adjacency::const_it<tid_t,eid_t> adj_transition_const_it;

  typedef connection<edge_type, tid_t, pid_t> connection_t;

// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{
public:
  typedef bijection::const_it<Place,pid_t> place_const_it;
  typedef bijection::const_it<Transition,tid_t> transition_const_it;
  typedef bijection::const_it<Edge,eid_t> edge_const_it;

  typedef multirel::right_const_it<Token, pid_t> token_place_it;

  typedef Function::Transition::Traits<Token> tf_traits;
  typedef typename tf_traits::place_via_edge_t place_via_edge_t;
  typedef typename tf_traits::token_input_t token_input_t;
  typedef typename tf_traits::input_t input_t;

  typedef typename tf_traits::output_descr_t output_descr_t;
  typedef typename tf_traits::output_t output_t;

  typedef typename tf_traits::fun_t trans_t;
  typedef std::tr1::unordered_map<tid_t, trans_t> trans_map_t;

  typedef Function::Condition::Traits<Token> cd_traits;
  typedef typename cd_traits::in_cond_t in_cond_t;
  typedef typename cd_traits::out_cond_t out_cond_t;

  typedef std::tr1::unordered_map<tid_t, in_cond_t> in_cond_map_t;
  typedef std::tr1::unordered_map<tid_t, out_cond_t> out_cond_map_t;

  typedef std::pair<input_t, output_descr_t> enabled_descr_t;
  typedef svector<tid_t> enabled_t;

private:
  bijection::bijection<Place,pid_t> pmap; // Place <-> internal id
  bijection::bijection<Transition,tid_t> tmap; // Transition <-> internal id
  bijection::bijection<Edge,eid_t> emap; // Edge <-> internal id

  typedef std::tr1::unordered_map<eid_t, connection_t> connection_map_t;

  connection_map_t connection_map;

  adjacency::table<pid_t,tid_t,eid_t> adj_pt;
  adjacency::table<tid_t,pid_t,eid_t> adj_tp;

  pid_t num_places;
  tid_t num_transitions;
  eid_t num_edges;

  typename multirel::multirel<Token,pid_t> token_place_rel;

  enabled_t enabled;

  trans_map_t trans;
  in_cond_map_t in_cond;
  out_cond_map_t out_cond;

public:
  net (const pid_t & _places = 10, const tid_t & _transitions = 10)
    throw (std::bad_alloc)
    : pmap ("place")
    , tmap ("transition")
    , emap ("edge name")
    , connection_map ()
    , adj_pt (eid_invalid, _places, _transitions)
    , adj_tp (eid_invalid, _transitions, _places)
    , num_places (0)
    , num_transitions (0)
    , num_edges (0)
    , token_place_rel ()
    , enabled ()
    , trans ()
    , in_cond ()
    , out_cond ()
  {};

  // numbers of elements
  pid_t get_num_places (void) const { return num_places; }
  tid_t get_num_transitions (void) const { return num_transitions; }
  eid_t get_num_edges (void) const { return num_edges; }

  // condition accessores
  const trans_map_t & get_trans (void) const { return trans; }
  const in_cond_map_t & get_in_cond (void) const { return in_cond; }
  const out_cond_map_t & get_out_cond (void) const { return out_cond; }

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

  void set_transition_function (const tid_t & tid, const trans_t & f)
  {
    trans[tid] = f;
  }

  // WORK HERE: update enabled
  void set_in_condition_function (const tid_t & tid, const in_cond_t & f)
  {
    in_cond[tid] = f;
  }

  // WORK HERE: update enabled
  void set_out_condition_function (const tid_t & tid, const out_cond_t & f)
  {
    out_cond[tid] = f;
  }

  tid_t add_transition
  ( const Transition & transition
  , const trans_t & tf = Function::Transition::Default<Token>()
  , const in_cond_t & inc = Function::Condition::In::Default<Token>()
  , const out_cond_t & outc = Function::Condition::Out::Default<Token>()
  )
    throw (bijection::exception::already_there)
  {
    ++num_transitions;

    const tid_t tid (tmap.add (transition));

    set_transition_function (tid, tf);
    set_in_condition_function (tid, inc);
    set_out_condition_function (tid, outc);

    return tid;
  }

private:
  template<typename ROW, typename COL>
  eid_t gen_add_edge ( const Edge & edge
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
  eid_t add_edge (const Edge & edge, const connection_t & connection)
  {
    const eid_t eid
      ( (connection.type == PT)
      ? gen_add_edge<pid_t,tid_t> (edge, connection.pid, connection.tid, adj_pt)
      : gen_add_edge<tid_t,pid_t> (edge, connection.tid, connection.pid, adj_tp)
      );

    connection_map[eid] = connection;

    update_enabled_transitions (connection.tid);

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
  connection_t get_edge_info (const eid_t & eid) const
    throw (exception::no_such)
  {
    const typename connection_map_t::const_iterator it
      (connection_map.find (eid));

    if (it == connection_map.end())
      throw exception::no_such ("connection");

    return it->second;
  }

  // delete elements
  const eid_t & delete_edge (const eid_t & eid)
  {
    typename connection_map_t::iterator it (connection_map.find (eid));

    if (it == connection_map.end())
      throw exception::no_such ("connection");

    const connection_t connection (it->second);

    if (connection.type == PT)
      {
        adj_pt.clear_adjacent (connection.pid, connection.tid);
      }
    else
      {
        adj_tp.clear_adjacent (connection.tid, connection.pid);
      }

    update_enabled_transitions (connection.tid);

    connection_map.erase (it);

    emap.erase (eid);

    --num_edges;

    return eid;
  }

  const pid_t & delete_place (const pid_t & pid)
    throw (bijection::exception::no_such)
  {
    token_place_rel.delete_right (pid);

    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      delete_edge (tit());

    for ( adj_transition_const_it tit (in_to_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      delete_edge (tit());

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
      delete_edge (pit());

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      delete_edge (pit());

    tmap.erase (tid);

    enabled.erase (tid);

    trans.erase (tid);
    in_cond.erase (tid);
    out_cond.erase (tid);

    --num_transitions;

    return tid;
  }

  // WORK HERE: update the enabled structures
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
  typedef typename std::pair<Token,eid_t> token_via_edge_t;
  typedef std::vector<token_via_edge_t> vec_token_via_edge_t;
  typedef std::tr1::unordered_map<pid_t,vec_token_via_edge_t> pid_in_map_t;

  typedef std::tr1::unordered_set<tid_t> set_of_tid_t;
  typedef set_of_tid_t in_enabled_t;
  typedef set_of_tid_t out_enabled_t;

  typedef std::tr1::unordered_map<tid_t,pid_in_map_t> in_map_t;
  typedef std::tr1::unordered_map<tid_t,output_descr_t> out_map_t;

  in_enabled_t in_enabled;
  out_enabled_t out_enabled;
  enabled_t new_enabled;

  in_map_t in_map;
  out_map_t out_map;

  void update_new_enabled ( const tid_t & tid
                          , const bool can_fire
                          , set_of_tid_t & a
                          , set_of_tid_t & b
                          )
  {
    if (can_fire)
      {
        a.insert (tid);

        if (b.find (tid) != b.end())
          new_enabled.insert (tid);
      }
    else
      {
        a.erase (tid);
        new_enabled.erase (tid);
      }
  }

  const pid_in_map_t & update_in_enabled (const tid_t & tid)
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    
    typename std::tr1::unordered_map<tid_t, in_cond_t>::const_iterator f
      (get_in_cond().find(tid));

    assert (f != get_in_cond().end());

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        vec_token_via_edge_t & vec_token_via_edge (pid_in_map[*pit]);

        vec_token_via_edge.clear();

        const place_via_edge_t place_via_edge (*pit, pit());

        for (token_place_it tp (get_token (*pit)); tp.has_more(); ++tp)
          if (f->second(token_input_t (*tp, place_via_edge)))
            vec_token_via_edge.push_back(token_via_edge_t (*tp, pit()));
      }

    update_new_enabled ( tid
                       , pid_in_map.size() == in_to_transition (tid).size()
                       , in_enabled
                       , out_enabled
                       );

    return pid_in_map;
  }

  const output_descr_t & update_out_enabled (const tid_t & tid)
  {
    output_descr_t & output_descr (out_map[tid]);
    
    output_descr.clear();

    typename std::tr1::unordered_map<tid_t, out_cond_t>::const_iterator f
      (get_out_cond().find(tid));

    assert (f != get_out_cond().end());

    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        place_via_edge_t place_via_edge (*pit, pit());

        if (f->second (place_via_edge))
          output_descr.push_back (place_via_edge);
      }

    update_new_enabled ( tid
                       , output_descr.size() == out_of_transition (tid).size()
                       , out_enabled
                       , in_enabled
                       );

    return output_descr;
  }

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

  std::size_t num_token (const pid_t & pid) const
  {
    return token_place_rel.left_of(pid).size();
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

    const typename trans_map_t::const_iterator f (get_trans().find (tid));

    assert (f != get_trans().end());

    const output_t output (f->second(input, output_descr));

    for ( typename output_t::const_iterator out (output.begin())
        ; out != output.end()
        ; ++out
        )
      put_token (out->second, out->first);
  }
};
} // namespace petri_net

#endif // _NET_HPP
