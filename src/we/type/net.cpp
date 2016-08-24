#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/show.hpp>

#include <we/require_type.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/join.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace we
{
  namespace type
  {
    namespace
    {
      class cross_type
      {
      public:
        bool enables (net_type* const, transition_id_type);
        void write_to ( std::unordered_map< place_id_type
                                          , std::pair<token_id_type, bool>
                                          >&
                      ) const;
        void push (place_id_type, net_type::token_by_id_type const&, bool);
        void push
          (place_id_type, net_type::token_by_id_type::const_iterator, bool);

      private:
        class iterators_type
        {
        public:
          iterators_type (net_type::token_by_id_type const&, bool);
          iterators_type (net_type::token_by_id_type::const_iterator, bool);
          bool end() const;
          net_type::token_by_id_type::const_iterator const& pos() const;
          bool is_read_connection() const;
          void operator++();
          void rewind();
        private:
          net_type::token_by_id_type::const_iterator const _begin;
          net_type::token_by_id_type::const_iterator const _end;
          net_type::token_by_id_type::const_iterator _pos;
          bool const _is_read_connection;
        };

        typedef std::unordered_map<place_id_type, iterators_type> map_type;

        map_type _m;

        bool do_step (map_type::iterator, map_type::iterator const&);
      };
    }

    place_id_type net_type::add_place (const place::type& place)
    {
      const place_id_type pid (_place_id++);

      _pmap.emplace (pid, place);

      if (!_place_id_by_name.emplace (place.name(), pid).second)
      {
        //! \todo more specific exception
        throw std::invalid_argument
          ( ( boost::format ("duplicate place with name '%1%'")
            % place.name()
            ).str()
          );
      }

      return pid;
    }

    transition_id_type
    net_type::add_transition (const we::type::transition_t& transition)
    {
      const transition_id_type tid (_transition_id++);

      _tmap.emplace (tid, transition);

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
        _adj_tp.emplace (place_id, transition_id);
        if (!_port_to_place[transition_id].emplace
             ( std::piecewise_construct
             , std::forward_as_tuple (port_id)
             , std::forward_as_tuple (place_id, property)
             ).second
           )
        {
          throw std::logic_error ("duplicate connection");
        }
        break;
      case edge::PT:
        _adj_pt_consume.insert
          (adj_pt_type::value_type (place_id, transition_id));
        if (!_place_to_port[transition_id].emplace
             ( std::piecewise_construct
             , std::forward_as_tuple (place_id)
             , std::forward_as_tuple (port_id, property)
             ).second
           )
        {
          throw std::logic_error ("duplicate connection");
        }
        update_enabled (transition_id);
        break;
      case edge::PT_READ:
        _adj_pt_read.insert (adj_pt_type::value_type (place_id, transition_id));
        if (!_place_to_port[transition_id].emplace
             ( std::piecewise_construct
             , std::forward_as_tuple (place_id)
             , std::forward_as_tuple (port_id, property)
             ).second
           )
        {
          throw std::logic_error ("duplicate read connection");
        }
        update_enabled (transition_id);
        break;
      }
    }

    void net_type::add_response ( transition_id_type transition_id
                                , port_id_type port_id
                                , std::string const& to
                                , we::type::property::type const& property
                                )
    {
      _port_to_response[transition_id].insert
        (port_to_response_with_info_type::value_type (port_id, to, property));
    }

    const std::unordered_map<place_id_type, place::type>&
      net_type::places() const
    {
      return _pmap;
    }

    const std::unordered_map<transition_id_type, we::type::transition_t>&
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
    net_type::port_to_response_type const& net_type::port_to_response() const
    {
      return _port_to_response;
    }
    net_type::place_to_port_type const& net_type::place_to_port() const
    {
      return _place_to_port;
    }

    void net_type::put_value ( place_id_type pid
                             , const pnet::type::value::value_type& value
                             )
    {
      do_update (do_put_value (pid, value));
    }

    void net_type::put_token ( std::string place_name
                             , pnet::type::value::value_type const& value
                             )
    {
      std::unordered_map<std::string, place_id_type>::const_iterator const
        pid (_place_id_by_name.find (place_name));

      if (pid == _place_id_by_name.end())
      {
        throw std::invalid_argument
          ( ( boost::format ("put_token (\"%1%\", %2%): place not found")
            % place_name
            % pnet::type::value::show (value)
            ).str()
          );
      }

      place::type const& place (_pmap.at (pid->second));

      if (!place.is_marked_for_put_token())
      {
        throw std::invalid_argument
          ( ( boost::format
              ("put_token (\"%1%\", %2%): place not marked with attribute"
              " put_token=\"true\""
              )
            % place_name
            % pnet::type::value::show (value)
            ).str()
          );
      }

      return put_value (pid->second, value);
    }

    net_type::to_be_updated_type net_type::do_put_value
      (place_id_type pid, const pnet::type::value::value_type& value)
    {
      const place::type& place (_pmap.at (pid));
      token_id_type const token_id (_token_id++);

      _token_by_place_id[pid].emplace
        (token_id, pnet::require_type (value, place.signature(), place.name()));

      return {pid, token_id};
    }

    void net_type::do_update (to_be_updated_type const& to_be_updated)
    {
      for ( transition_id_type tid
          : boost::join
              ( _adj_pt_consume.left.equal_range (to_be_updated.first)
              , _adj_pt_read.left.equal_range (to_be_updated.first)
              )
          | boost::adaptors::map_values
          )
      {
        update_enabled_put_token (tid, to_be_updated);
      }
    }

    namespace
    {
      const net_type::token_by_id_type& no_tokens()
      {
        static const net_type::token_by_id_type x;

        return x;
      }
    }

    const net_type::token_by_id_type&
      net_type::get_token (place_id_type pid) const
    {
      token_by_place_id_type::const_iterator const pos
        (_token_by_place_id.find (pid));

      return (pos != _token_by_place_id.end()) ? pos->second : no_tokens();
    }

    void net_type::update_enabled (transition_id_type tid)
    {
      cross_type cross;

      auto&& push
        ([&] (adj_pt_type& adj, bool is_read_connection)
         {
           for ( place_id_type place_id
               : adj.right.equal_range (tid) | boost::adaptors::map_values
               )
           {
             token_by_id_type const& tokens (_token_by_place_id[place_id]);

             if (tokens.empty())
             {
               disable (tid);

               return false;
             }

             cross.push (place_id, tokens, is_read_connection);
           }

           return true;
         }
        );

      if (push (_adj_pt_consume, false) && push (_adj_pt_read, true))
      {
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
    }

    void net_type::update_enabled_put_token
      ( transition_id_type tid
      , to_be_updated_type const& to_be_updated
      )
    {
      {
        enabled_type::const_iterator const pos
          (_enabled.find (_tmap.at (tid).priority()));

        if (  pos != _enabled.end()
           && pos->second.find (tid) != pos->second.end()
           )
        {
          return;
        }
      }

      cross_type cross;

      auto&& push
        ( [&] (adj_pt_type& adj, bool is_read_connection)
          {
            for ( place_id_type place_id
                : adj.right.equal_range (tid) | boost::adaptors::map_values
                )
            {
              if (place_id == to_be_updated.first)
              {
                cross.push
                  ( place_id
                  , _token_by_place_id.at (place_id).find (to_be_updated.second)
                  , is_read_connection
                  );
              }
              else
              {
                token_by_id_type const& tokens (_token_by_place_id[place_id]);

                if (tokens.empty())
                {
                  disable (tid);

                  return false;
                }

                cross.push (place_id, tokens, is_read_connection);
              }
            }

            return true;
          }
        );

      if (push (_adj_pt_consume, false) && push (_adj_pt_read, true))
      {
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

    std::forward_list<net_type::token_to_be_deleted_type> net_type::do_extract
      ( transition_id_type tid
      , std::function<void ( port_id_type
                           , pnet::type::value::value_type const&
                           )
                      > fun
      ) const
    {
      std::forward_list<token_to_be_deleted_type> tokens_to_be_deleted;

      for ( std::pair< place_id_type const
                     , std::pair<token_id_type, bool>
                     > const& pt
          : _enabled_choice.at (tid)
          )
      {
        place_id_type const pid (pt.first);
        token_id_type const token_id (pt.second.first);
        bool const is_read_connection (pt.second.second);

        fun ( _place_to_port.at (tid).at (pid).first
            , _token_by_place_id.at (pid).at (token_id)
            );

        if (!is_read_connection)
        {
          tokens_to_be_deleted.emplace_front (pid, token_id);
        }
      }

      return tokens_to_be_deleted;
    }

    void net_type::do_delete
      (std::forward_list<token_to_be_deleted_type> const& tokens_to_be_deleted)
    {
      for ( token_to_be_deleted_type const& token_to_be_deleted
          : tokens_to_be_deleted
          )
      {
        _token_by_place_id
          .at (token_to_be_deleted.first).erase (token_to_be_deleted.second);
      }

      for ( token_to_be_deleted_type const& token_to_be_deleted
          : tokens_to_be_deleted
          )
      {
        for ( transition_id_type t
            : boost::join
                ( _adj_pt_consume.left.equal_range (token_to_be_deleted.first)
                , _adj_pt_read.left.equal_range (token_to_be_deleted.first)
                )
            | boost::adaptors::map_values
            )
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
                     , std::bind ( &we::type::activity_t::add_input, &act
                                 , std::placeholders::_1, std::placeholders::_2
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
          _context.bind_ref
            (_transition.ports_input().at (port_id).name(), value);
        }

      private:
        expr::eval::context& _context;
        we::type::transition_t const& _transition;
      };
    }

    void net_type::fire_expression
      ( transition_id_type tid
      , we::type::transition_t const& transition
      , we::workflow_response_callback const& workflow_response
      )
    {
      expr::eval::context context;

      std::forward_list<token_to_be_deleted_type> const tokens_to_be_deleted
        (do_extract (tid, context_bind (context, transition)));

      transition.expression()->ast().eval_all (context);

      std::list<to_be_updated_type> pending_updates;

      for ( we::type::transition_t::port_map_t::value_type const& p
          : transition.ports_output()
          )
      {
        if (  _port_to_place.count (tid)
           && _port_to_place.at (tid).count (p.first)
           )
        {
          pending_updates.emplace_back
            ( do_put_value
              ( _port_to_place.at (tid).at (p.first).first
              , context.value ({p.second.name()})
              )
            );
        }
        else
        {
          fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                assert (_port_to_response.at (tid).left.count (p.first));

                std::string const to ( _port_to_response.at (tid)
                                     . left.find (p.first)->get_right()
                                     );

                workflow_response ( context.value ({to})
                                  , context.value ({p.second.name()})
                                  );
              }
              , "fire_expression: sending workflow response failed"
            );
        }
      }

      do_delete (tokens_to_be_deleted);

      for (to_be_updated_type const& to_be_updated : pending_updates)
      {
        do_update (to_be_updated);
      }
    }

    void net_type::inject ( activity_t const& child
                          , workflow_response_callback workflow_response
                          )
    {
      for (auto const& token_on_port : child.output())
      {
        if ( _port_to_place.count (*child.transition_id())
           && _port_to_place.at (*child.transition_id())
            . count (token_on_port.second)
           )
        {
          put_value ( _port_to_place.at (*child.transition_id())
                    . at (token_on_port.second).first
                    , token_on_port.first
                    );
        }
        else
        {
          fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                assert ( _port_to_response.at (*child.transition_id())
                       . left.count (token_on_port.second)
                       );
               pnet::type::value::value_type const description
                  ([this, &child, &token_on_port]
                   {
                     std::string const to
                       ( _port_to_response.at (*child.transition_id())
                       . left.find (token_on_port.second)->get_right()
                       );
                    we::port_id_type const input_port_id
                       (child.transition().input_port_by_name (to));

                    return std::find_if
                       ( child._input.begin(), child._input.end()
                       , [&input_port_id] (activity_t::token_on_port_t const& input_token_on_port)
                         {
                           return input_token_on_port.second == input_port_id;
                         }
                       )->first;
                   }()
                  );
               workflow_response (description, token_on_port.first);
              }
            , "inject result: sending workflow response failed"
            );
        }
      }
    }


    // cross_type

    cross_type::iterators_type::iterators_type
      (net_type::token_by_id_type const& tokens, bool is_read_connection)
        : _begin (tokens.begin())
        , _end (tokens.end())
        , _pos (_begin)
        , _is_read_connection (is_read_connection)
    {}
    cross_type::iterators_type::iterators_type
      ( net_type::token_by_id_type::const_iterator token
      , bool is_read_connection
      )
        : _begin (std::move (token))
        , _end (std::next (_begin))
        , _pos (_begin)
        , _is_read_connection (is_read_connection)
    {}
    bool cross_type::iterators_type::end() const
    {
      return _end == _pos;
    }
    net_type::token_by_id_type::const_iterator const&
      cross_type::iterators_type::pos() const
    {
      return _pos;
    }
    bool cross_type::iterators_type::is_read_connection() const
    {
      return _is_read_connection;
    }
    void cross_type::iterators_type::operator++()
    {
      ++_pos;
    }
    void cross_type::iterators_type::rewind()
    {
      _pos = _begin;
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

        for (std::pair<place_id_type const, iterators_type> const& pits : _m)
        {
          context.bind_ref
            ( transition.ports_input()
            . at (n->place_to_port().at (transition_id)
            . at (pits.first).first).name()
            , pits.second.pos()->second
            );
        }

        if (boost::get<bool>
            (transition.condition()->ast().eval_all (context))
           )
        {
          return true;
        }
      }
      while (do_step (_m.begin(), _m.end()));

      return false;
    }
    void cross_type::write_to
      ( std::unordered_map< place_id_type
                          , std::pair<token_id_type, bool>
                          >& choice
      ) const
    {
      for (std::pair<place_id_type const, iterators_type> const& pits : _m)
      {
        choice[pits.first] = std::make_pair ( pits.second.pos()->first
                                            , pits.second.is_read_connection()
                                            );
      }
    }
    void cross_type::push ( place_id_type place_id
                          , net_type::token_by_id_type const& tokens
                          , bool is_read_connection
                          )
    {
      _m.emplace (place_id, iterators_type (tokens, is_read_connection));
    }
    void cross_type::push ( place_id_type place_id
                          , net_type::token_by_id_type::const_iterator token
                          , bool is_read_connection
                          )
    {
      _m.emplace
        (place_id, iterators_type (std::move (token), is_read_connection));
    }
  }
}
