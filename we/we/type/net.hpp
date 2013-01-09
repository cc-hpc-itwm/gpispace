// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_NET_HPP
#define _WE_TYPE_NET_HPP

#include <we/type/net.fwd.hpp>

#include <fhg/assert.hpp>

#include <we/container/adjacency.hpp>
#include <we/container/priostore.hpp>
#include <we/container/it.hpp>
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

#include <boost/range/adaptor/map.hpp>

#include <vector>
#include <stack>
#include <list>

#include <iosfwd>

namespace petri_net
{
  typedef adjacency::const_it<place_id_type,connection_t> adj_place_const_it;
  typedef adjacency::const_it<transition_id_type,connection_t> adj_transition_const_it;

// WORK HERE: Performance: collect map<transition_id_type,X>, map<transition_id_type,Y> into a
// single map<transition_id_type,(X,Y)>?

// WORK HERE: Performance: The update mechanism is not optimal in all
// cases, e.g. if a token is putted to a place, re-evaluting the
// condition of already enabled transitions is not neccessary

  class net
  {
  public:
    typedef we::type::transition_t transition_type;

    typedef boost::unordered_map<place_id_type,place::type> pmap_type;
    typedef we::container::map_const_it<pmap_type> place_const_it;

    typedef boost::unordered_map<transition_id_type,transition_type> tmap_type;
    typedef we::container::map_const_it<tmap_type> transition_const_it;

    typedef std::vector<token::type> tokens_type;
    typedef boost::unordered_map<place_id_type, tokens_type> token_place_rel_t;

    typedef std::pair<token::type, place_id_type> token_input_t;
    typedef std::vector<token_input_t> input_t;

    //! \todo traits should be template parameters (with default values)
    typedef Function::Condition::Traits cd_traits;

    typedef cd_traits::pid_in_map_t pid_in_map_t;
    typedef cd_traits::choices_t choices_t;

    typedef priostore::type<transition_id_type> enabled_t;

  private:
    typedef cross::Traits<pid_in_map_t>::vec_t choice_vec_t;
    typedef boost::unordered_map<transition_id_type, choice_vec_t> enabled_choice_t;
    typedef enabled_choice_t::iterator choice_iterator_t;

    typedef boost::unordered_map<transition_id_type,pid_in_map_t> in_map_t;

    typedef boost::unordered_map< transition_id_type
                                , std::size_t
                                > adjacent_transition_size_map_type;

    // ********************************************************************* //

    place_id_type _place_id;
    pmap_type _pmap;

    transition_id_type _transition_id;
    tmap_type _tmap;

    adjacency::table<place_id_type,transition_id_type,connection_t> _adj_pt;
    adjacency::table<transition_id_type,place_id_type,connection_t> _adj_tp;

    token_place_rel_t _token_place_rel;

    enabled_t _enabled;
    enabled_choice_t _enabled_choice_consume;
    enabled_choice_t _enabled_choice_read;

    in_map_t _in_map;

    adjacent_transition_size_map_type _in_to_transition_size_map;
    adjacent_transition_size_map_type _out_of_transition_size_map;

    // ********************************************************************* //

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_place_id);
      ar & BOOST_SERIALIZATION_NVP(_pmap);
      ar & BOOST_SERIALIZATION_NVP(_transition_id);
      ar & BOOST_SERIALIZATION_NVP(_tmap);
      ar & BOOST_SERIALIZATION_NVP(_adj_pt);
      ar & BOOST_SERIALIZATION_NVP(_adj_tp);
      ar & BOOST_SERIALIZATION_NVP(_token_place_rel);
      ar & BOOST_SERIALIZATION_NVP(_enabled);
      ar & BOOST_SERIALIZATION_NVP(_enabled_choice_consume);
      ar & BOOST_SERIALIZATION_NVP(_enabled_choice_read);
      ar & BOOST_SERIALIZATION_NVP(_in_map);
    }

    // ********************************************************************* //

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
      const adjacent_transition_size_map_type::const_iterator
        pos (_in_to_transition_size_map.find (tid));

      if (pos == _in_to_transition_size_map.end())
        {
          return _in_to_transition_size_map[tid] = in_to_transition (tid).size();
        }

      return pos->second;
    }

    std::size_t out_of_transition_size (const transition_id_type& tid)
    {
      return adjacent_size
        ( _out_of_transition_size_map
        , boost::bind (&net::out_of_transition, this, _1)
        , tid
        );
    }

    // ********************************************************************* //

    void update_set_of_tid_in ( const transition_id_type& tid
                              , const pid_in_map_t& pid_in_map
                              )
    {
      if (pid_in_map.size() != in_to_transition_size (tid))
        {
          _enabled.erase (tid);
        }
      else
        {
          _enabled.insert (tid);

          choices_t cs (_in_map.at (tid));

          // call the global condition function here, that sets the
          // cross product either to the end or to some valid choice

          if (not get_transition (tid).condition()(cs))
            {
              _enabled.erase (tid);
            }
          else
            {
              _enabled.insert (tid);

              _enabled_choice_consume[tid].clear();
              _enabled_choice_read[tid].clear();

              for ( cross::iterator<pid_in_map_t> choice (*cs)
                  ; choice.has_more()
                  ; ++choice
                  )
                {
                  if (is_read_connection (tid, choice.key()))
                    {
                      _enabled_choice_read[tid].push_back (*choice);
                    }
                  else
                    {
                      _enabled_choice_consume[tid].push_back (*choice);
                    }
                }
            }
        }
    }

    void recalculate_enabled ( const transition_id_type& tid
                             , const place_id_type& pid
                             )
    {
      pid_in_map_t& pid_in_map (_in_map[tid]);

      if (has_token (pid))
        {
          pid_in_map[pid] = get_token (pid);
        }
      else
        {
          pid_in_map.erase (pid);
        }

      update_set_of_tid_in (tid, pid_in_map);
    }

    // ********************************************************************* //

  public:
    const place::type& get_place (const place_id_type& pid) const
    {
      const pmap_type::const_iterator pos (_pmap.find (pid));

      if (pos == _pmap.end())
        {
          throw we::container::exception::no_such ("get_place");
        }

      return pos->second;
    }

    const transition_type& get_transition (const transition_id_type& tid) const
    {
      const tmap_type::const_iterator pos (_tmap.find (tid));

      if (pos == _tmap.end())
        {
          throw we::container::exception::no_such ("get_transition");
        }

      return pos->second;
    }

    place_id_type add_place (const place::type& place)
    {
      const place_id_type pid (_place_id++);

      _pmap.insert (pmap_type::value_type (pid, place));

      return pid;
    }

    void set_transition_priority (const transition_id_type& tid, const priority_type& prio)
    {
      _enabled.set_priority (tid, prio);
    }

    priority_type get_transition_priority (const transition_id_type& tid) const
    {
      return _enabled.get_priority (tid);
    }

    transition_id_type add_transition (const transition_type& transition)
    {
      const transition_id_type tid (_transition_id++);

      _tmap.insert (tmap_type::value_type (tid, transition));

      pid_in_map_t& pid_in_map (_in_map[tid]);

      BOOST_FOREACH ( const place_id_type& place_id
                    , in_to_transition (tid) | boost::adaptors::map_keys
                    )
        {
          if (has_token (place_id))
            {
              pid_in_map.insert
                (pid_in_map_t::value_type (place_id, get_token (place_id)));
            }
        }

      update_set_of_tid_in (tid, pid_in_map);

      return tid;
    }

    void add_connection (const connection_t& connection)
    {
      if (edge::is_PT (connection.type))
        {
          _adj_pt.set_adjacent ( connection.pid
                               , connection.tid
                               , connection
                               , "add_connection"
                               );
        }
      else
        {
          _adj_tp.set_adjacent ( connection.tid
                               , connection.pid
                               , connection
                               , "add_connection"
                               );
        }

      _in_to_transition_size_map.erase (connection.tid);
      _out_of_transition_size_map.erase (connection.tid);

      if (edge::is_PT (connection.type))
        {
          recalculate_enabled (connection.tid, connection.pid);
        }
    }

    place_const_it places() const
    {
      return place_const_it (_pmap);
    }

    transition_const_it transitions (void) const
    {
      return transition_const_it (_tmap);
    }

    //! \todo Implement more efficient if necessary
    const boost::unordered_set<connection_t> connections() const
    {
      boost::unordered_set<connection_t> s;

      BOOST_FOREACH (const connection_t& connection, _adj_tp.adjacencies())
        {
          s.insert (connection);
        }
      BOOST_FOREACH (const connection_t& connection, _adj_pt.adjacencies())
        {
          s.insert (connection);
        }

      return s;
    }

    adj_place_const_it out_of_transition (const transition_id_type& tid) const
    {
      return adj_place_const_it (_adj_tp.row_const_it (tid));
    }
    const boost::unordered_map<place_id_type, connection_t>&
    in_to_transition (const transition_id_type& tid) const
    {
      return _adj_pt.row_adj_tab (tid);
    }

    adj_transition_const_it out_of_place (const place_id_type& pid) const
    {
      return adj_transition_const_it (_adj_pt.row_const_it (pid));
    }
    const boost::unordered_map<transition_id_type, connection_t>&
    in_to_place (const place_id_type& pid) const
    {
      return _adj_tp.row_adj_tab (pid);
    }

    connection_t get_connection_out ( const transition_id_type& tid
                                    , const place_id_type& pid
                                    ) const
    {
      return _adj_tp.get_adjacent (tid, pid, "get_connection_out");
    }
    connection_t get_connection_in ( const transition_id_type& tid
                                   , const place_id_type& pid
                                   ) const
    {
      return _adj_pt.get_adjacent (pid, tid, "get_connection_in");
    }

    bool is_read_connection ( const transition_id_type& tid
                            , const place_id_type& pid
                            ) const
    {
      return edge::is_pt_read
        (_adj_pt.get_adjacent (pid, tid, "is_read_connection").type);
    }

    void delete_edge_out ( const transition_id_type& tid
                         , const place_id_type& pid
                         )
    {
      _adj_tp.clear_adjacent (tid, pid);
      _in_to_transition_size_map.erase (tid);
      _out_of_transition_size_map.erase (tid);
    }
    void delete_edge_in ( const transition_id_type& tid
                        , const place_id_type& pid
                        )
    {
      _adj_pt.clear_adjacent (pid, tid);
      _in_to_transition_size_map.erase (tid);
      _out_of_transition_size_map.erase (tid);

      pid_in_map_t& pid_in_map (_in_map[tid]);

      pid_in_map.erase (pid);

      update_set_of_tid_in (tid, pid_in_map);
    }

    void delete_place (const place_id_type& pid)
    {
      // make the token deletion visible to delete_connection
      _token_place_rel.erase (pid);

      std::stack<std::pair<transition_id_type, place_id_type> > stack_out;
      std::stack<std::pair<transition_id_type, place_id_type> > stack_in;

      for ( adj_transition_const_it tit (out_of_place (pid))
          ; tit.has_more()
          ; ++tit
          )
        {
          stack_in.push (std::make_pair (*tit, pid));
          // TODO: get port and remove place from there
        }

      BOOST_FOREACH ( const transition_id_type& transition_id
                    , in_to_place (pid) | boost::adaptors::map_keys
                    )
        {
          stack_out.push (std::make_pair (transition_id, pid));
          // TODO: get port and remove place from there
          // transition_t::port_id_t portId = transition->transition().input_port_by_pid(place_.id()).first;
        }

      while (!stack_out.empty())
        {
          delete_edge_out (stack_out.top().first, stack_out.top().second);
          stack_out.pop();
        }
      while (!stack_in.empty())
        {
          delete_edge_in (stack_in.top().first, stack_in.top().second);
          stack_in.pop();
        }

      _pmap.erase (pid);
    }

    void delete_transition (const transition_id_type& tid)
    {
      std::stack<std::pair<transition_id_type, place_id_type> > stack_out;
      std::stack<std::pair<transition_id_type, place_id_type> > stack_in;

      for ( adj_place_const_it pit (out_of_transition (tid))
          ; pit.has_more()
          ; ++pit
          )
        {
          stack_out.push (std::make_pair (tid, *pit));
        }

      BOOST_FOREACH ( const place_id_type& place_id
                    , in_to_transition (tid) | boost::adaptors::map_keys
                    )
        {
          stack_in.push (std::make_pair (tid, place_id));
        }

      while (!stack_out.empty())
        {
          delete_edge_out (stack_out.top().first, stack_out.top().second);
          stack_out.pop();
        }
      while (!stack_in.empty())
        {
          delete_edge_in (stack_in.top().first, stack_in.top().second);
          stack_in.pop();
        }

      _tmap.erase (tid);

      _enabled.erase (tid);
      _enabled.erase_priority (tid);
      _in_map.erase (tid);
      _enabled_choice_consume.erase (tid);
      _enabled_choice_read.erase (tid);
      _in_to_transition_size_map.erase (tid);
      _out_of_transition_size_map.erase (tid);
    }

    // erased in case of conflict after modification
    place_id_type modify_place ( const place_id_type& pid
                               , const place::type& place
                               )
    {
      _pmap[pid] = place;

      for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
        {
          recalculate_enabled (*t, pid);
        }

      return pid;
    }

    transition_id_type modify_transition ( const transition_id_type& tid
                                         , const transition_type& transition
                                         )
    {
      _tmap[tid] = transition;

      return tid;
    }

    void put_token (const place_id_type& pid, const token::type& token)
    {
      _token_place_rel[pid].push_back (token);

      for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
        {
          pid_in_map_t& pid_in_map (_in_map[*t]);

          pid_in_map[pid].push_back (token);

          update_set_of_tid_in (*t, pid_in_map);
        }
    }

    void put_token (const place_id_type& pid)
    {
      put_token (pid, token::type());
    }

    const tokens_type& get_token (const place_id_type&) const;

    bool has_token (const place_id_type& pid) const
    {
      return _token_place_rel.find (pid) != _token_place_rel.end();
    }

    void delete_all_token (const place_id_type& pid)
    {
      _token_place_rel.erase (pid);

      for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
        {
          recalculate_enabled (*t, pid);
        }
    }

    bool can_fire() const
    {
      return not _enabled.empty();
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

      const choice_iterator_t choice_consume (_enabled_choice_consume.find(tid));
      const choice_iterator_t choice_read (_enabled_choice_read.find(tid));

      assert (  (choice_consume != _enabled_choice_consume.end())
             || (choice_read != _enabled_choice_read.end())
             );

      const choice_vec_t choice_vec_consume (choice_consume->second);
      _enabled_choice_consume.erase (choice_consume);
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

          for (adj_transition_const_it t (out_of_place (pid)); t.has_more(); ++t)
            {
              pid_in_map_t& pid_in_map (_in_map[*t]);
              tokens_type& tokens (pid_in_map[pid]);
              tokens_type::iterator it (tokens.begin());

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

              update_set_of_tid_in (*t, pid_in_map);
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
    activity_t extract_activity_random (Engine& engine)
    {
      return extract_activity (_enabled.random (engine));
    }
  };

  std::ostream& operator<< (std::ostream&, const net&);
}

#endif
