// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_NET_HPP
#define _WE_TYPE_NET_HPP

#include <we/type/net.fwd.hpp>

#include <fhg/assert.hpp>

#include <we/container/adjacency.hpp>
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

#include <boost/range/adaptor/map.hpp>

#include <vector>
#include <stack>
#include <list>

#include <iosfwd>

namespace petri_net
{
  class net
  {
  public:
    typedef we::type::transition_t transition_type;

    typedef boost::unordered_map<place_id_type,place::type> pmap_type;

    typedef boost::unordered_map<transition_id_type,transition_type> tmap_type;

    typedef std::vector<token::type> tokens_type;
    typedef boost::unordered_map<place_id_type, tokens_type> token_place_rel_t;

    typedef std::pair<token::type, place_id_type> token_input_t;
    typedef std::vector<token_input_t> input_t;

    typedef boost::unordered_map< petri_net::place_id_type
                                , tokens_type
                                > tokens_by_place_id_t;

    typedef priostore::type<transition_id_type> enabled_t;

  private:
    typedef cross::Traits<tokens_by_place_id_t>::vec_t choice_vec_t;
    typedef boost::unordered_map<transition_id_type, choice_vec_t> enabled_choice_t;
    typedef enabled_choice_t::iterator choice_iterator_t;

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
    }

    // ********************************************************************* //

    void update_enabled (const transition_id_type& tid)
    {
      tokens_by_place_id_t tokens_by_place_id;

      BOOST_FOREACH ( const place_id_type& place_id
                    , in_to_transition (tid) | boost::adaptors::map_keys
                    )
      {
        if (_token_place_rel[place_id].size() > 0)
        {
          tokens_by_place_id[place_id] = _token_place_rel[place_id];
        }
      }

      if (tokens_by_place_id.size() != in_to_transition (tid).size())
        {
          _enabled.erase (tid);
        }
      else
        {
          cross::cross<tokens_by_place_id_t> cs (tokens_by_place_id);

          if (not get_transition (tid).condition()(cs))
            {
              _enabled.erase (tid);
            }
          else
            {
              _enabled.insert (tid);

              _enabled_choice_consume[tid].clear();
              _enabled_choice_read[tid].clear();

              for ( cross::iterator<tokens_by_place_id_t> choice (*cs)
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

      update_enabled (tid);

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

      if (edge::is_PT (connection.type))
        {
          update_enabled (connection.tid);
        }
    }

    const boost::unordered_map<place_id_type,place::type>& places() const
    {
      return _pmap;
    }

    const boost::unordered_map<transition_id_type,transition_type>&
    transitions () const
    {
      return _tmap;
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

    const boost::unordered_map<place_id_type, connection_t>&
    out_of_transition (const transition_id_type& tid) const
    {
      return _adj_tp.col_adj_tab (tid);
    }
    const boost::unordered_map<place_id_type, connection_t>&
    in_to_transition (const transition_id_type& tid) const
    {
      return _adj_pt.row_adj_tab (tid);
    }
    const boost::unordered_map<transition_id_type, connection_t>&
    out_of_place (const place_id_type& pid) const
    {
      return _adj_pt.col_adj_tab (pid);
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
    }
    void delete_edge_in ( const transition_id_type& tid
                        , const place_id_type& pid
                        )
    {
      _adj_pt.clear_adjacent (pid, tid);

      update_enabled (tid);
    }

    void delete_place (const place_id_type& pid)
    {
      // make the token deletion visible to delete_connection
      _token_place_rel.erase (pid);

      std::stack<std::pair<transition_id_type, place_id_type> > stack_out;
      std::stack<std::pair<transition_id_type, place_id_type> > stack_in;

      BOOST_FOREACH ( const transition_id_type& tid
                    , out_of_place (pid) | boost::adaptors::map_keys
                    )
        {
          stack_in.push (std::make_pair (tid, pid));
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

      BOOST_FOREACH ( const place_id_type& place_id
                    , out_of_transition (tid) | boost::adaptors::map_keys
                    )
      {
        stack_out.push (std::make_pair (tid, place_id));
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
      _enabled_choice_consume.erase (tid);
      _enabled_choice_read.erase (tid);
    }

    place_id_type modify_place ( const place_id_type& pid
                               , const place::type& place
                               )
    {
      _pmap[pid] = place;

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

      BOOST_FOREACH ( const transition_id_type& tid
                    , out_of_place (pid) | boost::adaptors::map_keys
                    )
        {
          update_enabled (tid);
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

      BOOST_FOREACH ( const transition_id_type& tid
                    , out_of_place (pid) | boost::adaptors::map_keys
                    )
      {
        update_enabled (tid);
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
    class token_eq
    {
    private:
      const token::type& _token;
    public:
      token_eq (const token::type& token) : _token (token) {}
      bool operator() (const token::type& other) { return other == _token; }
    };

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

          tokens_type& tokens (_token_place_rel[pid]);
          tokens.erase (find_if ( tokens.begin()
                                , tokens.end()
                                , token_eq (token)
                                )
                       );

          BOOST_FOREACH ( const transition_id_type& t
                        , out_of_place (pid) | boost::adaptors::map_keys
                        )
            {
              update_enabled (t);
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
