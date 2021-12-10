// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/type/net.hpp>
#include <we/type/Activity.hpp>
#include <we/type/Transition.hpp>
#include <we/type/value.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/wrap.hpp>
#include <we/type/value/unwrap.hpp>

#include <we/require_type.hpp>

#include <fhg/util/next.hpp>

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_container.hpp>
#include <util-generic/functor_visitor.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/join.hpp>

#include <boost/variant/get.hpp>

#include <boost/format.hpp>

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

        net_type& _net;
        transition_id_type _tid;
        we::type::Transition const& _transition;
        ::boost::optional<Expression> const& _condition;
        expr::parse::node::KeyRoots _key_roots;

        typedef std::unordered_map<place_id_type, iterators_type> map_type;

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

    place_id_type net_type::add_place (place::type const& place)
    {
      const place_id_type pid (_place_id++);

      _pmap.emplace (pid, place);

      if (!_place_id_by_name.emplace (place.name(), pid).second)
      {
        //! \todo more specific exception
        throw std::invalid_argument
          ( ( ::boost::format ("duplicate place with name '%1%'")
            % place.name()
            ).str()
          );
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
                 ( std::piecewise_construct
                 , std::forward_as_tuple (port_id)
                 , std::forward_as_tuple (place_id, property)
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
                 ( std::piecewise_construct
                 , std::forward_as_tuple (port_id)
                 , std::forward_as_tuple (place_id, property)
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
                 ( std::piecewise_construct
                 , std::forward_as_tuple (place_id)
                 , std::forward_as_tuple (port_id, property)
                 ).second
               )
            {
              throw std::logic_error ("duplicate connection");
            }
            update_enabled (transition_id);
          }
        , [&] (edge::PT_READ)
          {
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
           ( std::piecewise_construct
           , std::forward_as_tuple (port_id)
           , std::forward_as_tuple (to, property)
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
          ( ( ::boost::format ("put_token (\"%1%\", %2%): place not found")
            % place_name
            % pnet::type::value::show (value)
            ).str()
          );
      }

      place::type const& place (_pmap.at (pid->second));

      if (!place.is_marked_for_put_token())
      {
        throw std::invalid_argument
          ( ( ::boost::format
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
      (place_id_type pid, pnet::type::value::value_type const& value)
    {
      place::type const& place (_pmap.at (pid));
      token_id_type const token_id (_token_id++);

      _token_by_place_id[pid].emplace
        (token_id, pnet::require_type (value, place.signature(), place.name()));

      return {pid, token_id};
    }

    void net_type::do_update (to_be_updated_type const& to_be_updated)
    {
      for ( transition_id_type tid
          : ::boost::join
              ( _adj_pt_consume.left.equal_range (to_be_updated.first)
              , _adj_pt_read.left.equal_range (to_be_updated.first)
              )
          | ::boost::adaptors::map_values
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
           for ( place_id_type place_id
               : adj.right.equal_range (tid) | ::boost::adaptors::map_values
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

      cross_type cross (*this, tid);

      auto&& push
        ( [&] (adj_pt_type const& adj, bool is_read_connection)
          {
            for ( place_id_type place_id
                : adj.right.equal_range (tid) | ::boost::adaptors::map_values
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
      std::unordered_set<token_id_type> deleted_tokens;

      for ( token_to_be_deleted_type const& token_to_be_deleted
          : tokens_to_be_deleted
          )
      {
        _token_by_place_id
          .at (token_to_be_deleted.first).erase (token_to_be_deleted.second);

        deleted_tokens.emplace (token_to_be_deleted.second);
      }

      for ( token_to_be_deleted_type const& token_to_be_deleted
          : tokens_to_be_deleted
          )
      {
        for ( transition_id_type t
            : ::boost::join
                ( _adj_pt_consume.left.equal_range (token_to_be_deleted.first)
                , _adj_pt_read.left.equal_range (token_to_be_deleted.first)
                )
            | ::boost::adaptors::map_values
            )
        {
          auto const& priority (_tmap.at (t).priority());

          if (_enabled.count (priority) && _enabled.at (priority).count (t))
          {
            auto const& choices (_enabled_choice.at (t));

            if ( std::any_of
                 ( choices.cbegin(), choices.cend()
                 , [&] (decltype (*choices.cbegin()) choice)
                   {
                     return deleted_tokens.count (choice.second.first);
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

      std::forward_list<token_to_be_deleted_type> const tokens_to_be_deleted
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

      std::list<to_be_updated_type> pending_updates;

      for ( we::type::Transition::PortByID::value_type const& p
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
        else if (  _port_many_to_place.count (tid)
                && _port_many_to_place.at (tid). count (p.first)
                )
        {
          auto const& many_tokens
            (::boost::get<std::list<pnet::type::value::value_type>>
              (context.value ({p.second.name()}))
            );

          for (auto const& token : many_tokens) {
            pending_updates.emplace_back
              ( do_put_value
                ( _port_many_to_place.at (tid).at (p.first).first
                , token
                )
              );
          }
        }
        else if (  _port_to_eureka.count (tid)
                && _port_to_eureka.at (tid) == p.first
                )
        {
          auto const& ids
            ( ::boost::get<std::set<pnet::type::value::value_type>>
              (context.value ({p.second.name()}))
            );

          type::eureka_ids_type const eureka_ids
            = pnet::type::value::unwrap<type::eureka_id_type>(ids);

          if (eureka_ids.size())
          {
            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  eureka_response (eureka_ids);
                }
                , "inject result: sending eureka response failed"
              );
          }
        }
        else
        {
          fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                assert (_port_to_response.at (tid).count (p.first));

                std::string const to ( _port_to_response.at (tid)
                                     . at (p.first).first
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

    void net_type::inject ( transition_id_type tid
                          , TokensOnPorts const& output
                          , TokensOnPorts const& input
                          , workflow_response_callback workflow_response
                          , eureka_response_callback eureka_response
                          )
    {
      for (auto const& token_on_port : output)
      {
        if (  _port_to_place.count (tid)
           && _port_to_place.at (tid).count (token_on_port.second)
           )
        {
          put_value ( _port_to_place.at (tid).at (token_on_port.second).first
                    , token_on_port.first
                    );
        }
        else if (  _port_many_to_place.count (tid)
                && _port_many_to_place.at (tid).count (token_on_port.second)
                )
        {
          auto const& many_tokens
            (::boost::get<std::list<pnet::type::value::value_type>>
              (token_on_port.first)
            );

          for (auto const& token : many_tokens)
          {
            put_value
              ( _port_many_to_place.at (tid).at (token_on_port.second).first
              , token
              );
          }
        }
        else if (  _port_to_eureka.count (tid)
                && (_port_to_eureka.at (tid) == token_on_port.second)
                )
        {
          std::set<type::eureka_id_type> const eureka_ids
            ([&token_on_port]
             {
              auto const& ids
              ( ::boost::get<std::set<pnet::type::value::value_type>>
                 (token_on_port.first)
              );

              return pnet::type::value::unwrap<type::eureka_id_type>(ids);
             }()
            );

          if (eureka_ids.size())
          {
            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  eureka_response (eureka_ids);
                }
                , "inject result: sending eureka response failed"
              );
          }
        }
        else
        {
          fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                assert (_port_to_response.at (tid).count (token_on_port.second));

               pnet::type::value::value_type const description
                   ([this, &input, tid, &token_on_port]
                   {
                     std::string const to
                       (_port_to_response.at (tid).at (token_on_port.second).first);
                    we::port_id_type const input_port_id
                       (_tmap.at (tid).input_port_by_name (to));

                    return std::find_if
                       ( input.begin(), input.end()
                       , [&input_port_id] (TokenOnPort const& input_token_on_port)
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

    net_type& net_type::assert_correct_expression_types()
    {
      for (auto& transition : _tmap | ::boost::adaptors::map_values)
      {
        transition.assert_correct_expression_types();
      }

      return *this;
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
      auto const port (_net.place_to_port().at (_tid).at (pid).first);

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
