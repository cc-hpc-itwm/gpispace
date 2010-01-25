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

#include <deque>

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

// WORK HERE: Performance: collect map<tid_t,X>, map<tid_t,Y> into a
// single map<tid_t,(X,Y)>?

// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{

  // *********************************************************************** //
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

  typedef typename std::pair<Token,eid_t> token_via_edge_t;

  typedef std::deque<token_via_edge_t> deque_token_via_edge_t;
  //  typedef std::vector<token_via_edge_t> deque_token_via_edge_t;
  typedef std::tr1::unordered_map<pid_t,deque_token_via_edge_t> pid_in_map_t;

  typedef svector<tid_t> enabled_t;

  // *********************************************************************** //
private:
  typedef std::tr1::unordered_map<eid_t, connection_t> connection_map_t;
  typedef typename multirel::multirel<Token,pid_t> token_place_rel_t;

  typedef std::tr1::unordered_set<tid_t> set_of_tid_t;
  typedef set_of_tid_t in_enabled_t;
  typedef set_of_tid_t out_enabled_t;

  typedef std::tr1::unordered_map<tid_t,pid_in_map_t> in_map_t;
  typedef std::tr1::unordered_map<tid_t,output_descr_t> out_map_t;

  // *********************************************************************** //

  bijection::bijection<Place,pid_t> pmap; // Place <-> internal id
  bijection::bijection<Transition,tid_t> tmap; // Transition <-> internal id
  bijection::bijection<Edge,eid_t> emap; // Edge <-> internal id

  connection_map_t connection_map;

  adjacency::table<pid_t,tid_t,eid_t> adj_pt;
  adjacency::table<tid_t,pid_t,eid_t> adj_tp;

  pid_t num_places;
  tid_t num_transitions;
  eid_t num_edges;

  token_place_rel_t token_place_rel;

  enabled_t enabled;

  trans_map_t trans;
  in_cond_map_t in_cond;
  out_cond_map_t out_cond;

  in_map_t in_map;
  out_map_t out_map;
  in_enabled_t in_enabled;
  out_enabled_t out_enabled;

  // *********************************************************************** //

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

  void update_set_of_tid ( const tid_t & tid
                         , const bool can_fire
                         , set_of_tid_t & a
                         , set_of_tid_t & b
                         )
  {
    if (can_fire)
      {
        a.insert (tid);

        if (b.find (tid) != b.end())
          enabled.insert (tid);
      }
    else
      {
        a.erase (tid);
        enabled.erase (tid);
      }
  }

  void recalculate_pid_in_map ( pid_in_map_t & pid_in_map
                              , const in_cond_t & f
                              , const pid_t & pid
                              , const eid_t & eid
                              )
  {
    deque_token_via_edge_t & deque_token_via_edge (pid_in_map[pid]);

    deque_token_via_edge.clear();

    for (token_place_it tp (get_token (pid)); tp.has_more(); ++tp)
      if (f(*tp, pid, eid))
        deque_token_via_edge.push_back(token_via_edge_t (*tp, eid));

    if (deque_token_via_edge.empty())
      pid_in_map.erase (pid);
  }

  void recalculate_enabled_by_place (const pid_t & pid)
  {
    for (adj_transition_const_it t (in_to_place (pid)); t.has_more(); ++t)
      update_out_enabled (*t, pid, t());

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      recalculate_in_enabled (*t, pid, t());
  }

  void recalculate_enabled_by_edge (const eid_t & eid)
  {
    const connection_t connection (get_edge_info (eid));

    if (connection.type == PT)
      recalculate_in_enabled (connection.tid, connection.pid, eid);
    else
      update_out_enabled (connection.tid, connection.pid, eid);
  }

  void recalculate_in_enabled ( const tid_t & tid
                              , const pid_t & pid
                              , const eid_t & eid
                              )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    
    const typename in_cond_map_t::const_iterator f (get_in_cond().find(tid));

    assert (f != get_in_cond().end());

    recalculate_pid_in_map (pid_in_map, f->second, pid, eid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void calculate_in_enabled (const tid_t & tid)
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    
    const typename in_cond_map_t::const_iterator f (get_in_cond().find(tid));

    assert (f != get_in_cond().end());

    adj_place_const_it pit (in_to_transition (tid));

    for (; pit.has_more(); ++pit)
      recalculate_pid_in_map (pid_in_map, f->second, *pit, pit());

    update_set_of_tid ( tid
                      , pid_in_map.size() == pit.size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_in_enabled_put_token ( const tid_t & tid
                                   , const pid_t & pid
                                   , const eid_t & eid
                                   , const Token & token
                                   )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    deque_token_via_edge_t & deque_token_via_edge (pid_in_map[pid]);
    
    const typename in_cond_map_t::const_iterator f (get_in_cond().find(tid));

    assert (f != get_in_cond().end());

    if (f->second(token, pid, eid))
      deque_token_via_edge.push_back(token_via_edge_t (token, eid));

    if (deque_token_via_edge.empty())
      pid_in_map.erase (pid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_in_enabled_del_one_token ( const tid_t & tid
                                       , const pid_t & pid
                                       , const Token & token
                                       )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    deque_token_via_edge_t & deque_token_via_edge (pid_in_map[pid]);
    typename deque_token_via_edge_t::iterator it (deque_token_via_edge.begin());

    while (it != deque_token_via_edge.end() && it->first != token)
      ++it;

    if (it != deque_token_via_edge.end())
      {
        assert (it->first == token);

        deque_token_via_edge.erase (it);
      }

    if (deque_token_via_edge.empty())
      pid_in_map.erase (pid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_in_enabled_del_all_token ( const tid_t & tid
                                       , const pid_t & pid
                                       , const Token & token
                                       )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    deque_token_via_edge_t & deque_token_via_edge (pid_in_map[pid]);
    const deque_token_via_edge_t old (deque_token_via_edge);

    deque_token_via_edge.clear();

    for ( typename deque_token_via_edge_t::const_iterator it (old.begin())
        ; it != old.end()
        ; ++it
        )
      if (it->first != token)
        deque_token_via_edge.push_back (*it);

    if (deque_token_via_edge.empty())
      pid_in_map.erase (pid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_output_descr ( output_descr_t & output_descr
                           , const out_cond_t & f
                           , const pid_t & pid
                           , const eid_t & eid
                           )
  {
    if (f(pid, eid))
      {
        output_descr[pid] = eid;
      }
    else
      {
        output_descr.erase (pid);
      }
  }

  void calculate_out_enabled (const tid_t & tid)
  {
    output_descr_t & output_descr (out_map[tid]);
    
    const typename out_cond_map_t::const_iterator f (get_out_cond().find(tid));

    assert (f != get_out_cond().end());

    adj_place_const_it pit (out_of_transition (tid));

    for (; pit.has_more(); ++pit)
      update_output_descr (output_descr, f->second, *pit, pit());

    update_set_of_tid ( tid
                      , output_descr.size() == pit.size()
                      , out_enabled
                      , in_enabled
                      );
  }

  void update_out_enabled ( const tid_t & tid
                          , const pid_t & pid
                          , const eid_t & eid
                          )
  {
    output_descr_t & output_descr (out_map[tid]);
    
    const typename out_cond_map_t::const_iterator f (get_out_cond().find(tid));

    assert (f != get_out_cond().end());

    update_output_descr (output_descr, f->second, pid, eid);

    update_set_of_tid ( tid
                      , output_descr.size() == out_of_transition(tid).size()
                      , out_enabled
                      , in_enabled
                      );
  }

  // *********************************************************************** //

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
    , in_map ()
    , out_map ()
    , in_enabled ()
    , out_enabled ()
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

  void set_in_condition_function (const tid_t & tid, const in_cond_t & f)
  {
    in_cond[tid] = f;

    calculate_in_enabled (tid);
  }

  void set_out_condition_function (const tid_t & tid, const out_cond_t & f)
  {
    out_cond[tid] = f;

    calculate_out_enabled (tid);
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

  eid_t add_edge (const Edge & edge, const connection_t & connection)
  {
    const eid_t eid
      ( (connection.type == PT)
      ? gen_add_edge<pid_t,tid_t> (edge, connection.pid, connection.tid, adj_pt)
      : gen_add_edge<tid_t,pid_t> (edge, connection.tid, connection.pid, adj_tp)
      );

    connection_map[eid] = connection;

    if (connection.type == PT)
      recalculate_in_enabled (connection.tid, connection.pid, eid);
    else
      update_out_enabled (connection.tid, connection.pid, eid);

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
        recalculate_in_enabled (connection.tid, connection.pid, eid);
      }
    else
      {
        adj_tp.clear_adjacent (connection.tid, connection.pid);
        update_out_enabled (connection.tid, connection.pid, eid);
      }

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

    trans.erase (tid);
    in_cond.erase (tid);
    out_cond.erase (tid);

    in_enabled.erase (tid);
    out_enabled.erase (tid);
    enabled.erase (tid);
    in_map.erase (tid);
    out_map.erase (tid);

    --num_transitions;

    return tid;
  }

  // modify and replace
  // erased in case of conflict after modification
  pid_t modify_place (const pid_t & pid, const Place & place)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    const pid_t new_pid (pmap.modify (pid, place));

    recalculate_enabled_by_place (new_pid);

    return new_pid;
  }

  // kept old value in case of conflict after modification
  pid_t replace_place (const pid_t & pid, const Place & place)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    const pid_t new_pid (pmap.replace (pid, place));

    recalculate_enabled_by_place (new_pid);

    return new_pid;
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
    const eid_t new_eid (emap.modify (eid, edge));

    recalculate_enabled_by_edge (new_eid);

    return new_eid;
  }

  eid_t replace_edge (const eid_t & eid, const Edge & edge)
    throw (bijection::exception::no_such, bijection::exception::already_there)
  {
    const eid_t new_eid (emap.replace (eid, edge));

    recalculate_enabled_by_edge (new_eid);

    return new_eid;
  }

  // deal with tokens
  const pid_in_map_t & get_pid_in_map (const tid_t & tid) const
    throw (exception::no_such)
  {
    const typename in_map_t::const_iterator m (in_map.find (tid));

    if (m == in_map.end())
      throw exception::no_such ("transition in in_map");

    return m->second;
  }

  const output_descr_t & get_output_descr (const tid_t & tid) const
    throw (exception::no_such)
  {
    const typename out_map_t::const_iterator m (out_map.find (tid));

    if (m == out_map.end())
      throw exception::no_such ("transition in out_map");

    return m->second;
  }

  const enabled_t & enabled_transitions (void) const
  {
    return enabled;
  }

  bool put_token (const pid_t & pid, const Token & token)
  {
    const bool successful (token_place_rel.add (token, pid));

    if (successful)
      {
        for (adj_transition_const_it t (in_to_place (pid)); t.has_more(); ++t)
          update_out_enabled (*t, pid, t());

        for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
          update_in_enabled_put_token (*t, pid, t(), token);
      }

    return successful;
  }

  bool put_token (const pid_t & pid)
  {
    return put_token (pid, Token());
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
    for (adj_transition_const_it t (in_to_place (pid)); t.has_more(); ++t)
      update_out_enabled (*t, pid, t());

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      update_in_enabled_del_one_token (*t, pid, token);

    return (token_place_rel.delete_one (token, pid));
  }

  std::size_t delete_all_token (const pid_t & pid, const Token & token)
  {
    for (adj_transition_const_it t (in_to_place (pid)); t.has_more(); ++t)
      update_out_enabled (*t, pid, t());

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      update_in_enabled_del_all_token (*t, pid, token);

    return (token_place_rel.delete_all (token, pid));
  }

  // WORK HERE: implement more efficient?
  std::size_t replace_one_token
  (const pid_t & pid, const Token & old_token, const Token & new_token)
  {
    const std::size_t k (delete_one_token (pid, old_token));

    if (k > 0)
      put_token (pid, new_token);

    return k;
  }

  // WORK HERE: implement more efficient?
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
    return enabled.elem(tid);
  }

  void fire (const tid_t & tid) throw (exception::transition_not_enabled)
  {
    if (!can_fire (tid))
      throw exception::transition_not_enabled ("during call of fire");
      
    const typename trans_map_t::const_iterator f (get_trans().find (tid));

    assert (f != get_trans().end());

    output_descr_t output_descr (out_map[tid]);
    pid_in_map_t pid_in_map (in_map[tid]);

    input_t input;

    for ( typename pid_in_map_t::iterator pid_in_map_it (pid_in_map.begin())
        ; pid_in_map_it != pid_in_map.end()
        ; ++pid_in_map_it
        )
      {
        const pid_t pid (pid_in_map_it->first);
        deque_token_via_edge_t & deque_token_via_edge (pid_in_map_it->second);

        const token_via_edge_t token_via_edge (deque_token_via_edge.front());

        deque_token_via_edge.erase (deque_token_via_edge.begin());
        //deque_token_via_edge.pop_front();

        const Token token (token_via_edge.first);
        const eid_t eid (token_via_edge.second);

        input.push_back (token_input_t (token, place_via_edge_t(pid, eid)));

        delete_one_token (pid, token);
      }

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
