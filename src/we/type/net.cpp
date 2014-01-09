// mirko.rahn@itwm.fraunhofer.de

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/condition.hpp>

#include <we/require_type.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/join.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <stack>

namespace petri_net
{
  net::net()
    : _place_id()
    , _pmap()
    , _transition_id()
    , _tmap()
    , _adj_pt_consume()
    , _adj_pt_read()
    , _adj_tp()
    , _port_to_place()
    , _place_to_port()
    , _token_by_place_id()
    , _enabled()
    , _enabled_choice()
  {}
  net::net (const net& other)
    : _place_id (other._place_id)
    , _pmap (other._pmap)
    , _transition_id (other._transition_id)
    , _tmap (other._tmap)
    , _adj_pt_consume (other._adj_pt_consume)
    , _adj_pt_read (other._adj_pt_read)
    , _adj_tp (other._adj_tp)
    , _port_to_place (other._port_to_place)
    , _place_to_port (other._place_to_port)
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
    _adj_pt_consume = other._adj_pt_consume;
    _adj_pt_read = other._adj_pt_read;
    _adj_tp = other._adj_tp;
    _port_to_place = other._port_to_place;
    _place_to_port = other._place_to_port;
    _token_by_place_id = other._token_by_place_id;
    _enabled = other._enabled;

    get_enabled_choice (other);

    return *this;
  }
  void net::get_enabled_choice (const net& other)
  {
    typedef boost::unordered_map< place_id_type
                                , pos_and_distance_type
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

  void net::add_connection ( edge::type type
                           , transition_id_type transition_id
                           , place_id_type place_id
                           , port_id_type port_id
                           , we::type::property::type const& property
                           )
  {
    switch (type)
    {
    case edge::TP:
      _adj_tp.insert (adj_tp_type::value_type (transition_id, place_id));
      _port_to_place[transition_id].insert
        (port_to_place_with_info_type::value_type (port_id, place_id, property));
      break;
    case edge::PT:
      _adj_pt_consume.insert (adj_pt_type::value_type (place_id, transition_id));
      _place_to_port[transition_id].insert
        (place_to_port_with_info_type::value_type (place_id, port_id, property));
      update_enabled (transition_id);
      break;
    case edge::PT_READ:
      _adj_pt_read.insert (adj_pt_type::value_type (place_id, transition_id));
      _place_to_port[transition_id].insert
        (place_to_port_with_info_type::value_type (place_id, port_id, property));
      update_enabled (transition_id);
      break;
    }
  }

  void net::set_transition_priority (transition_id_type tid, priority_type prio)
  {
    _enabled.set_priority (tid, prio);
  }

  priority_type
  net::get_transition_priority (transition_id_type tid) const
  {
    return _enabled.get_priority (tid);
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

  net::adj_tp_type const& net::transition_to_place() const
  {
    return _adj_tp;
  }
  net::adj_pt_type const& net::place_to_transition_consume() const
  {
    return _adj_pt_consume;
  }
  net::adj_pt_type const& net::place_to_transition_read() const
  {
    return _adj_pt_read;
  }
  net::port_to_place_type const& net::port_to_place() const
  {
    return _port_to_place;
  }
  net::place_to_port_type const& net::place_to_port() const
  {
    return _place_to_port;
  }

  namespace
  {
    typedef boost::any_range
      < const petri_net::transition_id_type
      , boost::single_pass_traversal_tag
      , const petri_net::transition_id_type&
      , std::ptrdiff_t
      > transition_id_range_type;

    typedef boost::any_range
      < const petri_net::place_id_type
      , boost::single_pass_traversal_tag
      , const petri_net::place_id_type&
      , std::ptrdiff_t
      > place_id_range_type;
  }

  void net::put_value ( place_id_type pid
                      , const pnet::type::value::value_type& value
                      )
  {
    const place::type& place (_pmap.at (pid));

    std::list<pnet::type::value::value_type>& tokens (_token_by_place_id[pid]);
    std::list<pnet::type::value::value_type>::iterator const tokenpos
      ( tokens.insert
        ( tokens.end()
        , pnet::require_type (value, place.signature(), place.name())
        )
      );

    transition_id_range_type consume
      (_adj_pt_consume.left.equal_range (pid) | boost::adaptors::map_values);
    transition_id_range_type read
      (_adj_pt_read.left.equal_range (pid) | boost::adaptors::map_values);

    BOOST_FOREACH (transition_id_type tid, boost::join (consume, read))
    {
      update_enabled_put_token (tid, pid, tokenpos);
    }
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
  net::get_token (place_id_type pid) const
  {
    typedef boost::unordered_map< place_id_type
                                , std::list<pnet::type::value::value_type>
                                > token_by_place_id_t;

    token_by_place_id_t::const_iterator pos (_token_by_place_id.find (pid));

    return (pos != _token_by_place_id.end()) ? pos->second : no_tokens();
  }

  void net::delete_all_token (place_id_type pid)
  {
    _token_by_place_id.erase (pid);

    transition_id_range_type consume
      (_adj_pt_consume.left.equal_range (pid) | boost::adaptors::map_values);
    transition_id_range_type read
      (_adj_pt_read.left.equal_range (pid) | boost::adaptors::map_values);

    BOOST_FOREACH (transition_id_type tid, boost::join (consume, read))
    {
      disable (tid);
    }
  }

  bool net::can_fire() const
  {
    return not _enabled.empty();
  }

  void net::update_enabled (transition_id_type tid)
  {
    cross_type cross;

    place_id_range_type consume
      (_adj_pt_consume.right.equal_range (tid) | boost::adaptors::map_values);
    place_id_range_type read
      (_adj_pt_read.right.equal_range (tid) | boost::adaptors::map_values);

    BOOST_FOREACH (place_id_type place_id, boost::join (consume, read))
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

    if (cross.enables (this, tid))
    {
      _enabled.insert (tid);
      cross.write_to (_enabled_choice[tid]);
    }
    else
    {
      disable (tid);
    }
  }

  void net::update_enabled_put_token
    ( transition_id_type tid
    , place_id_type pid
    , const std::list<pnet::type::value::value_type>::iterator& token
    )
  {
    if (_enabled.elem (tid))
    {
      return;
    }

    cross_type cross;

    place_id_range_type consume
      (_adj_pt_consume.right.equal_range (tid) | boost::adaptors::map_values);
    place_id_range_type read
      (_adj_pt_read.right.equal_range (tid) | boost::adaptors::map_values);

    BOOST_FOREACH (place_id_type place_id, boost::join (consume, read))
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

    if (cross.enables (this, tid))
    {
      _enabled.insert (tid);
      cross.write_to (_enabled_choice[tid]);
    }
    else
    {
      disable (tid);
    }
  }

  void net::disable (transition_id_type tid)
  {
    _enabled.erase (tid);
    _enabled_choice.erase (tid);
  }

  void net::fire_expression ( transition_id_type transition_id
                            , we::type::transition_t const& transition
                            )
  {
    //! \todo further inline
    we::type::activity_t const activity
      (extract_activity (transition_id, transition));

    expr::eval::context context;

    BOOST_FOREACH ( const we::type::activity_t::input_t::value_type& top
                  , activity.input()
                  )
    {
      context.bind_ref (transition.ports().at (top.second).name(), top.first);
    }

    transition.expression()->ast().eval_all (context);

    BOOST_FOREACH
      ( we::type::transition_t::port_map_t::value_type const& p
      , transition.ports()
      )
    {
      if (p.second.is_output())
      {
        put_value
          ( _port_to_place.at (transition_id).left.find (p.first)->get_right()
          , context.value (p.second.name())
          );
      }
    }

  }

  we::type::activity_t net::extract_activity
    (transition_id_type tid, we::type::transition_t const& transition)
  {
    we::type::activity_t act (transition, tid);

    boost::unordered_set<transition_id_type> transitions_to_update;

    typedef std::pair< place_id_type
                     , pos_and_distance_type
                     > place_and_token_type;

    BOOST_FOREACH (const place_and_token_type& pt, _enabled_choice.at (tid))
    {
      place_id_type pid (pt.first);
      const pos_and_distance_type& pos_and_distance (pt.second);
      const std::list<pnet::type::value::value_type>::iterator&
        token (pos_and_distance.first);

      act.add_input ( _place_to_port.at (tid).left.find (pid)->get_right()
                    , *token
                    );

      //! \todo save the information whether or not it is a read
      //! connection in _enabled_choice and omit that conditional
      if (  _adj_pt_read.find (adj_pt_type::value_type (pid, tid))
         == _adj_pt_read.end()
         )
      {
        _token_by_place_id.at (pid).erase (token);

        transition_id_range_type consume
          (_adj_pt_consume.left.equal_range (pid) | boost::adaptors::map_values);
        transition_id_range_type read
          (_adj_pt_read.left.equal_range (pid) | boost::adaptors::map_values);

        BOOST_FOREACH (transition_id_type t, boost::join (consume, read))
        {
          transitions_to_update.insert (t);
        }
      }
    }

    BOOST_FOREACH (transition_id_type t, transitions_to_update)
    {
      update_enabled (t);
    }

    return act;
  }


  // cross_type

  net::cross_type::iterators_type::iterators_type (std::list<pnet::type::value::value_type>& tokens)
    : _begin (tokens.begin())
    , _end (tokens.end())
    , _pos_and_distance (tokens.begin(), 0)
  {}
  net::cross_type::iterators_type::iterators_type (const std::list<pnet::type::value::value_type>::iterator& token)
    : _begin (token)
    , _end (boost::next (token))
    , _pos_and_distance (token, 0)
  {}
  bool net::cross_type::iterators_type::end() const
  {
    return _end == _pos_and_distance.first;
  }
  const net::pos_and_distance_type& net::cross_type::iterators_type::pos_and_distance() const
  {
    return _pos_and_distance;
  }
  void net::cross_type::iterators_type::operator++()
  {
    ++_pos_and_distance.first;
    ++_pos_and_distance.second;
  }
  void net::cross_type::iterators_type::rewind()
  {
    _pos_and_distance.first = _begin;
    _pos_and_distance.second = 0;
  }

  bool net::cross_type::do_step
  (map_type::iterator slot, map_type::iterator const& end)
  {
    while (slot != end)
    {
      //! \note all sequences are non-empty
      ++slot->second;

      if (!slot->second.end())
      {
        return true;
      }

      slot->second.rewind();
      ++slot;
    }

    return false;
  }

  bool net::cross_type::enables (net* const n, transition_id_type transition_id)
  {
    //! \note that means the transitions without in-port cannot fire
    //! instead of fire unconditionally
    if (_m.empty())
    {
      return false;
    }

    we::type::transition_t const& transition
      (n->_tmap.at (transition_id));

    //! \todo use is_const_true and boost::optional...
    if (transition.condition().expression() == "true")
    {
      return true;
    }

    do
    {
      expr::eval::context context;

      typedef std::pair<place_id_type, iterators_type> pits_type;

      BOOST_FOREACH (const pits_type& pits, _m)
      {
        context.bind_ref
          ( transition.ports()
          .at (n->_place_to_port.at (transition_id).left.find (pits.first)->get_right()).name()
          , *pits.second.pos_and_distance().first
          );
      }

      if (transition.condition().parser().eval_all_bool (context))
      {
        return true;
      }
    }
    while (do_step (_m.begin(), _m.end()));

    return false;
  }
  void net::cross_type::write_to (boost::unordered_map< place_id_type
                                                      , pos_and_distance_type
                                                      >& choice
                                 ) const
  {
    choice.clear();

    typedef std::pair<place_id_type, iterators_type> pits_type;

    BOOST_FOREACH (const pits_type& pits, _m)
    {
      choice.insert
        (std::make_pair (pits.first, pits.second.pos_and_distance()));
    }
  }
  void net::cross_type::push ( place_id_type place_id
                             , std::list<pnet::type::value::value_type>& tokens
                             )
  {
    _m.insert (std::make_pair (place_id, iterators_type (tokens)));
  }
  void net::cross_type::push ( place_id_type place_id
                             , const std::list<pnet::type::value::value_type>::iterator& token
                             )
  {
    _m.insert (std::make_pair (place_id, iterators_type (token)));
  }

}
