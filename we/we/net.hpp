// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#ifndef _NET_HPP
#define _NET_HPP

#include <cassert>

#include <we/container/adjacency.hpp>
#include <we/container/bijection.hpp>
#include <we/container/multirel.hpp>
#include <we/container/priostore.hpp>
#include <we/function/cond.hpp>
#include <we/function/trans.hpp>
#include <we/serialize/unordered_map.hpp>
#include <we/serialize/unordered_set.hpp>
#include <we/type/connection.hpp>
#include <we/type/id.hpp>
#include <we/util/cross.hpp>

#include <fhg/util/show.hpp>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

#include <boost/function.hpp>
#include <boost/foreach.hpp>

#include <boost/serialization/nvp.hpp>

#include <stack>

namespace petri_net
{
  namespace exception
  {
    class generic : public std::runtime_error
    {
    public:
      explicit generic (const std::string& msg)
        : std::runtime_error ("net: " + msg)
      {}
    };

    class transition_not_enabled : public generic
    {
    public:
      explicit transition_not_enabled (const std::string & msg)
        : generic ("transition_not_enabled: " + msg)
      {}
    };

    class capacity_exceeded : public generic
    {
    public:
      explicit capacity_exceeded ()
        : generic ("capacity exceeded")
      {}
    };

    class no_such : public generic
    {
    public:
      explicit no_such (const std::string & msg)
        : generic ("no_such: " + msg)
      {}
    };
  }

  enum edge_type {PT,PT_READ,TP};

  static inline bool is_pt_read (const edge_type & e)
  {
    return e == PT_READ;
  }

  static inline bool is_PT (const edge_type & et)
  {
    return (et == PT || et == PT_READ);
  }

  static inline edge_type pt_read (void)
  {
    return PT_READ;
  }

  typedef adjacency::const_it<pid_t,eid_t> adj_place_const_it;
  typedef adjacency::const_it<tid_t,eid_t> adj_transition_const_it;

  typedef connection<edge_type, tid_t, pid_t> connection_t;

  namespace detail
  {
    struct capacity_and_placeholder_type
    {
    private:
      capacity_t _left;
      capacity_t _used;
    public:
      capacity_and_placeholder_type () : _left (0), _used (0) {}
      explicit capacity_and_placeholder_type (const capacity_t& cap)
        : _left (cap)
        , _used (0)
      {}
      const capacity_t& left () const { return _left; }
      const capacity_t capacity () const { return _left + _used; }
      void extract () { assert (_left > 0); --_left; ++_used; }
      void put () { if (_used > 0) { --_used; ++_left; } }

      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(_left);
        ar & BOOST_SERIALIZATION_NVP(_used);
      }
    };
  }

// WORK HERE: Performance: collect map<tid_t,X>, map<tid_t,Y> into a
// single map<tid_t,(X,Y)>?

// WORK HERE: Performance: The update mechanism is not optimal in all
// cases, e.g. if a token is putted to a place, re-evaluting the
// condition of already enabled transitions is not neccessary

// the net itself
template<typename Place, typename Transition, typename Edge, typename Token>
class net
{

  // *********************************************************************** //
public:
  typedef Place place_type;
  typedef Transition transition_type;
  typedef Edge edge_type;
  typedef Token token_type;

  typedef bijection::const_it<place_type,pid_t> place_const_it;
  typedef bijection::const_it<transition_type,tid_t> transition_const_it;
  typedef bijection::const_it<edge_type,eid_t> edge_const_it;
  typedef typename place_const_it::size_type size_type;

  typedef multirel::right_const_it<token_type, pid_t> token_place_it;

  // TODO: traits should be template parameters (with default values)
  typedef Function::Transition::Traits<token_type> tf_traits;
  typedef typename tf_traits::place_via_edge_t place_via_edge_t;
  typedef typename tf_traits::token_input_t token_input_t;
  typedef typename tf_traits::input_t input_t;

  typedef typename tf_traits::output_descr_t output_descr_t;
  typedef typename tf_traits::output_t output_t;

  typedef typename tf_traits::fun_t trans_t;
  typedef boost::unordered_map<tid_t, trans_t> trans_map_t;

  // TODO: traits should be template parameters (with default values)
  typedef Function::Condition::Traits<token_type> cd_traits;

  typedef typename cd_traits::token_via_edge_t token_via_edge_t;
  typedef typename cd_traits::vec_token_via_edge_t vec_token_via_edge_t;
  typedef typename cd_traits::pid_in_map_t pid_in_map_t;
  typedef typename cd_traits::choices_t choices_t;

  typedef typename cross::iterator<pid_in_map_t> choice_it;

  typedef priostore::type<tid_t> enabled_t;

  // *********************************************************************** //
private:
  typedef boost::unordered_map<eid_t, connection_t> connection_map_t;
  typedef typename multirel::multirel<token_type,pid_t> token_place_rel_t;

  typedef boost::unordered_set<tid_t> set_of_tid_t;
  typedef set_of_tid_t in_enabled_t;
  typedef set_of_tid_t out_enabled_t;

  typedef typename cross::Traits<pid_in_map_t>::vec_t choice_vec_t;
  typedef boost::unordered_map<tid_t, choice_vec_t> enabled_choice_t;

  typedef boost::unordered_map<tid_t,pid_in_map_t> in_map_t;
  typedef boost::unordered_map<tid_t,output_descr_t> out_map_t;

  typedef boost::unordered_map< pid_t
                              , detail::capacity_and_placeholder_type
                              > capacity_map_t;

  // *********************************************************************** //

  std::string name;
  bijection::bijection<place_type,pid_t> pmap; // place_type <-> internal id
  bijection::bijection<transition_type,tid_t> tmap; // transition_type <-> internal id
  bijection::bijection<edge_type,eid_t> emap; // edge_type <-> internal id

  connection_map_t connection_map;

  adjacency::table<pid_t,tid_t,eid_t> adj_pt;
  adjacency::table<tid_t,pid_t,eid_t> adj_tp;

  token_place_rel_t token_place_rel;

  enabled_t enabled;
  enabled_choice_t enabled_choice_consume;
  enabled_choice_t enabled_choice_read;

  trans_map_t trans;

  capacity_map_t capacity_map;

  in_map_t in_map;
  out_map_t out_map;
  in_enabled_t in_enabled;
  out_enabled_t out_enabled;

  // *********************************************************************** //

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(name);
    ar & BOOST_SERIALIZATION_NVP(pmap);
    ar & BOOST_SERIALIZATION_NVP(tmap);
    ar & BOOST_SERIALIZATION_NVP(emap);
    ar & BOOST_SERIALIZATION_NVP(connection_map);
    ar & BOOST_SERIALIZATION_NVP(adj_pt);
    ar & BOOST_SERIALIZATION_NVP(adj_tp);
    ar & BOOST_SERIALIZATION_NVP(token_place_rel);
    ar & BOOST_SERIALIZATION_NVP(enabled);
    ar & BOOST_SERIALIZATION_NVP(enabled_choice_consume);
    ar & BOOST_SERIALIZATION_NVP(enabled_choice_read);
    // WORK HERE: serialize the functions
    // AP: we do not need this with the new transitions
    //     because the function is already encapsulated
    //
    //    ar & BOOST_SERIALIZATION_NVP(trans);
    ar & BOOST_SERIALIZATION_NVP(capacity_map);
    ar & BOOST_SERIALIZATION_NVP(in_map);
    ar & BOOST_SERIALIZATION_NVP(out_map);
    ar & BOOST_SERIALIZATION_NVP(in_enabled);
    ar & BOOST_SERIALIZATION_NVP(out_enabled);
  }

  // *********************************************************************** //

  template<typename ROW, typename COL>
  eid_t gen_add_edge ( const edge_type & edge
                     , const ROW & r
                     , const COL & c
                     , adjacency::table<ROW,COL,eid_t> & m
                     )
  {
    if (m.get_adjacent (r, c) != eid_invalid)
      throw bijection::exception::already_there ("adjacency");

    const eid_t eid (emap.add (edge));

    m.set_adjacent (r, c, eid);

    return eid;
  }

  template<typename MAP>
  const typename MAP::mapped_type & get_fun ( const MAP & map
                                            , const tid_t & tid
                                            ) const
  {
    const typename MAP::const_iterator f (map.find (tid));

    if (f == map.end())
      throw exception::no_such ("transition id in get_fun");

    return f->second;
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
          {
            choices_t cs (choices(tid));

            // call the global condition function here, that sets the
            // cross product either to the end or to some valid choice

            if (get_transition (tid).condition (cs))
              {
                enabled.insert (tid);

                enabled_choice_consume[tid].clear();
                enabled_choice_read[tid].clear();

                for ( typename cross::iterator<pid_in_map_t> choice (*cs)
                    ; choice.has_more()
                    ; ++choice
                    )
                  {
                    const token_via_edge_t & token_via_edge (choice.val());
                    const eid_t & eid (token_via_edge.second);

                    if (is_pt_read (get_edge_info (eid).type))
                      {
                        enabled_choice_read[tid].push_back (*choice);
                      }
                    else
                      {
                        enabled_choice_consume[tid].push_back (*choice);
                      }
                  }
              }
            else
              {
                enabled.erase (tid);
              }
          }
      }
    else
      {
        a.erase (tid);
        enabled.erase (tid);
      }
  }

  void recalculate_pid_in_map ( pid_in_map_t & pid_in_map
                              , const pid_t & pid
                              , const eid_t & eid
                              )
  {
    vec_token_via_edge_t & vec_token_via_edge (pid_in_map[pid]);

    vec_token_via_edge.clear();

    for (token_place_it tp (get_token (pid)); tp.has_more(); ++tp)
      vec_token_via_edge.push_back(token_via_edge_t (*tp, eid));

    if (vec_token_via_edge.empty())
      pid_in_map.erase (pid);
  }

  void recalculate_enabled_by_place (const pid_t & pid)
  {
    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      recalculate_in_enabled (*t, pid, t());

    recalculate_out_enabled_by_place (pid);
  }

  void recalculate_out_enabled_by_place (const pid_t & pid)
  {
    for (adj_transition_const_it t (in_to_place (pid)); t.has_more(); ++t)
      update_out_enabled (*t, pid, t());
  }

  void recalculate_enabled_by_edge ( const eid_t & eid
                                   , const connection_t & connection
                                   )
  {
    if (is_PT (connection.type))
      {
        recalculate_in_enabled (connection.tid, connection.pid, eid);
      }
    else
      {
        update_out_enabled (connection.tid, connection.pid, eid);
      }
  }

  void recalculate_enabled_by_edge (const eid_t & eid)
  {
    recalculate_enabled_by_edge (eid, get_edge_info (eid));
  }

  void recalculate_in_enabled ( const tid_t & tid
                              , const pid_t & pid
                              , const eid_t & eid
                              )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);

    recalculate_pid_in_map (pid_in_map, pid, eid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void calculate_in_enabled (const tid_t & tid)
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    adj_place_const_it pit (in_to_transition (tid));

    for (; pit.has_more(); ++pit)
      recalculate_pid_in_map (pid_in_map, *pit, pit());

    update_set_of_tid ( tid
                      , pid_in_map.size() == pit.size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_in_enabled_put_token ( const tid_t & tid
                                   , const pid_t & pid
                                   , const eid_t & eid
                                   , const token_type & token
                                   )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    vec_token_via_edge_t & vec_token_via_edge (pid_in_map[pid]);

    vec_token_via_edge.push_back(token_via_edge_t (token, eid));

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_in_enabled_del_one_token ( const tid_t & tid
                                       , const pid_t & pid
                                       , const token_type & token
                                       )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    vec_token_via_edge_t & vec_token_via_edge (pid_in_map[pid]);
    typename vec_token_via_edge_t::iterator it (vec_token_via_edge.begin());

    while (it != vec_token_via_edge.end() && it->first != token)
      ++it;

    if (it != vec_token_via_edge.end())
      vec_token_via_edge.erase (it);

    if (vec_token_via_edge.empty())
      pid_in_map.erase (pid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  void update_in_enabled_del_all_token ( const tid_t & tid
                                       , const pid_t & pid
                                       , const token_type & token
                                       )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    vec_token_via_edge_t & vec_token_via_edge (pid_in_map[pid]);
    const vec_token_via_edge_t old (vec_token_via_edge);

    vec_token_via_edge.clear();

    for ( typename vec_token_via_edge_t::const_iterator it (old.begin())
        ; it != old.end()
        ; ++it
        )
      if (it->first != token)
        vec_token_via_edge.push_back (*it);

    if (vec_token_via_edge.empty())
      pid_in_map.erase (pid);

    update_set_of_tid ( tid
                      , pid_in_map.size() == in_to_transition(tid).size()
                      , in_enabled
                      , out_enabled
                      );
  }

  bool capacity_exceeded (const pid_t & pid) const
  {
    const capacity_map_t::const_iterator cap (capacity_map.find (pid));

    return cap != capacity_map.end() && num_token (pid) >= cap->second.left();
  }

  void update_output_descr ( output_descr_t & output_descr
                           , const pid_t & pid
                           , const eid_t & eid
                           )
  {
    if (capacity_exceeded (pid))
      {
        output_descr.erase (pid);
      }
    else
      {
        output_descr[pid] = eid;
      }
  }

  void calculate_out_enabled (const tid_t & tid)
  {
    output_descr_t & output_descr (out_map[tid]);
    adj_place_const_it pit (out_of_transition (tid));

    for (; pit.has_more(); ++pit)
      update_output_descr (output_descr, *pit, pit());

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

    update_output_descr (output_descr, pid, eid);

    update_set_of_tid ( tid
                      , output_descr.size() == out_of_transition(tid).size()
                      , out_enabled
                      , in_enabled
                      );
  }

  // *********************************************************************** //

public:
  net (const std::string & _name = "noname", const pid_t & _places = 10, const tid_t & _transitions = 10)
    : name (_name)
    , pmap ("place")
    , tmap ("transition")
    , emap ("edge name")
    , connection_map ()
    , adj_pt (eid_invalid, _places, _transitions)
    , adj_tp (eid_invalid, _transitions, _places)
    , token_place_rel ()
    , enabled ()
    , enabled_choice_consume ()
    , enabled_choice_read ()
    , trans ()
    , capacity_map ()
    , in_map ()
    , out_map ()
    , in_enabled ()
    , out_enabled ()
  {};

  // numbers of elements
  size_type get_num_places (void) const { return places().size(); }
  size_type get_num_transitions (void) const { return transitions().size(); }
  size_type get_num_edges (void) const { return edges().size(); }

  // condition+transition function accessores
  const trans_t & get_trans (const tid_t & tid) const
  {
    return get_fun (trans, tid);
  }

  // get id
  const pid_t & get_place_id (const place_type & place) const
  {
    return pmap.get_id (place);
  }

  const tid_t & get_transition_id (const transition_type & transition) const
  {
    return tmap.get_id (transition);
  }

  const eid_t & get_edge_id (const edge_type & edge) const
  {
    return emap.get_id (edge);
  }

  // get element
  const place_type & get_place (const pid_t & pid) const
  {
    return pmap.get_elem (pid);
  }

  const transition_type & get_transition (const tid_t & tid) const
  {
    return tmap.get_elem (tid);
  }

  const edge_type & get_edge (const eid_t & eid) const
  {
    return emap.get_elem (eid);
  }

  // add element
  pid_t add_place (const place_type & place)
  {
    return pmap.add (place);
  }

  void set_capacity (const pid_t & pid, const capacity_t & cap)
  {
    capacity_map[pid] = detail::capacity_and_placeholder_type (cap);

    recalculate_out_enabled_by_place (pid);
  }

  void clear_capacity (const pid_t & pid)
  {
    capacity_map.erase (pid);

    recalculate_out_enabled_by_place (pid);
  }

  boost::optional<capacity_t> get_capacity (const pid_t & pid) const
  {
    const capacity_map_t::const_iterator pos (capacity_map.find (pid));

    return (pos == capacity_map.end())
      ? boost::none
      : boost::optional<capacity_t> (pos->second.capacity())
      ;
  }

  void set_transition_function (const tid_t & tid, const trans_t & f)
  {
    trans[tid] = f;
  }

  void set_transition_priority (const tid_t & tid, const prio_t & prio)
  {
    enabled.set_priority (tid, prio);
  }

  prio_t get_transition_priority (const tid_t & tid) const
  {
    return enabled.get_priority (tid);
  }

  tid_t add_transition
  ( const transition_type & transition
  , const trans_t & tf = Function::Transition::Default<token_type>()
  )
  {
    const tid_t tid (tmap.add (transition));

    trans[tid] = tf;

    calculate_in_enabled (tid);
    calculate_out_enabled (tid);

    return tid;
  }

  eid_t add_edge (const edge_type & edge, const connection_t & connection)
  {
    const eid_t eid
      ( (is_PT (connection.type))
      ? gen_add_edge<pid_t,tid_t> (edge, connection.pid, connection.tid, adj_pt)
      : gen_add_edge<tid_t,pid_t> (edge, connection.tid, connection.pid, adj_tp)
      );

    connection_map[eid] = connection;

    recalculate_enabled_by_edge (eid, connection);

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
  {
    const typename connection_map_t::const_iterator it
      (connection_map.find (eid));

    if (it == connection_map.end())
      throw exception::no_such ("connection");

    return it->second;
  }

  eid_t get_eid_out (const tid_t & tid, const pid_t & pid) const
  {
    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        if (*pit == pid)
          {
            return pit();
          }
      }

    throw exception::no_such ("specific out connection");
  }

  eid_t get_eid_in (const tid_t & tid, const pid_t & pid) const
  {
    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        if (*pit == pid)
          {
            return pit();
          }
      }

    throw exception::no_such ("specific in connection");
  }

  bool is_read_connection (const tid_t & tid, const pid_t & pid) const
  {
    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        if (*pit == pid)
          {
            return is_pt_read (get_edge_info (pit()).type);
          }
      }

    return false;
  }

  // delete elements
  const eid_t & delete_edge (const eid_t & eid)
  {
    const typename connection_map_t::iterator it (connection_map.find (eid));

    if (it == connection_map.end())
      throw exception::no_such ("connection");

    const connection_t connection (it->second);

    if (is_PT (connection.type))
      {
        adj_pt.clear_adjacent (connection.pid, connection.tid);

        in_map[connection.tid].erase (connection.pid);

        update_set_of_tid
          ( connection.tid
          ,  in_map[connection.tid].size()
          == in_to_transition(connection.tid).size()
          , in_enabled
          , out_enabled
          );
      }
    else
      {
        adj_tp.clear_adjacent (connection.tid, connection.pid);

        out_map[connection.tid].erase (connection.pid);

        update_set_of_tid
          ( connection.tid
          ,  out_map[connection.tid].size()
          == out_of_transition(connection.tid).size()
          , out_enabled
          , in_enabled
          );
      }

    connection_map.erase (it);

    emap.erase (eid);

    return eid;
  }

  const pid_t & delete_place (const pid_t & pid)
  {
    // make the token deletion visible to delete_edge
    token_place_rel.delete_right (pid);

    std::stack<eid_t> stack;

    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      {
        stack.push (tit());
      }

    for ( adj_transition_const_it tit (in_to_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      {
        stack.push (tit());
      }

    while (!stack.empty())
      {
        delete_edge (stack.top()); stack.pop();
      }

    pmap.erase (pid);

    return pid;
  }

  const tid_t & delete_transition (const tid_t & tid)
  {
    std::stack<eid_t> stack;

    for ( adj_place_const_it pit (out_of_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        stack.push (pit());
      }

    for ( adj_place_const_it pit (in_to_transition (tid))
        ; pit.has_more()
        ; ++pit
        )
      {
        stack.push (pit());
      }

    while (!stack.empty())
      {
        delete_edge (stack.top()); stack.pop();
      }

    tmap.erase (tid);

    trans.erase (tid);

    in_enabled.erase (tid);
    out_enabled.erase (tid);
    enabled.erase (tid);
    enabled.erase_priority (tid);
    in_map.erase (tid);
    out_map.erase (tid);
    enabled_choice_consume.erase (tid);
    enabled_choice_read.erase (tid);

    return tid;
  }

  // modify and replace
  // erased in case of conflict after modification
  pid_t modify_place (const pid_t & pid, const place_type & place)
  {
    const pid_t new_pid (pmap.modify (pid, place));

    recalculate_enabled_by_place (new_pid);

    return new_pid;
  }

  // kept old value in case of conflict after modification
  pid_t replace_place (const pid_t & pid, const place_type & place)
  {
    const pid_t new_pid (pmap.replace (pid, place));

    recalculate_enabled_by_place (new_pid);

    return new_pid;
  }

  tid_t modify_transition ( const tid_t & tid
                          , const transition_type & transition
                          )
  {
    return tmap.modify (tid, transition);
  }

  tid_t replace_transition ( const tid_t & tid
                           , const transition_type & transition
                           )
  {
    return tmap.replace (tid, transition);
  }

  eid_t modify_edge (const eid_t & eid, const edge_type & edge)
  {
    const eid_t new_eid (emap.modify (eid, edge));

    recalculate_enabled_by_edge (new_eid);

    return new_eid;
  }

  eid_t replace_edge (const eid_t & eid, const edge_type & edge)
  {
    const eid_t new_eid (emap.replace (eid, edge));

    recalculate_enabled_by_edge (new_eid);

    return new_eid;
  }

  // deal with tokens
  const pid_in_map_t & get_pid_in_map (const tid_t & tid) const
  {
    const typename in_map_t::const_iterator m (in_map.find (tid));

    if (m == in_map.end())
      throw exception::no_such ("transition in in_map");

    return m->second;
  }

  const output_descr_t & get_output_descr (const tid_t & tid) const
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

  void put_token (const pid_t & pid, const token_type & token)
  {
    capacity_map_t::iterator cap (capacity_map.find (pid));

    if (cap != capacity_map.end())
      {
        cap->second.put();

        if (num_token (pid) >= cap->second.left())
          {
            throw exception::capacity_exceeded ();
          }
      }

    token_place_rel.add (token, pid);

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      update_in_enabled_put_token (*t, pid, t(), token);

    recalculate_out_enabled_by_place (pid);
  }

  void put_token (const pid_t & pid)
  {
    put_token (pid, token_type());
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

  std::size_t delete_one_token (const pid_t & pid, const token_type & token)
  {
    const std::size_t ret (token_place_rel.delete_one (token, pid));

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      update_in_enabled_del_one_token (*t, pid, token);

    recalculate_out_enabled_by_place (pid);

    return ret;
  }

  std::size_t delete_all_token (const pid_t & pid, const token_type & token)
  {
    const std::size_t ret (token_place_rel.delete_all (token, pid));

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      update_in_enabled_del_all_token (*t, pid, token);

    recalculate_out_enabled_by_place (pid);

    return ret;
  }

  std::size_t delete_all_token (const pid_t & pid)
  {
    const std::size_t ret (token_place_rel.delete_right (pid));

    recalculate_enabled_by_place (pid);

    return ret;
  }

  // WORK HERE: implement more efficient?
  std::size_t replace_one_token
  (const pid_t & pid, const token_type & old_token, const token_type & new_token)
  {
    const std::size_t k (delete_one_token (pid, old_token));

    if (k > 0)
      put_token (pid, new_token);

    return k;
  }

  // WORK HERE: implement more efficient?
  std::size_t replace_all_token
  (const pid_t & pid, const token_type & old_token, const token_type & new_token)
  {
    const std::size_t k (delete_all_token (pid, old_token));

    for (std::size_t i (0); i < k; ++i)
      put_token (pid, new_token);

    return k;
  }

  // FIRE
  bool get_can_fire (const tid_t & tid) const
  {
    return enabled.elem(tid);
  }

  choices_t choices (const tid_t & tid) const
  {
    return choices_t (get_pid_in_map (tid));
  }

  struct activity_t
  {
  public:
    const tid_t tid;
    const input_t input;
    const output_descr_t output_descr;

    activity_t ( const tid_t _tid
               , const input_t _input
               , const output_descr_t _output_descr
               )
      : tid (_tid)
      , input (_input)
      , output_descr (_output_descr)
    {}
  };

  output_t run_activity (const activity_t & activity) const
  {
    return get_fun(trans, activity.tid)(activity.input, activity.output_descr);
  }

  template <typename Output>
  void inject_activity_result (Output const & output)
  {
    for ( typename Output::const_iterator out (output.begin())
        ; out != output.end()
        ; ++out
        )
      put_token (out->second, out->first);
  }

  activity_t extract_activity (const tid_t tid)
  {
    if (!get_can_fire (tid))
      throw exception::transition_not_enabled ("during call of fire");

    // before! constructing the input, as this consumes tokens
    output_descr_t output_descr (get_output_descr(tid));

    BOOST_FOREACH (const typename output_descr_t::value_type& val, output_descr)
      {
        capacity_map_t::iterator cap (capacity_map.find (val.first));

        if (cap != capacity_map.end())
          {
            cap->second.extract();

            recalculate_out_enabled_by_place (val.first);
          }
      }

    input_t input;
    const typename enabled_choice_t::iterator choice_consume
      (enabled_choice_consume.find(tid));
    const typename enabled_choice_t::iterator choice_read
      (enabled_choice_read.find(tid));

    assert (  (choice_consume != enabled_choice_consume.end())
           || (choice_read != enabled_choice_read.end())
           );

    const choice_vec_t choice_vec_consume (choice_consume->second);
    enabled_choice_consume.erase (choice_consume);
    const choice_vec_t choice_vec_read (choice_read->second);

    for ( typename choice_vec_t::const_iterator choice
            (choice_vec_consume.begin())
        ; choice != choice_vec_consume.end()
        ; ++choice
        )
      {
        const pid_t & pid (choice->first);
        const token_via_edge_t & token_via_edge (choice->second);
        const token_type & token (token_via_edge.first);
        const eid_t & eid (token_via_edge.second);

        input.push_back (token_input_t (token, place_via_edge_t(pid, eid)));

        assert (!is_pt_read (get_edge_info (eid).type));

        delete_one_token (pid, token);
      }

    for ( typename choice_vec_t::const_iterator choice
            (choice_vec_read.begin())
        ; choice != choice_vec_read.end()
        ; ++choice
        )
      {
        const pid_t & pid (choice->first);
        const token_via_edge_t & token_via_edge (choice->second);
        const token_type & token (token_via_edge.first);
        const eid_t & eid (token_via_edge.second);

        input.push_back (token_input_t (token, place_via_edge_t(pid, eid)));
      }

    return activity_t (tid, input, output_descr);
  }

  template<typename Engine>
  activity_t extract_activity_random (Engine & engine)
  {
    return extract_activity (enabled.random (engine));
  }

  tid_t fire (const tid_t & tid)
  {
    const activity_t activity (extract_activity (tid));
    const output_t output (run_activity (activity));
    inject_activity_result (output);
    return tid;
  }

  template<typename Engine>
  tid_t fire_random (Engine & engine)
  {
    return fire (enabled.random (engine));
  }
};
} // namespace petri_net

#endif // _NET_HPP
