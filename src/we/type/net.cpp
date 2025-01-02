// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/Activity.hpp>
#include <we/type/Transition.hpp>
#include <we/type/net.hpp>
#include <we/type/value.hpp>
#include <we/type/value/name.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/unwrap.hpp>
#include <we/type/value/wrap.hpp>

#include <we/require_type.hpp>

#include <fhg/util/next.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_container.hpp>

#include <boost/range/join.hpp>

#include <boost/variant/get.hpp>

#include <fmt/core.h>
#include <FMT/we/type/value/show.hpp>
#include <algorithm>
#include <exception>
#include <list>
#include <stdexcept>
#include <unordered_set>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ALLOW_CONVERSION (ID, unsigned long);
    }
  }
}

namespace we
{
  namespace edge
  {
    bool is_incoming (type const& e)
    {
      return fhg::util::visit<bool>
        ( e
        , [] (PT const&) { return true; }
        , [] (PT_READ const&) { return true; }
        , [] (PT_NUMBER_OF_TOKENS const&) { return true; }
        , [] (auto&) { return false; }
        );
    }

    std::ostream& operator<< (std::ostream& os, PT const&)
    {
      return os << "in";
    }
    std::ostream& operator<< (std::ostream& os, PT_READ const&)
    {
      return os << "read";
    }
    std::ostream& operator<< (std::ostream& os, PT_NUMBER_OF_TOKENS const&)
    {
      return os << "number-of-tokens";
    }
    std::ostream& operator<< (std::ostream& os, TP const&)
    {
      return os << "out";
    }
    std::ostream& operator<< (std::ostream& os, TP_MANY const&)
    {
      return os << "out-many";
    }
  }

  namespace type
  {
    namespace
    {
      class cross_type
      {
      public:
        cross_type (net_type&, transition_id_type);
        bool enables();
        void write_to ( std::unordered_map< place_id_type
                                          , net_type::SelectedToken
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

        net_type& _net;
        transition_id_type _tid;
        we::type::Transition const& _transition;
        ::boost::optional<Expression> const& _condition;
        expr::parse::node::KeyRoots _key_roots;

        using map_type = std::unordered_map<place_id_type, iterators_type>;

        map_type _m;

        using ReferencedPlaces = std::unordered_set<place_id_type>;

        ReferencedPlaces _referenced_places;

        bool is_referenced (place_id_type) const;
        std::string const& input_port_name (place_id_type) const;
        bool do_step ( ReferencedPlaces::const_iterator
                     , ReferencedPlaces::const_iterator const&
                     );
      };
    }

    auto net_type::number_of_tokens_place
      ( place_id_type place_id
      ) const noexcept -> std::optional<place_id_type>
    {
      auto const number_of_tokens_place
        {_number_of_tokens_place.find (place_id)};

      if (number_of_tokens_place == std::end (_number_of_tokens_place))
      {
        return {};
      }

      return number_of_tokens_place->second;
    }

    auto net_type::add_number_of_tokens_place
      ( place_id_type place_id
      ) -> place_id_type
    {
      auto const number_of_tokens_place_id
        { add_place ( place::type { "#" + _pmap.at (place_id).name()
                                  , pnet::type::value::ULONG()
                                  , false
                                  , type::property::type{}
                                  }
                    )
        };

      if (!_number_of_tokens_place
           . emplace (place_id, number_of_tokens_place_id)
           . second
         )
      {
        throw std::logic_error {"Duplicate number of tokens place."};
      }

      auto const tokens {_token_by_place_id.find (place_id)};

      do_put_value
        ( number_of_tokens_place_id
        , tokens == std::end (_token_by_place_id) ? 0UL : tokens->second.size()
        );

      return number_of_tokens_place_id;
    }

    template<typename Update>
      auto net_type::update_number_of_tokens
        ( place_id_type number_of_tokens_place_id
        , Update&& update
        ) -> void
    {
      auto const& tokens {_token_by_place_id.at (number_of_tokens_place_id)};

      fhg_assert (tokens.size() == 1);

      auto const [token_id, value] {*std::cbegin (tokens)};

      do_delete ({{number_of_tokens_place_id, token_id}});
      put_value ( number_of_tokens_place_id
                , std::invoke
                  ( std::forward<Update> (update)
                  , ::boost::get<unsigned long> (value)
                  )
                );
    }

    auto net_type::increment_number_of_tokens
      ( place_id_type number_of_tokens_place_id
      ) -> void
    {
      update_number_of_tokens
        ( number_of_tokens_place_id
        , [] (unsigned long number_of_tokens)
          {
            return number_of_tokens + 1UL;
          }
        );
    }

    auto net_type::decrement_number_of_tokens
      ( place_id_type number_of_tokens_place_id
      ) -> void
    {
      update_number_of_tokens
        ( number_of_tokens_place_id
        , [] (unsigned long number_of_tokens)
          {
            fhg_assert (number_of_tokens > 0UL);

            return number_of_tokens - 1UL;
          }
        );
    }

    place_id_type net_type::add_place (place::type const& place)
    {
      const place_id_type pid (_place_id++);

      _pmap.emplace (pid, place);

      if (!_place_id_by_name.emplace (place.name(), pid).second)
      {
        //! \todo more specific exception
        throw std::invalid_argument
          {fmt::format ("duplicate place with name '{}'", place.name())};
      }

      return pid;
    }

    transition_id_type
    net_type::add_transition (we::type::Transition const& transition)
    {
      const transition_id_type tid (_transition_id++);

      _tmap.emplace (tid, transition);

      return tid;
    }

    namespace
    {
      void require_no_output_connection
        ( net_type::port_to_place_type const& port_to_place
        , transition_id_type const transition_id
        , port_id_type const port_id
        )
      {
        auto const output_connector (port_to_place.find (transition_id));

        if (  output_connector != port_to_place.end ()
           && output_connector->second.count (port_id) > 0
           )
        {
          throw std::logic_error ("duplicate connection: out and out-many");
        }
      }
    }

    void net_type::add_connection ( edge::type type
                                  , transition_id_type transition_id
                                  , place_id_type place_id
                                  , port_id_type port_id
                                  , we::type::property::type const& property
                                  )
    {
      fhg::util::visit<void>
        ( type
        , [&] (edge::TP)
          {
            require_no_output_connection (_port_many_to_place, transition_id, port_id);
            _adj_tp.emplace (place_id, transition_id);
            if (!_port_to_place[transition_id].emplace
                 ( port_id
                 , PlaceIDWithProperty {place_id, property}
                 ).second
               )
            {
              throw std::logic_error ("duplicate connection");
            }
          }
        , [&] (edge::TP_MANY)
          {
            require_no_output_connection (_port_to_place, transition_id, port_id);
            _adj_tp.emplace (place_id, transition_id);
            if (!_port_many_to_place[transition_id].emplace
                 ( port_id
                 , PlaceIDWithProperty {place_id, property}
                 ).second
               )
            {
              throw std::logic_error ("duplicate connection");
            }
          }
        , [&] (edge::PT)
          {
            _adj_pt_consume.insert
              (adj_pt_type::value_type (place_id, transition_id));
            if (!_place_to_port[transition_id].emplace
                 ( place_id
                 , PortIDWithProperty {port_id, property}
                 ).second
               )
            {
              throw std::logic_error ("duplicate connection");
            }
            if (!_consumed_from_places_by_port[transition_id]
                 . emplace (port_id, place_id)
                 . second
               )
            {
              throw std::logic_error {"duplicate consume"};
            }
            update_enabled (transition_id);
          }
        , [&] (edge::PT_READ)
          {
            _adj_pt_read.insert (adj_pt_type::value_type (place_id, transition_id));
            if (!_place_to_port[transition_id].emplace
                 ( place_id
                 , PortIDWithProperty {port_id, property}
                 ).second
               )
            {
              throw std::logic_error ("duplicate read connection");
            }
            update_enabled (transition_id);
          }
        , [&] (edge::PT_NUMBER_OF_TOKENS)
          {
            return add_connection
              ( edge::PT_READ{}
              , transition_id
              , std::invoke
                ( [&]
                  {
                    if ( auto number_of_tokens_place_id
                          {number_of_tokens_place (place_id)}
                       )
                    {
                      return *number_of_tokens_place_id;
                    }

                    return add_number_of_tokens_place (place_id);
                  }
                )
              , port_id
              , property
              );
          }
        );
    }

    void net_type::add_response ( transition_id_type transition_id
                                , port_id_type port_id
                                , std::string const& to
                                , we::type::property::type const& property
                                )
    {
      if (!_port_to_response[transition_id].emplace
           ( port_id
           , ResponseWithProperty {to, property}
           ).second
         )
      {
        throw std::logic_error ("duplicate response");
      }
    }

    void net_type::add_eureka ( transition_id_type transition_id
                              , port_id_type port_id
                              )
    {
      if (!_port_to_eureka.emplace (transition_id, port_id).second)
      {
        throw std::logic_error ("duplicate eureka");
      }
    }

    const std::unordered_map<place_id_type, place::type>&
      net_type::places() const
    {
      return _pmap;
    }

    const std::unordered_map<transition_id_type, we::type::Transition>&
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
    net_type::port_to_place_type const& net_type::port_many_to_place() const
    {
      return _port_many_to_place;
    }
    net_type::port_to_response_type const& net_type::port_to_response() const
    {
      return _port_to_response;
    }
    net_type::port_to_eureka_type const& net_type::port_to_eureka() const
    {
      return _port_to_eureka;
    }
    net_type::place_to_port_type const& net_type::place_to_port() const
    {
      return _place_to_port;
    }

    void net_type::put_value ( place_id_type pid
                             , pnet::type::value::value_type const& value
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
          { fmt::format ( "put_token (\"{}\", {}): place not found"
                        , place_name
                        , pnet::type::value::show (value)
                        )
          };
      }

      place::type const& place (_pmap.at (pid->second));

      if (!place.is_marked_for_put_token())
      {
        throw std::invalid_argument
          { fmt::format
              ( "put_token (\"{}\", {}): place not marked with attribute"
                " put_token=\"true\""
              , place_name
              , pnet::type::value::show (value)
              )
          };
      }

      return put_value (pid->second, value);
    }

    net_type::ToBeUpdated net_type::do_put_value
      (place_id_type pid, pnet::type::value::value_type const& value)
    {
      place::type const& place (_pmap.at (pid));
      token_id_type const token_id (_token_id++);

      _token_by_place_id[pid].emplace
        (token_id, pnet::require_type (value, place.signature(), place.name()));

      if (auto const number_of_tokens_place_id {number_of_tokens_place (pid)})
      {
        increment_number_of_tokens (*number_of_tokens_place_id);
      }

      return ToBeUpdated {pid, token_id};
    }

    void net_type::do_update (ToBeUpdated const& to_be_updated)
    {
      for ( auto [_ignore, tid]
          : ::boost::join
              ( _adj_pt_consume.left.equal_range (to_be_updated._place_id)
              , _adj_pt_read.left.equal_range (to_be_updated._place_id)
              )
          )
      {
        update_enabled_put_token (tid, to_be_updated);
      }
    }

    namespace
    {
      net_type::token_by_id_type const& no_tokens()
      {
        static const net_type::token_by_id_type x;

        return x;
      }
    }

    net_type::token_by_id_type const&
      net_type::get_token (place_id_type pid) const
    {
      token_by_place_id_type::const_iterator const pos
        (_token_by_place_id.find (pid));

      return (pos != _token_by_place_id.end()) ? pos->second : no_tokens();
    }

    void net_type::update_enabled (transition_id_type tid)
    {
      cross_type cross (*this, tid);

      auto&& push
        ([&] (adj_pt_type const& adj, bool is_read_connection)
         {
           for ( auto [adj_place, end] {adj.right.equal_range (tid)}
               ; adj_place != end
               ; ++adj_place
               )
           {
             auto const place_id {adj_place->second};
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
        if (cross.enables())
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
      , ToBeUpdated const& to_be_updated
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

      cross_type cross (*this, tid);

      auto&& push
        ( [&] (adj_pt_type const& adj, bool is_read_connection)
          {
            for ( auto [adj_place, end] {adj.right.equal_range (tid)}
                ; adj_place != end
                ; ++adj_place
                )
            {
              auto const place_id {adj_place->second};
              if (place_id == to_be_updated._place_id)
              {
                cross.push
                  ( place_id
                  , _token_by_place_id.at (place_id).find (to_be_updated._token_id)
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
        if (cross.enables())
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

    std::forward_list<net_type::ToBeDeleted> net_type::do_extract
      ( transition_id_type tid
      , std::function<void ( port_id_type
                           , pnet::type::value::value_type const&
                           )
                      > fun
      ) const
    {
      std::forward_list<ToBeDeleted> tokens_to_be_deleted;

      for ( auto const& [pid, selected_token] : _enabled_choice.at (tid)
          )
      {
        auto const [token_id, is_read_connection] {selected_token};

        fun ( _place_to_port.at (tid).at (pid)._port_id
            , _token_by_place_id.at (pid).at (token_id)
            );

        if (!is_read_connection)
        {
          tokens_to_be_deleted.emplace_front (ToBeDeleted {pid, token_id});
        }
      }

      return tokens_to_be_deleted;
    }

    void net_type::do_delete
      (std::forward_list<ToBeDeleted> const& tokens_to_be_deleted)
    {
      std::unordered_set<token_id_type> deleted_tokens;

      for ( ToBeDeleted const& token_to_be_deleted
          : tokens_to_be_deleted
          )
      {
        _token_by_place_id
          . at (token_to_be_deleted._place_id)
          . erase (token_to_be_deleted._token_id)
          ;

        deleted_tokens.emplace (token_to_be_deleted._token_id);
      }

      for ( ToBeDeleted const& token_to_be_deleted
          : tokens_to_be_deleted
          )
      {
        for ( auto [_ignore, t]
            : ::boost::join
                ( _adj_pt_consume.left.equal_range (token_to_be_deleted._place_id)
                , _adj_pt_read.left.equal_range (token_to_be_deleted._place_id)
                )
            )
        {
          auto const& priority (_tmap.at (t).priority());

          if (_enabled.count (priority) && _enabled.at (priority).count (t))
          {
            auto const& choices (_enabled_choice.at (t));

            if ( std::any_of
                 ( choices.cbegin(), choices.cend()
                 , [&] (auto const& choice)
                   {
                     return deleted_tokens.count (choice.second._id);
                   }
                 )
               )
            {
              update_enabled (t);
            }
          }
        }
      }
    }

    we::type::Activity net_type::extract_activity
      (transition_id_type tid, we::type::Transition const& transition)
    {
      we::type::Activity act (transition, tid);

      do_delete
        ( do_extract ( tid
                     , [&] ( port_id_type port_id
                           , pnet::type::value::value_type const& value
                           )
                       {
                         act.add_input (port_id, value);
                       }
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
                     , we::type::Transition const& transition
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
        we::type::Transition const& _transition;
      };
    }

    ::boost::optional<we::type::Activity>
      net_type::fire_expressions_and_extract_activity_random
        ( std::mt19937& engine
        , we::workflow_response_callback const& workflow_response
        , we::eureka_response_callback const& eureka_response
        , gspc::we::plugin::Plugins& plugins
        , gspc::we::plugin::PutToken put_token
        )
    {
      while (!_enabled.empty())
      {
        std::unordered_set<we::transition_id_type> const& transition_ids
          (_enabled.begin()->second);
        std::uniform_int_distribution<std::size_t> random
          (0, transition_ids.size() - 1);
        transition_id_type const transition_id
          (*fhg::util::next (transition_ids.begin(), random (engine)));
        we::type::Transition const& transition (_tmap.at (transition_id));

        if (transition.expression())
        {
          fire_expression ( transition_id
                          , transition
                          , workflow_response
                          , eureka_response
                          , plugins
                          , put_token
                          );
        }
        else
        {
          return extract_activity (transition_id, transition);
        }
      }

      return ::boost::none;
    }

    ::boost::optional<we::type::Activity>
      net_type::fire_expressions_and_extract_activity_random_TESTING_ONLY
        ( std::mt19937& engine
        , we::workflow_response_callback const& workflow_response
        , we::eureka_response_callback const& eureka_response
        )
    {
      gspc::we::plugin::Plugins plugins;
      return fire_expressions_and_extract_activity_random
        ( engine
        , workflow_response
        , eureka_response
        , plugins
        , [] (std::string, pnet::type::value::value_type)
          {
            throw std::logic_error ("Unexpected call to put_token.");
          }
        );
    }

    void net_type::fire_expression
      ( transition_id_type tid
      , we::type::Transition const& transition
      , we::workflow_response_callback const& workflow_response
      , we::eureka_response_callback const& eureka_response
      , gspc::we::plugin::Plugins& plugins
      , gspc::we::plugin::PutToken put_token
      )
    {
      using pnet::type::value::peek;
      using values = std::list<pnet::type::value::value_type>;

      expr::eval::context context;

      std::forward_list<ToBeDeleted> const tokens_to_be_deleted
        (do_extract (tid, context_bind (context, transition)));

      auto const plugin_commands
        (transition.prop().get ({"gspc", "we", "plugin"}));

      if (plugin_commands)
      {
        {
          auto const call_before_eval
            (peek ("call_before_eval", plugin_commands.get()));

          if (call_before_eval)
          {
            Expression const expression
              (::boost::get<std::string> (call_before_eval.get()));
            auto const pids (expression.ast().eval_all (context));

            for (auto const& pid : ::boost::get<values> (pids))
            {
              plugins.before_eval
                ( gspc::we::plugin::ID {::boost::get<unsigned long> (pid)}
                , context
                );
            }
          }
        }

        if (peek ("destroy", plugin_commands.get()))
        {
          plugins.destroy
            ( gspc::we::plugin::ID
                {::boost::get<unsigned long> (context.value ({"plugin_id"}))}
            );
        }
      }

      transition.expression()->ast().eval_all (context);

      if (plugin_commands)
      {
        if (peek ("create", plugin_commands.get()))
        {
          context.bind_and_discard_ref
            ( {"plugin_id"}
            , static_cast<unsigned long>
              ( plugins.create
                ( ::boost::get<std::string> (context.value ({"plugin_path"}))
                , context
                , std::move (put_token)
                )
              )
            );
        }

        {
          auto const call_after_eval
            (peek ("call_after_eval", plugin_commands.get()));

          if (call_after_eval)
          {
            Expression const expression
              (::boost::get<std::string> (call_after_eval.get()));
            auto const pids (expression.ast().eval_all (context));

            for (auto const& pid : ::boost::get<values> (pids))
            {
              plugins.after_eval
                ( gspc::we::plugin::ID {::boost::get<unsigned long> (pid)}
                , context
                );
            }
          }
        }
      }

      std::list<ToBeUpdated> pending_updates;

      for (auto const& [port_id, port] : transition.ports_output())
      {
        if (  _port_to_place.count (tid)
           && _port_to_place.at (tid).count (port_id)
           )
        {
          pending_updates.emplace_back
            ( do_put_value
              ( _port_to_place.at (tid).at (port_id)._place_id
              , context.value ({port.name()})
              )
            );
        }
        else if (  _port_many_to_place.count (tid)
                && _port_many_to_place.at (tid). count (port_id)
                )
        {
          auto const& many_tokens
            (::boost::get<std::list<pnet::type::value::value_type>>
              (context.value ({port.name()}))
            );

          for (auto const& token : many_tokens) {
            pending_updates.emplace_back
              ( do_put_value
                ( _port_many_to_place.at (tid).at (port_id)._place_id
                , token
                )
              );
          }
        }
        else if (  _port_to_eureka.count (tid)
                && _port_to_eureka.at (tid) == port_id
                )
        {
          auto const& ids
            ( ::boost::get<std::set<pnet::type::value::value_type>>
              (context.value ({port.name()}))
            );

          type::eureka_ids_type const eureka_ids
            = pnet::type::value::unwrap<type::eureka_id_type> (ids);

          if (eureka_ids.size())
          {
            try
            {
              eureka_response (eureka_ids);
            }
            catch (...)
            {
              std::throw_with_nested
                ( std::runtime_error
                    {"inject result: sending eureka response failed"}
                );
            }
          }
        }
        else
        {
          try
          {
            assert (_port_to_response.at (tid).count (port_id));

            std::string const to ( _port_to_response.at (tid)
                                 . at (port_id)._to
                                 );

            workflow_response ( context.value ({to})
                              , context.value ({port.name()})
                              );
          }
          catch (...)
          {
            std::throw_with_nested
              ( std::runtime_error
                {"fire_expression: sending workflow response failed"}
              );
          }
        }
      }

      do_delete (tokens_to_be_deleted);

      for (ToBeUpdated const& to_be_updated : pending_updates)
      {
        do_update (to_be_updated);
      }

      for (auto const [place_id, _] : tokens_to_be_deleted)
      {
        if ( auto const number_of_tokens_place_id
              {number_of_tokens_place (place_id)}
           )
        {
          decrement_number_of_tokens (*number_of_tokens_place_id);
        }
      }
    }

    void net_type::inject ( transition_id_type tid
                          , TokensOnPorts const& outputs
                          , TokensOnPorts const& input
                          , workflow_response_callback workflow_response
                          , eureka_response_callback eureka_response
                          )
    {
      {
        auto const consumed_from_places
          {_consumed_from_places_by_port.find (tid)};

        if (consumed_from_places != std::end (_consumed_from_places_by_port))
        {
          for (auto const& [_, port_id] : input)
          {
            auto const consumed_from_place
              {consumed_from_places->second.find (port_id)};

            if (consumed_from_place != std::end (consumed_from_places->second))
            {
              if (auto const number_of_tokens_place_id
                    {number_of_tokens_place (consumed_from_place->second)}
                 )
              {
                decrement_number_of_tokens (*number_of_tokens_place_id);
              }
            }
          }
        }
      }

      for (auto const& output : outputs)
      {
        if (  _port_to_place.count (tid)
           && _port_to_place.at (tid).count (output._port_id)
           )
        {
          put_value ( _port_to_place.at (tid).at (output._port_id)._place_id
                    , output._token
                    );
        }
        else if (  _port_many_to_place.count (tid)
                && _port_many_to_place.at (tid).count (output._port_id)
                )
        {
          auto const& many_tokens
            (::boost::get<std::list<pnet::type::value::value_type>>
              (output._token)
            );

          for (auto const& token : many_tokens)
          {
            put_value
              ( _port_many_to_place.at (tid).at (output._port_id)._place_id
              , token
              );
          }
        }
        else if (  _port_to_eureka.count (tid)
                && (_port_to_eureka.at (tid) == output._port_id)
                )
        {
          std::set<type::eureka_id_type> const eureka_ids
            ([&output]
             {
              auto const& ids
              ( ::boost::get<std::set<pnet::type::value::value_type>>
                 (output._token)
              );

              return pnet::type::value::unwrap<type::eureka_id_type>(ids);
             }()
            );

          if (eureka_ids.size())
          {
            try
            {
              eureka_response (eureka_ids);
            }
            catch (...)
            {
              std::throw_with_nested
                ( std::runtime_error
                    {"inject result: sending eureka response failed"}
                );
            }
          }
        }
        else
        {
          try
          {
            assert (_port_to_response.at (tid).count (output._port_id));

           pnet::type::value::value_type const description
               ([this, &input, tid, &output]
               {
                 std::string const to
                   (_port_to_response.at (tid).at (output._port_id)._to);
                we::port_id_type const input_port_id
                   (_tmap.at (tid).input_port_by_name (to));

                return std::find_if
                   ( input.begin(), input.end()
                   , [&input_port_id] (TokenOnPort const& input_token_on_port)
                     {
                       return input_token_on_port._port_id == input_port_id;
                     }
                   )->_token;
               }()
              );
           workflow_response (description, output._token);
          }
          catch (...)
          {
            std::throw_with_nested
              ( std::runtime_error
                  {"inject result: sending workflow response failed"}
              );
          }
        }
      }
    }

    net_type& net_type::assert_correct_expression_types()
    {
      for (auto& [_ignore, transition] : _tmap)
      {
        transition.assert_correct_expression_types();
      }

      return *this;
    }

    bool net_type::might_use_virtual_memory() const
    {
      return std::any_of
        ( _tmap.begin()
        , _tmap.end()
        , [&] (auto const& id_and_transition)
          {
            return id_and_transition.second.might_use_virtual_memory();
          }
        );
    }

    bool net_type::might_have_tasks_requiring_multiple_workers() const
    {
      return std::any_of
        ( _tmap.begin()
        , _tmap.end()
        , [&] (auto const& id_and_transition)
          {
            return id_and_transition.second.might_have_tasks_requiring_multiple_workers();
          }
        );
    }

    bool net_type::might_use_modules_with_multiple_implementations() const
    {
      return std::any_of
        ( _tmap.begin()
        , _tmap.end()
        , [&] (auto const& id_and_transition)
          {
            return id_and_transition.second.might_use_modules_with_multiple_implementations();
          }
        );
    }

    // cross_type

    cross_type::cross_type (net_type& net, transition_id_type tid)
      : _net (net)
      , _tid (tid)
      , _transition (_net.transitions().at (_tid))
      , _condition (_transition.condition())
      , _key_roots ( _condition
                   ? _condition->ast().key_roots()
                   : decltype (_condition->ast().key_roots()) {}
                   )
    {}
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

    bool cross_type::do_step ( ReferencedPlaces::const_iterator place
                             , ReferencedPlaces::const_iterator const& end
                             )
    {
      while (place != end)
      {
        auto slot (_m.find (*place));

        //! \note all sequences are non-empty
        ++slot->second;

        if (!slot->second.end())
        {
          return true;
        }

        slot->second.rewind();
        ++place;
      }

      return false;
    }

    bool cross_type::is_referenced (place_id_type pid) const
    {
      return _key_roots.count (input_port_name (pid)) > 0;
    }
    std::string const& cross_type::input_port_name (place_id_type pid) const
    {
      auto const port (_net.place_to_port().at (_tid).at (pid)._port_id);

      auto input (_transition.ports_input().find (port));

      return input != _transition.ports_input().end()
        ? input->second.name()
        : _transition.ports_tunnel().at (port).name()
        ;
    }

    bool cross_type::enables()
    {
      if (_m.size() < _transition.ports_input().size())
      {
        return false;
      }

      if (!_condition)
      {
        return true;
      }

      do
      {
        expr::eval::context context;

        for (auto const& place : _referenced_places)
        {
          context.bind_ref
            ( input_port_name (place)
            , _m.at (place).pos()->second
            );
        }

        if (::boost::get<bool> (_condition->ast().eval_all (context)))
        {
          return true;
        }
      }
      while (do_step (_referenced_places.cbegin(), _referenced_places.cend()));

      return false;
    }
    void cross_type::write_to
      ( std::unordered_map<place_id_type, net_type::SelectedToken>& choice
      ) const
    {
      for (auto const& [place_id, iterators] : _m)
      {
        choice[place_id] = net_type::SelectedToken
          { iterators.pos()->first
          , iterators.is_read_connection()
          };
      }
    }
    void cross_type::push ( place_id_type place_id
                          , net_type::token_by_id_type const& tokens
                          , bool is_read_connection
                          )
    {
      _m.emplace (place_id, iterators_type (tokens, is_read_connection));

      if (is_referenced (place_id))
      {
        _referenced_places.emplace (place_id);
      }
    }
    void cross_type::push ( place_id_type place_id
                          , net_type::token_by_id_type::const_iterator token
                          , bool is_read_connection
                          )
    {
      _m.emplace
        (place_id, iterators_type (std::move (token), is_read_connection));

      if (is_referenced (place_id))
      {
        _referenced_places.emplace (place_id);
      }
    }
  }
}
