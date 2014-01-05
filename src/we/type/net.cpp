// mirko.rahn@itwm.fraunhofer.de

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/condition.hpp>

#include <we/require_type.hpp>
#include <we/exception.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
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

  void net::add_connection
    (edge::type type, transition_id_type transition_id, place_id_type place_id)
  {
    if (type == edge::TP)
    {
      _adj_tp.insert (adj_tp_type::value_type (transition_id, place_id));
    }
    else
    {
      _adj_pt.insert (adj_pt_type::value_type (place_id, transition_id, type));

      update_enabled (transition_id);
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

  const place::type& net::get_place (const place_id_type& pid) const
  {
    boost::unordered_map<place_id_type, place::type>::const_iterator const
      pos (_pmap.find (pid));

    if (pos == _pmap.end())
    {
      throw pnet::exception::place::no_such (pid);
    }

    return pos->second;
  }

  const we::type::transition_t&
  net::get_transition (const transition_id_type& tid) const
  {
    boost::unordered_map<transition_id_type, we::type::transition_t>
      ::const_iterator const pos (_tmap.find (tid));

    if (pos == _tmap.end())
    {
      throw pnet::exception::transition::no_such (tid);
    }

    return pos->second;
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
  net::adj_pt_type const& net::place_to_transition() const
  {
    return _adj_pt;
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

  boost::select_second_const_range<std::pair<net::adj_tp_type::left_const_iterator, net::adj_tp_type::left_const_iterator> >
  net::out_of_transition (const transition_id_type& tid) const
  {
    return _adj_tp.left.equal_range (tid) | boost::adaptors::map_values;
  }
  boost::select_second_const_range<std::pair<net::adj_pt_type::right_const_iterator, net::adj_pt_type::right_const_iterator> >
  net::in_to_transition (const transition_id_type& tid) const
  {
    return _adj_pt.right.equal_range (tid) | boost::adaptors::map_values;
  }
  boost::select_second_const_range<std::pair<net::adj_pt_type::left_const_iterator, net::adj_pt_type::left_const_iterator> >
  net::out_of_place (const place_id_type& pid) const
  {
    return _adj_pt.left.equal_range (pid) | boost::adaptors::map_values;
  }
  boost::select_second_const_range<std::pair<net::adj_tp_type::right_const_iterator, net::adj_tp_type::right_const_iterator> >
  net::in_to_place (const place_id_type& pid) const
  {
    return _adj_tp.right.equal_range (pid) | boost::adaptors::map_values;
  }

  bool net::is_read_connection ( const transition_id_type& tid
                               , const place_id_type& pid
                               ) const
  {
    adj_pt_type::const_iterator const pos
      (_adj_pt.find (adj_pt_type::value_type (pid, tid)));

    if (pos == _adj_pt.end())
    {
      throw pnet::exception::connection::no_such
        <transition_id_type, place_id_type> (tid, pid);
    }

    return pos->info == edge::PT_READ;
  }

  void net::delete_edge_out ( const transition_id_type& tid
                            , const place_id_type& pid
                            )
  {
    _adj_tp.erase (adj_tp_type::value_type (tid, pid));
  }
  void net::delete_edge_in ( const transition_id_type& tid
                           , const place_id_type& pid
                           )
  {
    _adj_pt.erase (adj_pt_type::value_type (pid, tid));

    update_enabled (tid);
  }

  void net::delete_place (const place_id_type& pid)
  {
    // make the token deletion visible to delete_connection
    _token_by_place_id.erase (pid);

    std::stack<std::pair<transition_id_type, place_id_type> > stack_out;
    std::stack<std::pair<transition_id_type, place_id_type> > stack_in;

    BOOST_FOREACH ( const transition_id_type& tid
                  , out_of_place (pid)
                  )
    {
      stack_in.push (std::make_pair (tid, pid));
      // TODO: get port and remove place from there
    }

    BOOST_FOREACH ( const transition_id_type& transition_id
                  , in_to_place (pid)
                  )
    {
      stack_out.push (std::make_pair (transition_id, pid));
      // TODO: get port and remove place from there
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
                  , out_of_transition (tid)
                  )
    {
      stack_out.push (std::make_pair (tid, place_id));
    }

    BOOST_FOREACH ( const place_id_type& place_id
                  , in_to_transition (tid)
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

  void net::put_value ( const place_id_type& pid
                      , const pnet::type::value::value_type& value
                      )
  {
    const place::type& place (get_place (pid));

    std::list<pnet::type::value::value_type>& tokens (_token_by_place_id[pid]);
    std::list<pnet::type::value::value_type>::iterator const tokenpos
      ( tokens.insert
        ( tokens.end()
        , pnet::require_type (value, place.signature(), place.name())
        )
      );

    BOOST_FOREACH ( const transition_id_type& tid
                  , out_of_place (pid)
                  )
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
                  , out_of_place (pid)
                  )
    {
      disable (tid);
    }
  }

  bool net::can_fire() const
  {
    return not _enabled.empty();
  }

  namespace
  {
    class get_port_name
    {
    public:
      get_port_name (we::type::transition_t const& transition)
        : _transition (transition)
      {}
      std::string const&
        operator() (petri_net::place_id_type const& place_id) const
      {
        return _transition.get_port
          (_transition.outer_to_inner().at (place_id).first).name();
      }
    private:
      we::type::transition_t const& _transition;
    };
  }

  void net::update_enabled (const transition_id_type& tid)
  {
    cross_type cross;

    BOOST_FOREACH ( const place_id_type& place_id
                  , in_to_transition (tid)
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

    we::type::transition_t const& transition (get_transition (tid));

    if (cross.enables (get_port_name (transition), transition.condition()))
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
    ( const transition_id_type& tid
    , const place_id_type& pid
    , const std::list<pnet::type::value::value_type>::iterator& token
    )
  {
    if (_enabled.elem (tid))
    {
      return;
    }

    cross_type cross;

    BOOST_FOREACH ( const place_id_type& place_id
                  , in_to_transition (tid)
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

    we::type::transition_t const& transition (get_transition (tid));

    if (cross.enables (get_port_name (transition), transition.condition()))
    {
      _enabled.insert (tid);
      cross.write_to (_enabled_choice[tid]);
    }
    else
    {
      disable (tid);
    }
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
                     , pos_and_distance_type
                     > place_and_token_type;

    BOOST_FOREACH (const place_and_token_type& pt, _enabled_choice.at (tid))
    {
      const place_id_type& pid (pt.first);
      const pos_and_distance_type& pos_and_distance (pt.second);
      const std::list<pnet::type::value::value_type>::iterator&
        token (pos_and_distance.first);

      act.add_input
        (std::make_pair ( *token
                        , transition.outer_to_inner().at (pid).first
                        )
        );

      if (!is_read_connection (tid, pid))
      {
        _token_by_place_id.at (pid).erase (token);

        BOOST_FOREACH ( const transition_id_type& t
                      , out_of_place (pid)
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

  bool net::cross_type::enables
  ( boost::function<std::string const& (petri_net::place_id_type const&)> name
  , condition::type const& condition
  )
  {
    //! \note that means the transitions without in-port cannot fire
    //! instead of fire unconditionally
    if (_m.empty())
    {
      return false;
    }

    //! \todo use is_const_true and boost::optional...
    if (condition.expression() == "true")
    {
      return true;
    }

    do
    {
      expr::eval::context context;

      typedef std::pair<petri_net::place_id_type, iterators_type> pits_type;

      BOOST_FOREACH (const pits_type& pits, _m)
      {
        context.bind_ref
          (name (pits.first), *pits.second.pos_and_distance().first);
      }

      if (condition.parser().eval_all_bool (context))
      {
        return true;
      }
    }
    while (do_step (_m.begin(), _m.end()));

    return false;
  }
  void net::cross_type::write_to (boost::unordered_map< petri_net::place_id_type
                                                      , pos_and_distance_type
                                                      >& choice
                                 ) const
  {
    choice.clear();

    typedef std::pair<petri_net::place_id_type, iterators_type> pits_type;

    BOOST_FOREACH (const pits_type& pits, _m)
    {
      choice.insert
        (std::make_pair (pits.first, pits.second.pos_and_distance()));
    }
  }
  void net::cross_type::push ( const petri_net::place_id_type& place_id
                             , std::list<pnet::type::value::value_type>& tokens
                             )
  {
    _m.insert (std::make_pair (place_id, iterators_type (tokens)));
  }
  void net::cross_type::push ( const petri_net::place_id_type& place_id
                             , const std::list<pnet::type::value::value_type>::iterator& token
                             )
  {
    _m.insert (std::make_pair (place_id, iterators_type (token)));
  }

}
