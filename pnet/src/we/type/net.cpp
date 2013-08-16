// mirko.rahn@itwm.fraunhofer.de

#include <we/type/net.hpp>
#include <we/type/condition.hpp>
#include <we/util/cross.hpp>

#include <we/require_type.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <stack>

#include <fhg/util/stat.hpp>

namespace petri_net
{
  net::net()
    : _place_id()
    , _pmap()
    , _transition_id()
    , _tmap()
    , _adj_pt()
    , _adj_tp()
    , _token_by_place_id()
    , _enabled()
    , _enabled_choice()
  {}
  net::net (const net& other)
    : _place_id (other._place_id)
    , _pmap (other._pmap)
    , _transition_id (other._transition_id)
    , _tmap (other._tmap)
    , _adj_pt (other._adj_pt)
    , _adj_tp (other._adj_tp)
    , _token_by_place_id (other._token_by_place_id)
    , _enabled (other._enabled)
    , _enabled_choice()
  {
    get_enabled_choice (other);
  }
  net& net::operator= (const net& other)
  {
    _place_id = other._place_id;
    _pmap = other._pmap;
    _transition_id = other._transition_id;
    _tmap = other._tmap;
    _adj_pt = other._adj_pt;
    _adj_tp = other._adj_tp;
    _token_by_place_id = other._token_by_place_id;
    _enabled = other._enabled;

    get_enabled_choice (other);

    return *this;
  }
  void net::get_enabled_choice (const net& other)
  {
    typedef boost::unordered_map< petri_net::place_id_type
                                , we::util::pos_and_distance_type
                                > enabled_by_place_id_type;

    typedef std::pair<transition_id_type, enabled_by_place_id_type> tm_type;

    BOOST_FOREACH (const tm_type& tm, other._enabled_choice)
    {
      enabled_by_place_id_type& enabled_by_place_id (_enabled_choice[tm.first]);

      BOOST_FOREACH (const enabled_by_place_id_type::value_type& pd, tm.second)
      {
        enabled_by_place_id[pd.first] = std::make_pair
          ( boost::next ( _token_by_place_id.at (pd.first).begin()
                        , pd.second.second
                        )
          , pd.second.second
          );
      }
    }
  }

  place_id_type net::add_place (const place::type& place)
  {
    const place_id_type pid (_place_id++);

    _pmap.insert (std::make_pair (pid, place));

    return pid;
  }

  transition_id_type
  net::add_transition (const we::type::transition_t& transition)
  {
    const transition_id_type tid (_transition_id++);

    _tmap.insert (std::make_pair (tid, transition));

    return tid;
  }

  void net::add_connection (const connection_t& connection)
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

  void net::set_transition_priority ( const transition_id_type& tid
                                    , const priority_type& prio
                                    )
  {
    _enabled.set_priority (tid, prio);
  }

  priority_type
  net::get_transition_priority (const transition_id_type& tid) const
  {
    return _enabled.get_priority (tid);
  }

  namespace
  {
    template<class MAP>
    const typename MAP::mapped_type& get ( const MAP& m
                                         , const typename MAP::key_type& key
                                         , const std::string& msg
                                         )
    {
      const typename MAP::const_iterator pos (m.find (key));

      if (pos == m.end())
        {
          throw we::container::exception::no_such (msg);
        }

      return pos->second;
    }
  }

  const place::type& net::get_place (const place_id_type& pid) const
  {
    return get (_pmap, pid, "get_place");
  }

  const we::type::transition_t&
  net::get_transition (const transition_id_type& tid) const
  {
    return get (_tmap, tid, "get_transition");
  }

  const boost::unordered_map<place_id_type,place::type>& net::places() const
  {
    return _pmap;
  }

  const boost::unordered_map<transition_id_type,we::type::transition_t>&
  net::transitions() const
  {
    return _tmap;
  }

  void net::modify_transitions
    ( const boost::function<void ( const transition_id_type&
                                 , we::type::transition_t&
                                 )>& f
    )
  {
    typedef std::pair<const transition_id_type, we::type::transition_t> it_type;
    BOOST_FOREACH (it_type& it, _tmap)
    {
      f (it.first, it.second);
    }
  }

  //! \todo Implement more efficient if necessary
  const boost::unordered_set<connection_t> net::connections() const
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
  net::out_of_transition (const transition_id_type& tid) const
  {
    return _adj_tp.col_adj_tab (tid);
  }
  const boost::unordered_map<place_id_type, connection_t>&
  net::in_to_transition (const transition_id_type& tid) const
  {
    return _adj_pt.row_adj_tab (tid);
  }
  const boost::unordered_map<transition_id_type, connection_t>&
  net::out_of_place (const place_id_type& pid) const
  {
    return _adj_pt.col_adj_tab (pid);
  }
  const boost::unordered_map<transition_id_type, connection_t>&
  net::in_to_place (const place_id_type& pid) const
  {
    return _adj_tp.row_adj_tab (pid);
  }

  connection_t net::get_connection_out ( const transition_id_type& tid
                                       , const place_id_type& pid
                                       ) const
  {
    return _adj_tp.get_adjacent (tid, pid, "get_connection_out");
  }
  connection_t net::get_connection_in ( const transition_id_type& tid
                                      , const place_id_type& pid
                                      ) const
  {
    return _adj_pt.get_adjacent (pid, tid, "get_connection_in");
  }
  bool net::is_read_connection ( const transition_id_type& tid
                               , const place_id_type& pid
                               ) const
  {
    return edge::is_pt_read
      (_adj_pt.get_adjacent (pid, tid, "is_read_connection").type);
  }

  void net::delete_edge_out ( const transition_id_type& tid
                            , const place_id_type& pid
                            )
  {
    _adj_tp.clear_adjacent (tid, pid);
  }
  void net::delete_edge_in ( const transition_id_type& tid
                           , const place_id_type& pid
                           )
  {
    _adj_pt.clear_adjacent (pid, tid);

    update_enabled (tid);
  }

  void net::delete_place (const place_id_type& pid)
  {
    // make the token deletion visible to delete_connection
    _token_by_place_id.erase (pid);

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

  void net::delete_transition (const transition_id_type& tid)
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
    _enabled_choice.erase (tid);
  }

  place_id_type net::modify_place ( const place_id_type& pid
                                  , const place::type& place
                                  )
  {
    _pmap[pid] = place;

    return pid;
  }

  transition_id_type
  net::modify_transition ( const transition_id_type& tid
                         , const we::type::transition_t& transition
                         )
  {
    _tmap[tid] = transition;

    return tid;
  }

  void net::put_token ( const place_id_type& pid
                      , const pnet::type::value::value_type& token
                      )
  {
    std::list<pnet::type::value::value_type>& tokens (_token_by_place_id[pid]);
    const std::list<pnet::type::value::value_type>::iterator
      tokenpos (tokens.insert (tokens.end(), token));

    BOOST_FOREACH ( const transition_id_type& tid
                  , out_of_place (pid) | boost::adaptors::map_keys
                  )
    {
      update_enabled_put_token (tid, pid, tokenpos);
    }
  }

  void net::put_value ( const place_id_type& pid
                      , const pnet::type::value::value_type& value
                      )
  {
    const place::type& place (get_place (pid));

    put_token (pid, pnet::require_type
                      ( value
                      , place.signature()
                      , place.name()
                      )
              );
  }

  namespace
  {
    const std::list<pnet::type::value::value_type>& no_tokens()
    {
      static const std::list<pnet::type::value::value_type> x;

      return x;
    }
  }

  const std::list<pnet::type::value::value_type>&
  net::get_token (const place_id_type& pid) const
  {
    typedef boost::unordered_map< place_id_type
                                , std::list<pnet::type::value::value_type>
                                > token_by_place_id_t;

    token_by_place_id_t::const_iterator pos (_token_by_place_id.find (pid));

    return (pos != _token_by_place_id.end()) ? pos->second : no_tokens();
  }

  void net::delete_all_token (const place_id_type& pid)
  {
    _token_by_place_id.erase (pid);

    BOOST_FOREACH ( const transition_id_type& tid
                  , out_of_place (pid) | boost::adaptors::map_keys
                  )
    {
      disable (tid);
    }
  }

  bool net::can_fire() const
  {
    return not _enabled.empty();
  }

  void net::eval_cross ( const transition_id_type& tid
                       , we::util::cross_type& cross
                       )
  {
    if (!cross.empty())
    {
      const we::type::transition_t& transition (get_transition (tid));

      FHG_UTIL_STAT_START ("eval    " + transition.name());

      do
      {
        FHG_UTIL_STAT_INC ("eval    " + transition.name());

        if (cross.eval (transition))
        {
          _enabled.insert (tid);

          cross.write_to (_enabled_choice[tid]);

          FHG_UTIL_STAT_INC  ("enable  " + transition.name());
          FHG_UTIL_STAT_STOP ("eval    " + transition.name());

          return;
        }
      }
      while (cross.step());

      FHG_UTIL_STAT_INC  ("disable " + transition.name());
      FHG_UTIL_STAT_STOP ("eval    " + transition.name());
    }

    disable (tid);
  }

  void net::update_enabled (const transition_id_type& tid)
  {
    we::util::cross_type cross;

    BOOST_FOREACH ( const place_id_type& place_id
                  , in_to_transition (tid) | boost::adaptors::map_keys
                  )
    {
      std::list<pnet::type::value::value_type>&
        tokens (_token_by_place_id[place_id]);

      if (tokens.empty())
      {
        disable (tid);

        return;
      }

      cross.push (place_id, tokens);
    }

    eval_cross (tid, cross);
  }

  void net::update_enabled_put_token
    ( const transition_id_type& tid
    , const place_id_type& pid
    , const std::list<pnet::type::value::value_type>::iterator& token
    )
  {
    if (_enabled.elem (tid))
    {
      return;
    }

    we::util::cross_type cross;

    BOOST_FOREACH ( const place_id_type& place_id
                  , in_to_transition (tid) | boost::adaptors::map_keys
                  )
    {
      if (place_id == pid)
      {
        cross.push (place_id, token);
      }
      else
      {
        std::list<pnet::type::value::value_type>&
          tokens (_token_by_place_id[place_id]);

        if (tokens.empty())
        {
          disable (tid);

          return;
        }

        cross.push (place_id, tokens);
      }
    }

    eval_cross (tid, cross);
  }

  void net::disable (const transition_id_type& tid)
  {
    _enabled.erase (tid);
    _enabled_choice.erase (tid);
  }

  we::mgmt::type::activity_t
  net::extract_activity (const transition_id_type& tid)
  {
    const we::type::transition_t& transition (get_transition (tid));
    we::mgmt::type::activity_t act (transition);

    boost::unordered_set<transition_id_type> transitions_to_update;

    typedef std::pair< place_id_type
                     , we::util::pos_and_distance_type
                     > place_and_token_type;

    BOOST_FOREACH (const place_and_token_type& pt, _enabled_choice.at (tid))
    {
      const place_id_type& pid (pt.first);
      const we::util::pos_and_distance_type& pos_and_distance (pt.second);
      const std::list<pnet::type::value::value_type>::iterator&
        token (pos_and_distance.first);

      act.add_input (std::make_pair ( *token
                                    , transition.outer_to_inner (pid)
                                    )
                    );

      if (!is_read_connection (tid, pid))
      {
        _token_by_place_id.at (pid).erase (token);

        BOOST_FOREACH ( const transition_id_type& t
                      , out_of_place (pid) | boost::adaptors::map_keys
                      )
        {
          transitions_to_update.insert (t);
        }
      }
    }

    BOOST_FOREACH (const transition_id_type& t, transitions_to_update)
    {
      update_enabled (t);
    }

    return act;
  }
} // namespace petri_net
