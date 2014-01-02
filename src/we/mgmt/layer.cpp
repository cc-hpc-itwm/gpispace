// mirko.rahn@itwm.fraunhofer.de

#include <we/mgmt/layer.hpp>

#include <boost/range/adaptor/map.hpp>

namespace we
{
  namespace mgmt
  {
    layer::layer
        ( boost::function<void (id_type, type::activity_t, id_type)> rts_submit
        , boost::function<void (id_type, reason_type)> rts_cancel
        , boost::function<void (id_type, type::activity_t)> rts_finished
        , boost::function<void (id_type, int, std::string)> rts_failed
        , boost::function<void (id_type)> rts_canceled
        , boost::function<id_type()> rts_id_generator
        )
      : _rts_submit (rts_submit)
      , _rts_cancel (rts_cancel)
      , _rts_finished (rts_finished)
      , _rts_failed (rts_failed)
      , _rts_canceled (rts_canceled)
      , _rts_id_generator (rts_id_generator)
      , _extract_from_nets_thread (&layer::extract_from_nets, this)
    {}
    layer::~layer()
    {
      _extract_from_nets_thread.interrupt();
      if (_extract_from_nets_thread.joinable())
      {
        _extract_from_nets_thread.join();
      }
    }

    namespace
    {
      type::activity_t wrap (type::activity_t const& act)
      {
        petri_net::net net;

        petri_net::transition_id_type const transition_id
          (net.add_transition (act.transition()));

        boost::unordered_map<std::string, petri_net::place_id_type> place_ids;

        BOOST_FOREACH
          ( we::type::port_t const& port
          , act.transition().ports() | boost::adaptors::map_values
          )
        {
          petri_net::place_id_type const place_id
            (net.add_place (place::type (port.name(), port.signature())));

          net.add_connection
            ( petri_net::connection_t
              ( port.is_output() ? petri_net::edge::TP
              : port.is_input() ? petri_net::edge::PT
              : throw std::runtime_error ("tried to wrap, found tunnel port!?")
              , transition_id
              , place_id
              )
            );

          place_ids.insert (std::make_pair (port.name(), place_id));
        }

        we::type::transition_t
          transition_net_wrapper ( "_wrapper_for_" + act.transition().name()
                                 , net
                                 , condition::type ("true")
                                 , true
                                 , we::type::property::type()
                                 );

        BOOST_FOREACH
          ( we::type::port_t const& port
          , act.transition().ports() | boost::adaptors::map_values
          )
        {
          transition_net_wrapper.add_port
            ( we::type::port_t ( port.name()
                               , port.direction()
                               , port.signature()
                               , port.property()
                               )
            );

          if (port.is_output())
          {
            transition_net_wrapper.add_connection
              ( place_ids.find (port.name())->second
              , port.name()
              , we::type::property::type()
              );
          }
          else if (port.is_input())
          {
            transition_net_wrapper.add_connection
              ( port.name()
              , place_ids.find (port.name())->second
              , we::type::property::type()
              );
          }
          else
          {
            throw std::runtime_error ("tried to wrap, found tunnel port!?");
          }
        }

        return type::activity_t (transition_net_wrapper);
      }
    }

    void layer::submit (id_type id, type::activity_t act)
    {
      _nets_to_extract_from.put
        ( activity_data_type (id, act.transition().net() ? act : wrap (act))
        , true
        );
    }

    void layer::finished (id_type id, type::activity_t result)
    {
      boost::optional<id_type> const parent (_running_jobs.parent (id));
      assert (parent);

      _nets_to_extract_from.apply
        ( *parent
        , boost::bind
          (&layer::finalize_finished, this, _1, result, *parent, id)
        );
    }
    void layer::finalize_finished ( activity_data_type& activity_data
                                  , type::activity_t result
                                  , id_type parent
                                  , id_type child
                                  )
    {
      activity_data.child_finished (result);
      _running_jobs.terminated (parent, child);
    }

    void layer::cancel (id_type id, reason_type reason)
    {
      request_cancel (id, boost::bind (_rts_canceled, id), reason);
    }

    void layer::failed (id_type id, int error_code, std::string reason)
    {
      boost::optional<id_type> const parent (_running_jobs.parent (id));
      assert (parent);

      _running_jobs.terminated (*parent, id);

      request_cancel
        ( *parent
        , boost::bind (_rts_failed, *parent, error_code, reason)
        , reason
        );
    }

    void layer::request_cancel
      (id_type id, boost::function<void()> after, reason_type reason)
    {
      _nets_to_extract_from.remove_and_apply
        (id, boost::bind (&layer::cancel_child_jobs, this, _1, after, reason));
    }

    void layer::cancel_child_jobs ( activity_data_type activity_data
                                  , boost::function<void()> after
                                  , reason_type reason
                                  )
    {
      if (!_running_jobs.contains (activity_data._id))
      {
        after();
      }
      else
      {
        _finalize_job_cancellation.insert
          (std::make_pair (activity_data._id, after));
        _running_jobs.apply
          (activity_data._id, boost::bind (_rts_cancel, _1, reason));
      }
    }

    void layer::canceled (id_type child)
    {
      boost::optional<id_type> const parent (_running_jobs.parent (child));
      assert (parent);

      if (_running_jobs.terminated (*parent, child))
      {
        boost::unordered_map<id_type, boost::function<void()> >::iterator
          const pos (_finalize_job_cancellation.find (*parent));

        pos->second();
        _finalize_job_cancellation.erase (pos);
      }
    }

    void layer::extract_from_nets()
    {
      while (true)
      {
        activity_data_type activity_data (_nets_to_extract_from.get());

        bool was_active (false);

        //! \todo How to cancel if the net is inside
        //! fire_internally_and_extract_external (endless loop in
        //! expressions)?
        if ( boost::optional<type::activity_t> activity
           = activity_data.fire_internally_and_extract_external()
           )
        {
          const id_type child_id (_rts_id_generator());
          _running_jobs.started (activity_data._id, child_id);
          _rts_submit (child_id, *activity, activity_data._id);
          was_active = true;
        }

        if (_running_jobs.contains (activity_data._id))
        {
          _nets_to_extract_from.put (activity_data, was_active);
        }
        else
        {
          _rts_finished (activity_data._id, activity_data._activity);
        }
      }
    }


    // async_remove_queue

    layer::activity_data_type layer::async_remove_queue::get()
    {
      boost::mutex::scoped_lock lock (_container_mutex);

      _condition_non_empty.wait
        ( lock
        , not boost::bind (&std::list<activity_data_type>::empty, &_container)
        );

      activity_data_type activity_data (_container.front());

      _container.pop_front();

      return activity_data;
    }

    void layer::async_remove_queue::put
      (activity_data_type activity_data, bool active)
    {
      boost::optional<boost::function<void (activity_data_type)> > fun;

      {
        boost::mutex::scoped_lock const _ (_to_be_removed_mutex);

        to_be_removed_type::iterator const pos
          (_to_be_removed.find (activity_data._id));

        if (pos != _to_be_removed.end())
        {
          fun = pos->second;
          _to_be_removed.erase (pos);
        }
      }

      if (fun)
      {
        (*fun) (activity_data);
      }
      else
      {
        boost::mutex::scoped_lock const _ (_container_mutex);

        if (active)
        {
          _container.push_back (activity_data);

          _condition_non_empty.notify_one();
        }
        else
        {
          _container_inactive.push_back (activity_data);
        }
      }
    }

    void layer::async_remove_queue::remove_and_apply
      (id_type id, boost::function<void (activity_data_type)> fun)
    {
      boost::mutex::scoped_lock const container_lock (_container_mutex);
      boost::mutex::scoped_lock const to_be_removed_lock (_to_be_removed_mutex);

      std::list<activity_data_type>::iterator pos
        ( std::find_if ( _container.begin(), _container.end()
                       , boost::bind (&activity_data_type::_id, _1) == id
                       )
        );

      std::list<activity_data_type>::iterator pos_container_inactive
        ( std::find_if ( _container_inactive.begin(), _container_inactive.end()
                       , boost::bind (&activity_data_type::_id, _1) == id
                       )
        );

      if (pos != _container.end())
      {
        fun (*pos);
        _container.erase (pos);
      }
      else if (pos_container_inactive != _container_inactive.end())
      {
        fun (*pos_container_inactive);
        _container_inactive.erase (pos_container_inactive);
      }
      else
      {
        _to_be_removed.insert (std::make_pair (id, fun));
      }
    }

    void layer::async_remove_queue::apply
      (id_type id, boost::function<void (activity_data_type&)> fun)
    {
      boost::mutex::scoped_lock const container_lock (_container_mutex);
      boost::mutex::scoped_lock const to_be_removed_lock (_to_be_removed_mutex);

      std::list<activity_data_type>::iterator pos
        ( std::find_if ( _container.begin(), _container.end()
                       , boost::bind (&activity_data_type::_id, _1) == id
                       )
        );

      std::list<activity_data_type>::iterator pos_container_inactive
        ( std::find_if ( _container_inactive.begin(), _container_inactive.end()
                       , boost::bind (&activity_data_type::_id, _1) == id
                       )
        );

      if (pos != _container.end())
      {
        fun (*pos);
      }
      else if (pos_container_inactive != _container_inactive.end())
      {
        activity_data_type data (*pos_container_inactive);
        _container_inactive.erase (pos_container_inactive);
        fun (data);
        _container.push_back (data);

        _condition_non_empty.notify_one();
      }
      else
      {
        _to_be_removed.insert
          ( std::make_pair
            ( id
            , boost::bind (&async_remove_queue::apply_callback, this, _1, fun)
            )
          );
      }
    }
    void layer::async_remove_queue::apply_callback
      ( activity_data_type activity_data
      , boost::function<void (activity_data_type&)> fun
      )
    {
      fun (activity_data);
      put (activity_data, true);
    }


    // activity_data_type

    boost::optional<type::activity_t>
      layer::activity_data_type::fire_internally_and_extract_external()
    {
      //! \note We wrap all input activites in a net.
      petri_net::net& net
        (boost::get<petri_net::net&> (_activity.transition().data()));

      while (net.can_fire())
      {
        type::activity_t activity
          (net.extract_activity_random (_random_extraction_engine));

        if (!activity.transition().expression())
        {
          return activity;
        }
        else
        {
          expr::eval::context context;

          BOOST_FOREACH ( const type::activity_t::input_t::value_type& top
                        , activity.input()
                        )
          {
            context.bind_ref
              ( activity.transition().get_port (top.second).name()
              , top.first
              );
          }

          activity.transition().expression()->ast().eval_all (context);

          BOOST_FOREACH
            ( we::type::transition_t::port_map_t::value_type const& p
            , activity.transition().ports()
            )
          {
            if (p.second.is_output())
            {
              net.put_value
                ( activity.transition().inner_to_outer().at (p.first).first
                , context.value (p.second.name())
                );
            }
          }
        }
      }

      return boost::none;
    }

    void layer::activity_data_type::child_finished (type::activity_t child)
    {
      //! \note We wrap all input activites in a net.
      petri_net::net& net
        (boost::get<petri_net::net&> (_activity.transition().data()));

      {
        BOOST_FOREACH ( const type::activity_t::token_on_port_t& top
                      , child.output()
                      )
        {
          net.put_value
            ( child.transition().inner_to_outer().at (top.second).first
            , top.first
            );
        }
      }
    }


    // locked_parent_child_relation_type

    void layer::locked_parent_child_relation_type::started
      (id_type parent, id_type child)
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      _relation[parent].insert (child);
    }

    bool layer::locked_parent_child_relation_type::terminated
      (id_type parent, id_type child)
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      relation_type::iterator const childs (_relation.find (parent));

      childs->second.erase (child);

      if (childs->second.empty())
      {
        _relation.erase (childs);

        return true;
      }

      return false;
    }

    boost::optional<layer::id_type>
      layer::locked_parent_child_relation_type::parent (id_type child)
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      BOOST_FOREACH (relation_type::value_type const& entry, _relation)
      {
        if (entry.second.count (child))
        {
          return entry.first;
        }
      }

      return boost::none;
    }

    bool layer::locked_parent_child_relation_type::contains
      (id_type parent) const
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      return _relation.find (parent) != _relation.end();
    }

    void layer::locked_parent_child_relation_type::apply
      (id_type parent, boost::function<void (id_type)> fun) const
    {
      boost::mutex::scoped_lock const _ (_relation_mutex);

      BOOST_FOREACH (id_type child, _relation.find (parent)->second)
      {
        fun (child);
      }
    }
  }
}
