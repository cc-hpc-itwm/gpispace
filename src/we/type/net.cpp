// mirko.rahn@itwm.fraunhofer.de

#include <we/type/net.hpp>
#include <we/type/transition.hpp>

#include <we/require_type.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/join.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <stack>

namespace we
{
  namespace type
  {
    namespace
    {
      class cross_type
      {
        typedef std::pair< std::list<pnet::type::value::value_type>::iterator
                         , std::list<pnet::type::value::value_type>::iterator::difference_type
                         > pos_and_distance_type;
      public:
        bool enables (net_type* const, transition_id_type);
        void write_to ( boost::unordered_map< place_id_type
                                            , pos_and_distance_type
                                            >&
                      ) const;
        void push (place_id_type, std::list<pnet::type::value::value_type>&);
        void push ( place_id_type
                  , const std::list<pnet::type::value::value_type>::iterator&
                  , const std::list<pnet::type::value::value_type>::iterator::difference_type&
                  );
      private:
        class iterators_type
        {
        public:
          iterators_type (std::list<pnet::type::value::value_type>&);
          iterators_type
            ( const std::list<pnet::type::value::value_type>::iterator&
            , const std::list<pnet::type::value::value_type>::iterator::difference_type&
            );
          bool end() const;
          const pos_and_distance_type& pos_and_distance() const;
          void operator++();
          void rewind();
        private:
          std::list<pnet::type::value::value_type>::iterator _begin;
          std::list<pnet::type::value::value_type>::iterator _end;
          std::list<pnet::type::value::value_type>::iterator::difference_type _distance_from_zero;
          pos_and_distance_type _pos_and_distance;
        };

        typedef boost::unordered_map<place_id_type, iterators_type> map_type;

        map_type _m;

        bool do_step (map_type::iterator, map_type::iterator const&);
      };
    }

    net_type::net_type()
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
    net_type::net_type (const net_type& other)
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
    net_type& net_type::operator= (const net_type& other)
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
    void net_type::get_enabled_choice (const net_type& other)
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

    place_id_type net_type::add_place (const place::type& place)
    {
      const place_id_type pid (_place_id++);

      _pmap.insert (std::make_pair (pid, place));

      return pid;
    }

    transition_id_type
    net_type::add_transition (const we::type::transition_t& transition)
    {
      const transition_id_type tid (_transition_id++);

      _tmap.insert (std::make_pair (tid, transition));

      return tid;
    }

    void net_type::add_connection ( edge::type type
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

    const boost::unordered_map<place_id_type,place::type>& net_type::places() const
    {
      return _pmap;
    }

    const boost::unordered_map<transition_id_type,we::type::transition_t>&
    net_type::transitions() const
    {
      return _tmap;
    }

    net_type::adj_tp_type const& net_type::transition_to_place() const
    {
      return _adj_tp;
    }
    net_type::adj_pt_type const& net_type::place_to_transition_consume() const
    {
      return _adj_pt_consume;
    }
    net_type::adj_pt_type const& net_type::place_to_transition_read() const
    {
      return _adj_pt_read;
    }
    net_type::port_to_place_type const& net_type::port_to_place() const
    {
      return _port_to_place;
    }
    net_type::place_to_port_type const& net_type::place_to_port() const
    {
      return _place_to_port;
    }

    namespace
    {
      typedef boost::any_range
        < const we::transition_id_type
        , boost::single_pass_traversal_tag
        , const we::transition_id_type&
        , std::ptrdiff_t
        > transition_id_range_type;

      typedef boost::any_range
        < const we::place_id_type
        , boost::single_pass_traversal_tag
        , const we::place_id_type&
        , std::ptrdiff_t
        > place_id_range_type;
    }

    void net_type::put_value ( place_id_type pid
                        , const pnet::type::value::value_type& value
                        )
    {
      do_update (do_put_value (pid, value));
    }

    net_type::to_be_updated_type net_type::do_put_value
      (place_id_type pid, const pnet::type::value::value_type& value)
    {
      const place::type& place (_pmap.at (pid));

      std::list<pnet::type::value::value_type>& tokens (_token_by_place_id[pid]);
      std::list<pnet::type::value::value_type>::iterator const tokenpos
        ( tokens.insert
          ( tokens.end()
          , pnet::require_type (value, place.signature(), place.name())
          )
        );

      return boost::make_tuple
        (pid, tokenpos, std::distance (tokens.begin(), tokenpos));
    }

    void net_type::do_update (to_be_updated_type const& to_be_updated)
    {
      transition_id_range_type consume
        ( _adj_pt_consume.left.equal_range (to_be_updated.get<0>())
        | boost::adaptors::map_values
        );
      transition_id_range_type read
        ( _adj_pt_read.left.equal_range (to_be_updated.get<0>())
        | boost::adaptors::map_values
        );

      BOOST_FOREACH (transition_id_type tid, boost::join (consume, read))
      {
        update_enabled_put_token (tid, to_be_updated);
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
    net_type::get_token (place_id_type pid) const
    {
      typedef boost::unordered_map< place_id_type
                                  , std::list<pnet::type::value::value_type>
                                  > token_by_place_id_t;

      token_by_place_id_t::const_iterator pos (_token_by_place_id.find (pid));

      return (pos != _token_by_place_id.end()) ? pos->second : no_tokens();
    }

    void net_type::update_enabled (transition_id_type tid)
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
        _enabled[_tmap.at (tid).priority()].insert (tid);
        cross.write_to (_enabled_choice[tid]);
      }
      else
      {
        disable (tid);
      }
    }

    void net_type::update_enabled_put_token
      ( transition_id_type tid
      , to_be_updated_type const& to_be_updated
      )
    {
      {
        enabled_type::const_iterator const pos
          (_enabled.find (_tmap.at (tid).priority()));

        if (pos != _enabled.end() && pos->second.find (tid) != pos->second.end())
        {
          return;
        }
      }

      cross_type cross;

      place_id_range_type consume
        (_adj_pt_consume.right.equal_range (tid) | boost::adaptors::map_values);
      place_id_range_type read
        (_adj_pt_read.right.equal_range (tid) | boost::adaptors::map_values);

      BOOST_FOREACH (place_id_type place_id, boost::join (consume, read))
      {
        if (place_id == to_be_updated.get<0>())
        {
          cross.push (place_id, to_be_updated.get<1>(), to_be_updated.get<2>());
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
        _enabled[_tmap.at (tid).priority()].insert (tid);
        cross.write_to (_enabled_choice[tid]);
      }
      else
      {
        disable (tid);
      }
    }

    void net_type::disable (transition_id_type tid)
    {
      enabled_type::iterator const pos
        (_enabled.find (_tmap.at (tid).priority()));

      if (pos != _enabled.end())
      {
        pos->second.erase (tid);

        if (pos->second.empty())
        {
          _enabled.erase (pos);
        }
      }

      _enabled_choice.erase (tid);
    }

    std::list<net_type::token_to_be_deleted_type> net_type::do_extract
      ( transition_id_type tid
      , boost::function<void ( port_id_type
                             , pnet::type::value::value_type const&
                             )
                        > fun
      ) const
    {
      std::list<token_to_be_deleted_type> tokens_to_be_deleted;

      typedef std::pair< place_id_type
                       , pos_and_distance_type
                       > place_and_token_type;

      BOOST_FOREACH (const place_and_token_type& pt, _enabled_choice.at (tid))
      {
        place_id_type pid (pt.first);
        const pos_and_distance_type& pos_and_distance (pt.second);
        const std::list<pnet::type::value::value_type>::iterator&
          token (pos_and_distance.first);

        fun (_place_to_port.at (tid).left.find (pid)->get_right(), *token);

        //! \todo save the information whether or not it is a read
        //! connection in _enabled_choice and omit that conditional
        if (  _adj_pt_read.find (adj_pt_type::value_type (pid, tid))
           == _adj_pt_read.end()
           )
        {
          tokens_to_be_deleted.push_back (std::make_pair (pid, token));
        }
      }

      return tokens_to_be_deleted;
    }

    void net_type::do_delete
      (std::list<token_to_be_deleted_type> const& tokens_to_be_deleted)
    {
      BOOST_FOREACH ( token_to_be_deleted_type const& token_to_be_deleted
                    , tokens_to_be_deleted
                    )
      {
        _token_by_place_id
          .at (token_to_be_deleted.first).erase (token_to_be_deleted.second);
      }

      BOOST_FOREACH ( token_to_be_deleted_type const& token_to_be_deleted
                    , tokens_to_be_deleted
                    )
      {
        transition_id_range_type consume
          ( _adj_pt_consume.left.equal_range (token_to_be_deleted.first)
          | boost::adaptors::map_values
          );
        transition_id_range_type read
          ( _adj_pt_read.left.equal_range (token_to_be_deleted.first)
          | boost::adaptors::map_values
          );

        BOOST_FOREACH (transition_id_type t, boost::join (consume, read))
        {
          update_enabled (t);
        }
      }
    }

    we::type::activity_t net_type::extract_activity
      (transition_id_type tid, we::type::transition_t const& transition)
    {
      we::type::activity_t act (transition, tid);

      do_delete
        ( do_extract ( tid
                     , boost::bind ( &we::type::activity_t::add_input, &act
                                   , _1, _2
                                   )
                     )
        );

      return act;
    }

    namespace
    {
      class context_bind
      {
      public:
        context_bind ( expr::eval::context& context
                     , we::type::transition_t const& transition
                     )
          : _context (context)
          , _transition (transition)
        {}

        void operator() ( port_id_type port_id
                        , pnet::type::value::value_type const& value
                        )
        {
          _context.bind_ref (_transition.ports_input().at (port_id).name(), value);
        }

      private:
        expr::eval::context& _context;
        we::type::transition_t const& _transition;
      };
    }

    void net_type::fire_expression ( transition_id_type tid
                              , we::type::transition_t const& transition
                              )
    {
      expr::eval::context context;

      std::list<token_to_be_deleted_type> const tokens_to_be_deleted
        (do_extract (tid, context_bind (context, transition)));

      transition.expression()->ast().eval_all (context);

      std::list<to_be_updated_type> pending_updates;

      BOOST_FOREACH
        ( we::type::transition_t::port_map_t::value_type const& p
        , transition.ports_output()
        )
      {
          pending_updates.push_back
            ( do_put_value
              ( _port_to_place.at (tid).left.find (p.first)->get_right()
              , context.value (p.second.name())
              )
            );
      }

      do_delete (tokens_to_be_deleted);

      BOOST_FOREACH (to_be_updated_type const& to_be_updated, pending_updates)
      {
        do_update (to_be_updated);
      }
    }


    // cross_type

    cross_type::iterators_type::iterators_type (std::list<pnet::type::value::value_type>& tokens)
      : _begin (tokens.begin())
      , _end (tokens.end())
      , _distance_from_zero (0)
      , _pos_and_distance (tokens.begin(), _distance_from_zero)
    {}
    cross_type::iterators_type::iterators_type
      ( const std::list<pnet::type::value::value_type>::iterator& token
      , const std::list<pnet::type::value::value_type>::iterator::difference_type& distance_from_zero
      )
      : _begin (token)
      , _end (boost::next (token))
      , _distance_from_zero (distance_from_zero)
      , _pos_and_distance (token, distance_from_zero)
    {}
    bool cross_type::iterators_type::end() const
    {
      return _end == _pos_and_distance.first;
    }
    const cross_type::pos_and_distance_type& cross_type::iterators_type::pos_and_distance() const
    {
      return _pos_and_distance;
    }
    void cross_type::iterators_type::operator++()
    {
      ++_pos_and_distance.first;
      ++_pos_and_distance.second;
    }
    void cross_type::iterators_type::rewind()
    {
      _pos_and_distance.first = _begin;
      _pos_and_distance.second = _distance_from_zero;
    }

    bool cross_type::do_step
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

    bool cross_type::enables (net_type* const n, transition_id_type transition_id)
    {
      //! \note that means the transitions without in-port cannot fire
      //! instead of fire unconditionally
      if (_m.empty())
      {
        return false;
      }

      we::type::transition_t const& transition
        (n->transitions().at (transition_id));

      if (_m.size() < transition.ports_input().size())
      {
        return false;
      }

      if (!transition.condition())
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
            ( transition.ports_input()
            .at (n->place_to_port().at (transition_id).left.find (pits.first)->get_right()).name()
            , *pits.second.pos_and_distance().first
            );
        }

        if (transition.condition()->ast().eval_all_bool (context))
        {
          return true;
        }
      }
      while (do_step (_m.begin(), _m.end()));

      return false;
    }
    void cross_type::write_to (boost::unordered_map< place_id_type
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
    void cross_type::push ( place_id_type place_id
                          , std::list<pnet::type::value::value_type>& tokens
                          )
    {
      _m.insert (std::make_pair (place_id, iterators_type (tokens)));
    }
    void cross_type::push
      ( place_id_type place_id
      , const std::list<pnet::type::value::value_type>::iterator& token
      , const std::list<pnet::type::value::value_type>::iterator::difference_type& distance_from_zero
      )
    {
      _m.insert
        (std::make_pair (place_id, iterators_type (token, distance_from_zero)));
    }
  }
}
