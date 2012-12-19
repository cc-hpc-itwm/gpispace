// simple approach to store petri nets, mirko.rahn@itwm.fraunhofer.de

#ifndef _NET_HPP
#define _NET_HPP

#include <we/type/net.fwd.hpp>

#include <fhg/assert.hpp>

#include <we/container/adjacency.hpp>
#include <we/container/bijection.hpp>
#include <we/container/multirel.hpp>
#include <we/container/priostore.hpp>
#include <we/serialize/unordered_map.hpp>
#include <we/serialize/unordered_set.hpp>
#include <we/type/connection.hpp>
#include <we/type/condition.hpp>
#include <we/type/id.hpp>
#include <we/type/token.hpp>
#include <we/type/place.hpp>
#include <we/util/cross.hpp>

#include <we/type/transition.hpp>

#include <boost/function.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>

#include <vector>
#include <stack>

#include <stdexcept>

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

    class no_such : public generic
    {
    public:
      explicit no_such (const std::string & msg)
        : generic ("no_such: " + msg)
      {}
    };
  }
} // namespace petri_net

namespace petri_net
{
  typedef adjacency::const_it<place_id_type,edge_id_type> adj_place_const_it;
  typedef adjacency::const_it<transition_id_type,edge_id_type> adj_transition_const_it;

  typedef adjacency::const_it<place_id_type,connection_t> adj_c_place_const_it;
  typedef adjacency::const_it<transition_id_type,connection_t> adj_c_transition_const_it;

// WORK HERE: Performance: collect map<transition_id_type,X>, map<transition_id_type,Y> into a
// single map<transition_id_type,(X,Y)>?

// WORK HERE: Performance: The update mechanism is not optimal in all
// cases, e.g. if a token is putted to a place, re-evaluting the
// condition of already enabled transitions is not neccessary

// the net itself
class net
{

  // *********************************************************************** //
public:
  typedef we::type::transition_t transition_type;

  typedef bijection::const_it<place::type,place_id_type> place_const_it;
  typedef bijection::const_it<transition_type,transition_id_type> transition_const_it;

  typedef multirel::right_const_it<token::type, place_id_type> token_place_it;

  typedef std::pair<token::type, place_id_type> token_input_t;
  typedef std::vector<token_input_t> input_t;

  // TODO: traits should be template parameters (with default values)
  typedef Function::Condition::Traits cd_traits;

  typedef cd_traits::tokens_t tokens_t;
  typedef cd_traits::pid_in_map_t pid_in_map_t;
  typedef cd_traits::choices_t choices_t;

  typedef cross::iterator<pid_in_map_t> choice_it;

  typedef priostore::type<transition_id_type> enabled_t;

  // *********************************************************************** //
private:
  typedef boost::unordered_map<edge_id_type, connection_t> connection_map_t;
  typedef multirel::multirel<token::type,place_id_type> token_place_rel_t;

  typedef cross::Traits<pid_in_map_t>::vec_t choice_vec_t;
  typedef boost::unordered_map<transition_id_type, choice_vec_t> enabled_choice_t;
  typedef enabled_choice_t::iterator choice_iterator_t;

  typedef boost::unordered_map<transition_id_type,pid_in_map_t> in_map_t;

  typedef boost::unordered_map< transition_id_type
                              , std::size_t
                              > adjacent_transition_size_map_type;

  // *********************************************************************** //

  bijection::bijection<place::type,place_id_type> pmap; // place::type <-> internal id
  bijection::bijection<transition_type,transition_id_type> tmap; // transition_type <-> internal id

  boost::unordered_set<connection_t> _connections;

  connection_map_t connection_map;

  adjacency::table<place_id_type,transition_id_type,edge_id_type> adj_pt;
  adjacency::table<transition_id_type,place_id_type,edge_id_type> adj_tp;

  adjacency::table<place_id_type,transition_id_type,connection_t> adj_c_pt;
  adjacency::table<transition_id_type,place_id_type,connection_t> adj_c_tp;

  token_place_rel_t token_place_rel;

  enabled_t enabled;
  enabled_choice_t enabled_choice_consume;
  enabled_choice_t enabled_choice_read;

  in_map_t in_map;

  adjacent_transition_size_map_type in_to_transition_size_map;
  adjacent_transition_size_map_type out_of_transition_size_map;

  edge_id_type _edge_id;

  // *********************************************************************** //

  friend class boost::serialization::access;
  template<typename Archive>
  void serialize (Archive & ar, const unsigned int)
  {
    ar & BOOST_SERIALIZATION_NVP(pmap);
    ar & BOOST_SERIALIZATION_NVP(tmap);
    ar & BOOST_SERIALIZATION_NVP(_connections);
    ar & BOOST_SERIALIZATION_NVP(connection_map);
    ar & BOOST_SERIALIZATION_NVP(adj_pt);
    ar & BOOST_SERIALIZATION_NVP(adj_tp);
    ar & BOOST_SERIALIZATION_NVP(adj_c_pt);
    ar & BOOST_SERIALIZATION_NVP(adj_c_tp);
    ar & BOOST_SERIALIZATION_NVP(token_place_rel);
    ar & BOOST_SERIALIZATION_NVP(enabled);
    ar & BOOST_SERIALIZATION_NVP(enabled_choice_consume);
    ar & BOOST_SERIALIZATION_NVP(enabled_choice_read);
    ar & BOOST_SERIALIZATION_NVP(in_map);
 }

  // *********************************************************************** //

  std::size_t adjacent_size
  ( adjacent_transition_size_map_type& m
  , boost::function<adj_place_const_it (const transition_id_type&)> f
  , const transition_id_type& tid
  ) const
  {
    const adjacent_transition_size_map_type::const_iterator pos (m.find (tid));

    if (pos == m.end())
      {
        const std::size_t s (f (tid).size());

        m[tid] = s;

        return s;
      }

    return pos->second;
  }

  std::size_t in_to_transition_size (const transition_id_type& tid)
  {
    return adjacent_size
      ( in_to_transition_size_map
      , boost::bind ( &net::in_to_transition
                    , this
                    , _1
                    )
      , tid
      );
  }

  std::size_t out_of_transition_size (const transition_id_type& tid)
  {
    return adjacent_size
      ( out_of_transition_size_map
      , boost::bind ( &net::out_of_transition
                    , this
                    , _1
                    )
      , tid
      );
  }

  // *********************************************************************** //

  template<typename ROW, typename COL>
  edge_id_type gen_add_edge ( const ROW & r
                            , const COL & c
                            , adjacency::table<ROW,COL,edge_id_type> & m
                            , adjacency::table<ROW,COL,connection_t> & m_c
                            , const connection_t& connection
                            )
  {
    if (m.get_adjacent (r, c) != edge_id_invalid())
      {
        throw bijection::exception::already_there ("adjacency");
      }

    m.set_adjacent (r, c, _edge_id);
    m_c.set_adjacent (r, c, connection);

    in_to_transition_size_map.erase (connection.tid);
    out_of_transition_size_map.erase (connection.tid);

    return _edge_id++;
  }

  // *********************************************************************** //

  void update_set_of_tid_in (const transition_id_type & tid, const bool can_fire)
  {
    if (not can_fire)
      {
        enabled.erase (tid);
      }
    else
      {
        enabled.insert (tid);

        choices_t cs (get_pid_in_map(tid));

        // call the global condition function here, that sets the
        // cross product either to the end or to some valid choice

        if (not get_transition (tid).condition()(cs))
          {
            enabled.erase (tid);
          }
        else
          {
            enabled.insert (tid);

            enabled_choice_consume[tid].clear();
            enabled_choice_read[tid].clear();

            for ( cross::iterator<pid_in_map_t> choice (*cs)
                ; choice.has_more()
                ; ++choice
                )
              {
                if (is_read_connection (tid, choice.key()))
                  {
                    enabled_choice_read[tid].push_back (*choice);
                  }
                else
                  {
                    enabled_choice_consume[tid].push_back (*choice);
                  }
              }
          }
      }
  }

  void recalculate_pid_in_map ( pid_in_map_t & pid_in_map
                              , const place_id_type & pid
                              , const edge_id_type & eid
                              )
  {
    tokens_t& tokens (pid_in_map[pid]);

    tokens.clear();

    for (token_place_it tp (get_token (pid)); tp.has_more(); ++tp)
      {
        tokens.push_back(*tp);
      }

    if (tokens.empty())
      {
        pid_in_map.erase (pid);
      }
  }

  void recalculate_enabled_by_place (const place_id_type & pid)
  {
    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      recalculate_enabled (*t, pid, t());
  }

  void recalculate_enabled_by_edge ( const edge_id_type & eid
                                   , const connection_t & connection
                                   )
  {
    if (edge::is_PT (connection.type))
      {
        recalculate_enabled (connection.tid, connection.pid, eid);
      }
  }

  void recalculate_enabled ( const transition_id_type & tid
                           , const place_id_type & pid
                           , const edge_id_type & eid
                           )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);

    recalculate_pid_in_map (pid_in_map, pid, eid);

    update_set_of_tid_in ( tid
                         , pid_in_map.size() == in_to_transition_size(tid)
                         );
  }

  void calculate_enabled (const transition_id_type & tid)
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    adj_place_const_it pit (in_to_transition (tid));

    for (; pit.has_more(); ++pit)
      recalculate_pid_in_map (pid_in_map, *pit, pit());

    update_set_of_tid_in ( tid
                         , pid_in_map.size() == pit.size()
                         );
  }

  void update_enabled_put_token ( const transition_id_type & tid
                                , const place_id_type & pid
                                , const edge_id_type & eid
                                , const token::type & token
                                )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    tokens_t& tokens (pid_in_map[pid]);

    tokens.push_back(token);

    update_set_of_tid_in ( tid
                         , pid_in_map.size() == in_to_transition_size(tid)
                         );
  }

  void update_enabled_del_one_token ( const transition_id_type & tid
                                    , const place_id_type & pid
                                    , const token::type & token
                                    )
  {
    pid_in_map_t & pid_in_map (in_map[tid]);
    tokens_t& tokens (pid_in_map[pid]);
    tokens_t::iterator it (tokens.begin());

    while (it != tokens.end() && *it != token)
      {
        ++it;
      }

    if (it != tokens.end())
      {
        tokens.erase (it);
      }

    if (tokens.empty())
      {
        pid_in_map.erase (pid);
      }

    update_set_of_tid_in ( tid
                         , pid_in_map.size() == in_to_transition_size(tid)
                         );
  }

  // *********************************************************************** //

public:
  net (const place_id_type & _places = 10, const transition_id_type & _transitions = 10)
    : pmap ("place")
    , tmap ("transition")
    , _connections()
    , connection_map ()
    , adj_pt (edge_id_invalid(), _places, _transitions)
    , adj_tp (edge_id_invalid(), _transitions, _places)
    , adj_c_pt (connection_invalid(), _places, _transitions)
    , adj_c_tp (connection_invalid(), _transitions, _places)
    , token_place_rel ()
    , enabled ()
    , enabled_choice_consume ()
    , enabled_choice_read ()
    , in_map ()
    , in_to_transition_size_map ()
    , out_of_transition_size_map ()
    , _edge_id (0)
  {}

  // get element
  const place::type & get_place (const place_id_type & pid) const
  {
    return pmap.get_elem (pid);
  }

  const transition_type & get_transition (const transition_id_type & tid) const
  {
    return tmap.get_elem (tid);
  }

  // add element
  place_id_type add_place (const place::type & place)
  {
    return pmap.add (place);
  }

  void set_transition_priority (const transition_id_type & tid, const priority_type& prio)
  {
    enabled.set_priority (tid, prio);
  }

  priority_type get_transition_priority (const transition_id_type & tid) const
  {
    return enabled.get_priority (tid);
  }

  transition_id_type add_transition (const transition_type & transition)
  {
    const transition_id_type tid (tmap.add (transition));

    calculate_enabled (tid);

    return tid;
  }

  void add_connection (const connection_t& connection)
  {
    const edge_id_type eid
      ( (edge::is_PT (connection.type))
      ? gen_add_edge<place_id_type,transition_id_type> (connection.pid, connection.tid, adj_pt, adj_c_pt, connection)
      : gen_add_edge<transition_id_type,place_id_type> (connection.tid, connection.pid, adj_tp, adj_c_tp, connection)
      );

    connection_map[eid] = connection;

    _connections.insert (connection);

    recalculate_enabled_by_edge (eid, connection);
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

  const boost::unordered_set<connection_t>& connections() const
  {
    return _connections;
  }

  // iterate through adjacencies
  adj_place_const_it out_of_transition (const transition_id_type & tid) const
  {
    return adj_place_const_it (adj_tp.row_const_it (tid));
  }
  adj_place_const_it in_to_transition (const transition_id_type & tid) const
  {
    return adj_place_const_it (adj_pt.col_const_it (tid));
  }
  adj_transition_const_it out_of_place (const place_id_type & pid) const
  {
    return adj_transition_const_it (adj_pt.row_const_it (pid));
  }
  adj_transition_const_it in_to_place (const place_id_type & pid) const
  {
    return adj_transition_const_it (adj_tp.col_const_it (pid));
  }

  adj_c_place_const_it c_out_of_transition (const transition_id_type & tid) const
  {
    return adj_c_place_const_it (adj_c_tp.row_const_it (tid));
  }
  adj_c_place_const_it c_in_to_transition (const transition_id_type & tid) const
  {
    return adj_c_place_const_it (adj_c_pt.col_const_it (tid));
  }
  adj_c_transition_const_it c_out_of_place (const place_id_type & pid) const
  {
    return adj_c_transition_const_it (adj_c_pt.row_const_it (pid));
  }
  adj_c_transition_const_it c_in_to_place (const place_id_type & pid) const
  {
    return adj_c_transition_const_it (adj_c_tp.col_const_it (pid));
  }

  connection_t get_connection_out ( const transition_id_type& tid
                                  , const place_id_type& pid
                                  ) const
  {
    const connection_t c (adj_c_tp.get_adjacent (tid, pid));

    if (c == connection_invalid())
      {
        throw exception::no_such ("specific out connection");
      }

    return c;
  }
  connection_t get_connection_in ( const transition_id_type& tid
                                 , const place_id_type& pid
                                 ) const
  {
    const connection_t c (adj_c_pt.get_adjacent (pid, tid));

    if (c == connection_invalid())
      {
        throw exception::no_such ("specific in connection");
      }

    return c;
  }

private:
  edge_id_type get_eid_out ( const transition_id_type& tid
                           , const place_id_type& pid
                           ) const
  {
    const edge_id_type eid (adj_tp.get_adjacent (tid, pid));

    if (eid == edge_id_invalid())
      {
        throw exception::no_such ("specific out connection");
      }

    return eid;
  }
  edge_id_type get_eid_in (const transition_id_type & tid, const place_id_type & pid) const
  {
    const edge_id_type eid (adj_pt.get_adjacent (pid, tid));

    if (eid == edge_id_invalid())
      {
        throw exception::no_such ("specific in connection");
      }

    return eid;
  }

public:
  bool is_read_connection ( const transition_id_type & tid
                          , const place_id_type & pid
                          ) const
  {
    return _connections.find (connection_t (edge::PT_READ, tid, pid))
      != _connections.end()
      ;
  }

  // delete elements
private:
  const edge_id_type & delete_edge (const edge_id_type & eid)
  {
    const connection_map_t::iterator it (connection_map.find (eid));

    if (it == connection_map.end())
      throw exception::no_such ("connection");

    const connection_t connection (it->second);

    if (edge::is_PT (connection.type))
      {
        adj_pt.clear_adjacent (connection.pid, connection.tid);
        adj_c_pt.clear_adjacent (connection.pid, connection.tid);
        in_to_transition_size_map.erase (connection.tid);
        out_of_transition_size_map.erase (connection.tid);

        in_map[connection.tid].erase (connection.pid);

        update_set_of_tid_in
          ( connection.tid
          ,  in_map[connection.tid].size()
          == in_to_transition_size(connection.tid)
          );
      }
    else
      {
        adj_tp.clear_adjacent (connection.tid, connection.pid);
        adj_c_tp.clear_adjacent (connection.tid, connection.pid);
        in_to_transition_size_map.erase (connection.tid);
        out_of_transition_size_map.erase (connection.tid);
      }

    connection_map.erase (it);

    _connections.erase (connection);

    return eid;
  }
public:
  const edge_id_type& delete_edge_out ( const transition_id_type& tid
                                      , const place_id_type& pid
                                      )
  {
    return delete_edge (get_eid_out (tid, pid));
  }
  const edge_id_type& delete_edge_in ( const transition_id_type& tid
                                     , const place_id_type& pid
                                     )
  {
    return delete_edge (get_eid_in (tid, pid));
  }

  const place_id_type & delete_place (const place_id_type & pid)
  {
    // make the token deletion visible to delete_edge
    token_place_rel.delete_right (pid);

    std::stack<edge_id_type> stack;

    for ( adj_transition_const_it tit (out_of_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      {
        stack.push (tit());
	// TODO: get port and remove place from there
      }

    for ( adj_transition_const_it tit (in_to_place (pid))
        ; tit.has_more()
        ; ++tit
        )
      {
        stack.push (tit());
	// TODO: get port and remove place from there
	// transition_t::port_id_t portId = transition->transition().input_port_by_pid(place_.id()).first;
      }

    while (!stack.empty())
      {
        delete_edge (stack.top()); stack.pop();
      }


    pmap.erase (pid);

    return pid;
  }

  const transition_id_type & delete_transition (const transition_id_type & tid)
  {
    std::stack<edge_id_type> stack;

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

    enabled.erase (tid);
    enabled.erase_priority (tid);
    in_map.erase (tid);
    enabled_choice_consume.erase (tid);
    enabled_choice_read.erase (tid);
    in_to_transition_size_map.erase (tid);
    out_of_transition_size_map.erase (tid);

    return tid;
  }

  // erased in case of conflict after modification
  place_id_type modify_place (const place_id_type & pid, const place::type & place)
  {
    const place_id_type new_pid (pmap.modify (pid, place));

    recalculate_enabled_by_place (new_pid);

    return new_pid;
  }

  transition_id_type modify_transition ( const transition_id_type & tid
                          , const transition_type & transition
                          )
  {
    return tmap.modify (tid, transition);
  }

 private:
  // deal with tokens
  const pid_in_map_t & get_pid_in_map (const transition_id_type & tid) const
  {
    const in_map_t::const_iterator m (in_map.find (tid));

    if (m == in_map.end())
      throw exception::no_such ("transition in in_map");

    return m->second;
  }

  const transition_id_type& enabled_first (void) const
  {
    return enabled.first();
  }

  template<typename Engine>
  const transition_id_type& enabled_random (Engine& engine) const
  {
    return enabled.random (engine);
  }

public:
  void put_token (const place_id_type & pid, const token::type & token)
  {
    token_place_rel.add (token, pid);

    for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
      update_enabled_put_token (*t, pid, t(), token);
  }

  void put_token (const place_id_type & pid)
  {
    put_token (pid, token::type());
  }

  token_place_it get_token (const place_id_type & pid) const
  {
    return token_place_rel.left_of (pid);
  }

  bool has_token (const place_id_type & pid) const
  {
    return token_place_rel.contains_right (pid);
  }

  std::size_t delete_all_token (const place_id_type & pid)
  {
    const std::size_t ret (token_place_rel.delete_right (pid));

    recalculate_enabled_by_place (pid);

    return ret;
  }

  // FIRE
  bool can_fire () const
  {
    return not enabled.empty();
  }

  struct activity_t
  {
  public:
    const transition_id_type tid;
    const input_t input;

    activity_t ( const transition_id_type _tid
               , const input_t& _input
               )
      : tid (_tid)
      , input (_input)
    {}
  };

private:
  activity_t extract_activity (const transition_id_type tid)
  {
    input_t input;

    const choice_iterator_t choice_consume (enabled_choice_consume.find(tid));
    const choice_iterator_t choice_read (enabled_choice_read.find(tid));

    assert (  (choice_consume != enabled_choice_consume.end())
           || (choice_read != enabled_choice_read.end())
           );

    const choice_vec_t choice_vec_consume (choice_consume->second);
    enabled_choice_consume.erase (choice_consume);
    const choice_vec_t choice_vec_read (choice_read->second);

    for ( choice_vec_t::const_iterator choice
            (choice_vec_consume.begin())
        ; choice != choice_vec_consume.end()
        ; ++choice
        )
      {
        const place_id_type& pid (choice->first);
        const token::type& token (choice->second);

        input.push_back (token_input_t (token, pid));

        assert (not is_read_connection (tid, pid));

        // delete_one_token (pid, token)
        for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
          {
            update_enabled_del_one_token (*t, pid, token);
          }
      }

    for ( choice_vec_t::const_iterator choice
            (choice_vec_read.begin())
        ; choice != choice_vec_read.end()
        ; ++choice
        )
      {
        const place_id_type& pid (choice->first);
        const token::type& token (choice->second);

        assert (is_read_connection (tid, pid));

        input.push_back (token_input_t (token, pid));
      }

    return activity_t (tid, input);
  }

public:
  template<typename Engine>
  activity_t extract_activity_random (Engine & engine)
  {
    return extract_activity (enabled.random (engine));
  }
};

    inline std::ostream & operator << ( std::ostream & s
                                      , const net & n
                                      )
    {
      typedef net pnet_t;

      for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
      {
        s << "[" << n.get_place (*p) << ":";

        typedef boost::unordered_map<token::type, size_t> token_cnt_t;
        token_cnt_t token;
        for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        {
          token[*tp]++;
        }

        for (token_cnt_t::const_iterator t (token.begin()); t != token.end(); ++t)
        {
          if (t->second > 1)
          {
            s << " " << t->second << "x " << t->first;
          }
          else
          {
            s << " " << t->first;
          }
        }
        s << "]";
      }

      for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
      {
        s << "/";
        s << n.get_transition (*t);
        s << "/";
      }

      return s;
    }
} // namespace petri_net

#endif // _NET_HPP
