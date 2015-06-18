// mirko.rahn@itwm.fraunhofer.de

#include <we/layer.hpp>

#include <fhg/assert.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/starts_with.hpp>

#include <boost/range/adaptor/map.hpp>

#include <functional>
#include <sstream>

namespace we
{
    layer::layer
        ( std::function<void (id_type, type::activity_t)> rts_submit
        , std::function<void (id_type)> rts_cancel
        , std::function<void (id_type, type::activity_t)> rts_finished
        , std::function<void (id_type, std::string)> rts_failed
        , std::function<void (id_type)> rts_canceled
        , std::function<void (id_type, id_type)> rts_discover
        , std::function<void (id_type, sdpa::discovery_info_t)> rts_discovered
        , std::function<void (std::string)> rts_token_put
        , std::function<id_type()> rts_id_generator
        , std::mt19937& random_extraction_engine
        )
      : _rts_submit (rts_submit)
      , _rts_cancel (rts_cancel)
      , _rts_finished (rts_finished)
      , _rts_failed (rts_failed)
      , _rts_canceled (rts_canceled)
      , _rts_discover (rts_discover)
      , _rts_discovered (rts_discovered)
      , _rts_token_put (rts_token_put)
      , _rts_id_generator (rts_id_generator)
      , _random_extraction_engine (random_extraction_engine)
      , _extract_from_nets_thread (&layer::extract_from_nets, this)
    {}

    namespace
    {
      std::string wrapped_activity_prefix()
      {
        return "_wrap_";
      }

      std::string wrapped_name (we::type::port_t const& port)
      {
        return (port.is_output() ? "_out_" : "_in_") + port.name();
      }

      type::activity_t wrap (type::activity_t const& activity)
      {
        we::type::net_type net;

        we::transition_id_type const transition_id
          (net.add_transition (activity.transition()));

        fhg_assert (activity.transition().ports_tunnel().size() == 0);

        std::unordered_map<std::string, we::place_id_type> place_ids;

        for ( we::type::transition_t::port_map_t::value_type const& p
            : activity.transition().ports_input()
            )
        {
          we::place_id_type const place_id
            (net.add_place (place::type ( wrapped_name (p.second)
                                        , p.second.signature()
                                        , boost::none
                                        )
                           )
            );

          net.add_connection ( we::edge::PT
                             , transition_id
                             , place_id
                             , p.first
                             , we::type::property::type()
                             );

          place_ids.emplace (wrapped_name (p.second), place_id);
        }
        for ( we::type::transition_t::port_map_t::value_type const& p
            : activity.transition().ports_output()
            )
        {
          we::place_id_type const place_id
            (net.add_place (place::type ( wrapped_name (p.second)
                                        , p.second.signature()
                                        , boost::none
                                        )
                           )
            );

          net.add_connection ( we::edge::TP
                             , transition_id
                             , place_id
                             , p.first
                             , we::type::property::type()
                             );

          place_ids.emplace (wrapped_name (p.second), place_id);
        }

        for (const type::activity_t::input_t::value_type& top : activity.input())
        {
          we::type::port_t const& port
            (activity.transition().ports_input().at (top.second));

          net.put_value
            (place_ids.find (wrapped_name (port))->second, top.first);
        }

        //! \todo copy output too

        we::type::transition_t const
          transition_net_wrapper ( wrapped_activity_prefix()
                                 + activity.transition().name()
                                 , net
                                 , boost::none
                                 , true
                                 , we::type::property::type()
                                 , we::priority_type()
                                 );

        return type::activity_t
          (transition_net_wrapper, activity.transition_id());
      }

      type::activity_t unwrap (type::activity_t const& activity)
      {
        we::type::net_type const& net (*activity.transition().net());

        type::activity_t activity_inner
          (net.transitions().begin()->second, activity.transition_id());

        for ( we::type::transition_t::port_map_t::value_type const& p
            : activity_inner.transition().ports_output()
            )
        {
            we::place_id_type const place_id
              ( net.port_to_place().at (net.transitions().begin()->first)
              .left.find (p.first)->get_right()
              );

            for ( const pnet::type::value::value_type& token
                : net.get_token (place_id)
                )
            {
              activity_inner.add_output (p.first, token);
            }
        }

        //! \todo copy input too

        return activity_inner;
      }
    }

    void layer::submit (id_type id, type::activity_t act)
    {
      _nets_to_extract_from.put
        ( activity_data_type ( id
                             , fhg::util::cxx14::make_unique<type::activity_t>
                                 (act.transition().net() ? act : wrap (act))
                             )
        , true
        );
    }

    void layer::finished (id_type id, type::activity_t result)
    {
      boost::optional<id_type> const parent (_running_jobs.parent (id));
      fhg_assert (parent);

      _nets_to_extract_from.apply
        ( *parent
        , [this, parent, result, id] (activity_data_type& activity_data)
        {
          activity_data.child_finished (result);
          _running_jobs.terminated (*parent, id);
        }
        );
    }

    void layer::cancel (id_type id)
    {
      _nets_to_extract_from.remove_and_apply
        ( id
        , [this, id] (activity_data_type const& activity_data)
        {
          const std::function<void()> after
            ([this, id]() { rts_canceled_and_forget (id); });

          //! \note Not a race: nobody can insert new running jobs as
          //! we have ownership of activity
          if (!_running_jobs.contains (activity_data._id))
          {
            after();
          }
          else
          {
            _finalize_job_cancellation.emplace (activity_data._id, after);
            _running_jobs.apply
              (activity_data._id, std::bind (_rts_cancel, std::placeholders::_1));
          }
        }
        );
    }

    void layer::failed (id_type id, std::string reason)
    {
      boost::optional<id_type> const parent (_running_jobs.parent (id));
      fhg_assert (parent);

      _nets_to_extract_from.remove_and_apply
        ( *parent
        , [this, id, reason] (activity_data_type const& parent_activity)
        {
          id_type const parent_activity_id (parent_activity._id);
          const std::function<void()> after
            ([this, parent_activity_id, reason]()
            { rts_failed_and_forget (parent_activity_id, reason); }
            );

          if (_running_jobs.terminated (parent_activity._id, id))
          {
            after();
          }
          else
          {
            _finalize_job_cancellation.emplace (parent_activity._id, after);
            _running_jobs.apply
              (parent_activity._id, std::bind (_rts_cancel, std::placeholders::_1));
          }
        }
        );
    }

    void layer::canceled (id_type child)
    {
      boost::optional<id_type> const parent (_running_jobs.parent (child));
      fhg_assert (parent);

      if (_running_jobs.terminated (*parent, child))
      {
        std::unordered_map<id_type, std::function<void()>>::iterator
          const pos (_finalize_job_cancellation.find (*parent));

        pos->second();
        _finalize_job_cancellation.erase (pos);
      }
    }

    void layer::discover (id_type discover_id, id_type id)
    {
      _nets_to_extract_from.apply
        ( id
        , [this, discover_id] (activity_data_type& activity_data)
        {
          const id_type id (activity_data._id);

          boost::mutex::scoped_lock const _ (_discover_state_mutex);
          fhg_assert (_discover_state.find (discover_id) == _discover_state.end());

          std::pair<std::size_t, sdpa::discovery_info_t > state
            (0, sdpa::discovery_info_t(id, boost::none, sdpa::discovery_info_set_t()));

          _running_jobs.apply ( id
                              , [this, &discover_id, &state] (id_type child)
                              {
                                ++state.first;
                                _rts_discover (discover_id, child);
                              }
                              );

          //! \note Not a race: discovered can't be called in parallel
          if (state.first == std::size_t (0))
          {
            _rts_discovered (discover_id, state.second);
          }
          else
          {
            _discover_state.emplace (discover_id, state);
          }
        }
        );
    }

    void layer::discovered
      (id_type discover_id, sdpa::discovery_info_t result)
    {
      boost::mutex::scoped_lock const _ (_discover_state_mutex);
      fhg_assert (_discover_state.find (discover_id) != _discover_state.end());

      std::pair<std::size_t, sdpa::discovery_info_t >& state
        (_discover_state.find (discover_id)->second);

      state.second.add_child_info (result);

      --state.first;

      if (state.first == std::size_t (0))
      {
        _rts_discovered (discover_id, state.second);

        _discover_state.erase (discover_id);
      }
    }

    void layer::put_token ( id_type id
                          , std::string put_token_id
                          , std::string place_name
                          , pnet::type::value::value_type value
                          )
    {
      _nets_to_extract_from.apply
        ( id
        , [this, put_token_id, place_name, value]
          (activity_data_type& activity_data)
        {
          boost::get<we::type::net_type>
            (activity_data._activity->transition().data())
            .put_token (place_name, value);

          _rts_token_put (put_token_id);
        }
        );
    }

  namespace
  {
    bool output_missing (type::activity_t const& activity)
    {
      if ( activity.output().size()
         < activity.transition().ports_output().size()
         )
      {
        return true;
      }

      std::unordered_set<port_id_type> port_ids_with_output;

      for ( port_id_type port_id : activity.output()
          | boost::adaptors::map_values
          )
      {
        port_ids_with_output.emplace (port_id);
      }

      return port_ids_with_output.size()
        != activity.transition().ports_output().size();
    }
  }

    void layer::extract_from_nets()
    {
      while (true)
      {
        activity_data_type activity_data (_nets_to_extract_from.get());

        bool was_active (false);

        //! \todo How to cancel if the net is inside
        //! fire_expression_and_extract_activity_random (endless loop
        //! in expressions)?

        boost::optional<type::activity_t> activity;
        try
        {
          fhg::util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                //! \note We wrap all input activites in a net.
                activity = boost::get<we::type::net_type>
                  (activity_data._activity->transition().data())
                  . fire_expressions_and_extract_activity_random
                  (_random_extraction_engine);
              }
            , "workflow interpretation"
            );
        }
        catch (...)
        {
          rts_failed_and_forget
            ( activity_data._id
            , fhg::util::current_exception_as_string (": ")
            );
          continue;
        }

        if (activity)
        {
          const id_type child_id (_rts_id_generator());
          _running_jobs.started (activity_data._id, child_id);
          _rts_submit (child_id, *activity);
          was_active = true;
        }

        if (  _running_jobs.contains (activity_data._id)
           || ( boost::get<bool> ( activity_data._activity->transition().prop()
                                 . get ({"drts", "wait_for_output"})
                                 . get_value_or (false)
                                 )
              && output_missing (*activity_data._activity)
              )
           )
        {
          _nets_to_extract_from.put (std::move (activity_data), was_active);
        }
        else
        {
          rts_finished_and_forget
            ( activity_data._id
            , fhg::util::starts_with
              ( wrapped_activity_prefix()
              , activity_data._activity->transition().name()
              )
            ? unwrap (*activity_data._activity)
            : *activity_data._activity
            );
        }
      }
    }

    void layer::rts_finished_and_forget (id_type id, type::activity_t activity)
    {
      _rts_finished (id, activity);
      _nets_to_extract_from.forget (id);
    }
    void layer::rts_failed_and_forget (id_type id, std::string message)
    {
      _rts_failed (id, message);
      _nets_to_extract_from.forget (id);
    }
    void layer::rts_canceled_and_forget (id_type id)
    {
      _rts_canceled (id);
      _nets_to_extract_from.forget (id);
    }


    // list_with_id_lookup

    layer::activity_data_type
      layer::async_remove_queue::list_with_id_lookup::get_front()
    {
      activity_data_type activity_data (std::move (_container.front()));

      _container.pop_front();
      _position_in_container.erase (activity_data._id);

      return activity_data;
    }
    void layer::async_remove_queue::list_with_id_lookup::push_back
      (activity_data_type activity_data)
    {
      //! \note not a temporary but projected out before move
      id_type const activity_data_id (activity_data._id);
      _position_in_container.emplace
        ( activity_data_id
        , _container.insert (_container.end(), std::move (activity_data))
        );
    }
    layer::async_remove_queue::list_with_id_lookup::iterator
      layer::async_remove_queue::list_with_id_lookup::find (id_type id)
    {
      return _position_in_container.find (id);
    }
    layer::async_remove_queue::list_with_id_lookup::iterator
      layer::async_remove_queue::list_with_id_lookup::end()
    {
      return _position_in_container.end();
    }
    void layer::async_remove_queue::list_with_id_lookup::erase (iterator pos)
    {
      _container.erase (pos->second);
      _position_in_container.erase (pos);
    }
    bool layer::async_remove_queue::list_with_id_lookup::empty() const
    {
      return _container.empty();
    }


    // async_remove_queue

    layer::activity_data_type layer::async_remove_queue::get()
    {
      boost::recursive_mutex::scoped_lock lock (_container_mutex);

      _condition_non_empty.wait
        (lock, [this]() -> bool { return !_container.empty(); });

      return _container.get_front();
    }

    void layer::async_remove_queue::put
      (activity_data_type activity_data, bool active)
    {
      boost::recursive_mutex::scoped_lock const _ (_container_mutex);

      bool do_put (true);

      to_be_removed_type::iterator const pos
        (_to_be_removed.find (activity_data._id));

      if (pos != _to_be_removed.end())
      {
        to_be_removed_type::mapped_type::iterator fun_and_do_put_it
          (pos->second.begin());

        while (fun_and_do_put_it != pos->second.end())
        {
          std::pair<std::function<void (activity_data_type&)>, bool>
            fun_and_do_put (*fun_and_do_put_it);
          fun_and_do_put_it = pos->second.erase (fun_and_do_put_it);

          fun_and_do_put.first (activity_data);
          do_put = do_put && (active = fun_and_do_put.second);

          if (!active)
          {
            break;
          }
        }

        if ( _to_be_removed.find (activity_data._id) != _to_be_removed.end()
           && _to_be_removed.find (activity_data._id)->second.empty()
           )
        {
          _to_be_removed.erase (activity_data._id);
        }
      }

      if (do_put)
      {
        if (active)
        {
          _container.push_back (std::move (activity_data));

          _condition_non_empty.notify_one();
        }
        else
        {
          _container_inactive.push_back (std::move (activity_data));
        }
      }
    }

    void layer::async_remove_queue::remove_and_apply
      (id_type id, std::function<void (activity_data_type const&)> fun)
    {
      boost::recursive_mutex::scoped_lock const _ (_container_mutex);

      list_with_id_lookup::iterator const pos_container (_container.find (id));
      list_with_id_lookup::iterator const pos_container_inactive
        (_container_inactive.find (id));

      if (pos_container != _container.end())
      {
        fun (std::move (*pos_container->second));
        _container.erase (pos_container);
      }
      else if (pos_container_inactive != _container_inactive.end())
      {
        fun (std::move (*pos_container_inactive->second));
        _container_inactive.erase (pos_container_inactive);
      }
      else
      {
        _to_be_removed[id].push_back (std::make_pair (fun, false));
      }
    }

    void layer::async_remove_queue::apply
      (id_type id, std::function<void (activity_data_type&)> fun)
    {
      boost::recursive_mutex::scoped_lock const _ (_container_mutex);

      list_with_id_lookup::iterator const pos_container (_container.find (id));
      list_with_id_lookup::iterator const pos_container_inactive
        (_container_inactive.find (id));

      if (pos_container != _container.end())
      {
        fun (*pos_container->second);
      }
      else if (pos_container_inactive != _container_inactive.end())
      {
        activity_data_type activity_data
          (std::move (*pos_container_inactive->second));
        _container_inactive.erase (pos_container_inactive);
        fun (activity_data);
        _container.push_back (std::move (activity_data));

        _condition_non_empty.notify_one();
      }
      else
      {
        _to_be_removed[id].push_back (std::make_pair (fun, true));
      }
    }

    void layer::async_remove_queue::forget (id_type id)
    {
      boost::recursive_mutex::scoped_lock const _ (_container_mutex);

      _to_be_removed.erase (id);
    }



    // activity_data_type

    void layer::activity_data_type::child_finished (type::activity_t child)
    {
      //! \note We wrap all input activites in a net.
      we::type::net_type& net
        (boost::get<we::type::net_type> (_activity->transition().data()));

      for (const type::activity_t::token_on_port_t& top : child.output())
      {
        net.put_value
          ( net.port_to_place().at (*child.transition_id())
          .left.find (top.second)->get_right()
          , top.first
          );
      }
    }


    // locked_parent_child_relation_type

    void layer::locked_parent_child_relation_type::started
      (id_type parent, id_type child)
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      _relation.insert (relation_type::value_type (parent, child));
    }

    bool layer::locked_parent_child_relation_type::terminated
      (id_type parent, id_type child)
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      _relation.erase (relation_type::value_type (parent, child));

      return _relation.left.find (parent) == _relation.left.end();
    }

    boost::optional<layer::id_type>
      layer::locked_parent_child_relation_type::parent (id_type child)
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      relation_type::right_map::const_iterator const pos
        (_relation.right.find (child));

      if (pos != _relation.right.end())
      {
        return pos->second;
      }

      return boost::none;
    }

    bool layer::locked_parent_child_relation_type::contains
      (id_type parent) const
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      return _relation.left.find (parent) != _relation.left.end();
    }

    void layer::locked_parent_child_relation_type::apply
      (id_type parent, std::function<void (id_type)> fun) const
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      for ( id_type child
          : _relation.left.equal_range (parent) | boost::adaptors::map_values
          )
      {
        fun (child);
      }
    }
}
