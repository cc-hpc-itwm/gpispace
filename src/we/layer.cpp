// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/layer.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/read_bool.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_exception.hpp>

#include <algorithm>
#include <exception>
#include <functional>
#include <stdexcept>

namespace we
{
    layer::layer
        ( std::function<void (id_type, type::Activity)> rts_submit
        , std::function<void (id_type)> rts_cancel
        , std::function<void (id_type, type::Activity)> rts_finished
        , std::function<void (id_type, std::string)> rts_failed
        , std::function<void (id_type)> rts_canceled
        , std::function<void (std::string, ::boost::optional<std::exception_ptr>)> rts_token_put
        , std::function<void (std::string workflow_response_id, std::variant<std::exception_ptr, pnet::type::value::value_type>)> rts_workflow_response
        , std::function<id_type()> rts_id_generator
        , std::mt19937& random_extraction_engine
        )
      : _rts_submit (rts_submit)
      , _rts_cancel (rts_cancel)
      , _rts_finished (rts_finished)
      , _rts_failed (rts_failed)
      , _rts_canceled (rts_canceled)
      , _rts_token_put (rts_token_put)
      , _rts_workflow_response (rts_workflow_response)
      , _rts_id_generator (rts_id_generator)
      , _random_extraction_engine (random_extraction_engine)
      , _extract_from_nets_thread (&layer::extract_from_nets, this)
      , _stop_extracting ([this] { _nets_to_extract_from.interrupt(); })
    {}

    void layer::submit (id_type id, type::Activity act)
    {
      _nets_to_extract_from.put
        ( activity_data_type ( id
                             , std::make_unique<type::Activity>
                                 (std::move (act).wrap())
                             )
        , true
        );
    }

    void layer::finished (id_type id, type::Activity result)
    {
      ::boost::optional<id_type> const parent (_running_jobs.parent (id));
      fhg_assert (parent);

      //! \todo Don't forget that this child actually finished and
      //! inject result.
      if (_finalize_job_cancellation.count (*parent))
      {
        canceled (id);
        return;
      }

      _nets_to_extract_from.apply
        ( *parent
        , async_remove_queue::RemovalFunction::ToFinish
            (this, *parent, std::move (result), id)
        , &std::rethrow_exception
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
      ::boost::optional<id_type> const parent (_running_jobs.parent (id));
      fhg_assert (parent);

      //! \todo Don't forget that this child actually failed and
      //! store reason.
      if (  _finalize_job_cancellation.count (*parent)
         || _ignore_canceled_by_eureka.count (id)
         )
      {
        canceled (id);
        return;
      }

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
      ::boost::optional<id_type> const parent (_running_jobs.parent (child));

      if (!parent)
      {
        return;
      }

      if (_ignore_canceled_by_eureka.erase (child))
      {
        _nets_to_extract_from.apply
          ( *parent
          , [this, child] (activity_data_type& activity_data)
          {
            id_type const parent_id (activity_data._id);
            _running_jobs.terminated (parent_id, child);
          }
          , &std::rethrow_exception
          );

        return;
      }

      if (_running_jobs.terminated (*parent, child))
      {
        std::unordered_map<id_type, std::function<void()>>::iterator
          const pos (_finalize_job_cancellation.find (*parent));

        pos->second();
        _finalize_job_cancellation.erase (pos);
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
          activity_data._activity->put_token (place_name, value);

          _rts_token_put (put_token_id, ::boost::none);
        }
        , std::bind (_rts_token_put, put_token_id, std::placeholders::_1)
        );
    }

  void layer::request_workflow_response
    ( id_type id
    , std::string workflow_response_id
    , std::string place_name
    , pnet::type::value::value_type value
    )
  {
    {
      std::lock_guard<std::mutex> const _ (_outstanding_responses_guard);
      _outstanding_responses[id].emplace (workflow_response_id);
    }
    _nets_to_extract_from.apply
      ( id
      , [workflow_response_id, place_name, value]
          (activity_data_type& activity_data)
        {
           activity_data._activity->
             put_token ( place_name
                       , make_response_description
                           (workflow_response_id, value)
                       );
        }
      , [this, id, workflow_response_id] (std::exception_ptr error)
        {
          workflow_response (id, workflow_response_id, error);
        }
      );
  }

  void layer::workflow_response
    ( id_type id
    , std::string const& response_id
    , std::variant<std::exception_ptr, pnet::type::value::value_type> const& response
    )
  {
    _rts_workflow_response (response_id, response);
    std::lock_guard<std::mutex> const _ (_outstanding_responses_guard);
    _outstanding_responses.at (id).erase (response_id);
    if (_outstanding_responses.at (id).empty())
    {
      _outstanding_responses.erase (id);
    }
  }

  void layer::eureka_response ( id_type parent
                              , ::boost::optional<id_type> eureka_calling_child
                              , type::eureka_ids_type const& eureka_ids
                              )
  {
    _nets_to_extract_from.apply
      ( parent
      , [this, parent, eureka_calling_child, eureka_ids]
          (activity_data_type const& activity_data)
        {
          if (_running_jobs.contains (activity_data._id))
          {
            for (type::eureka_id_type const& eureka_id : eureka_ids)
            {
              _running_jobs.apply_and_remove_eureka
                ( eureka_id
                , parent
                , eureka_calling_child
                , [&] (id_type t_id)
                  {
                    _ignore_canceled_by_eureka.emplace (t_id);
                    _rts_cancel (t_id);
                  }
                );
            }
          }
        }
      , &std::rethrow_exception
      );
  }

    void layer::extract_from_nets()
    try
    {
      while (true)
      {
        activity_data_type activity_data (_nets_to_extract_from.get());
        auto const id (activity_data._id);

        bool was_active (false);

        //! \todo How to cancel if the net is inside
        //! fire_expression_and_extract_activity_random (endless loop
        //! in expressions)?

        ::boost::optional<type::Activity> activity;
        try
        {
          try
          {
            activity = activity_data._activity->extract
                  ( _random_extraction_engine
                  , [this, &activity_data] ( pnet::type::value::value_type const& description
                                           , pnet::type::value::value_type const& value
                                           )
                    {
                      workflow_response ( activity_data._id
                                        , get_response_id (description)
                                        , value
                                        );
                    }
                  , [this, &activity_data] (type::eureka_ids_type const& eureka_ids)
                    {
                      eureka_response ( activity_data._id
                                      , ::boost::none
                                      , eureka_ids
                                      );
                    }
                  , _plugins
                  , [this, id]
                      ( std::string place_name
                      , pnet::type::value::value_type value
                      )
                    {
                      _nets_to_extract_from.apply
                        ( id
                        , [place_name, value]
                            (activity_data_type& ad)
                          {
                            ad._activity->put_token (place_name, value);
                          }
                        , &std::rethrow_exception
                        );
                    }
                  );
          }
          catch (...)
          {
            std::throw_with_nested
              (std::runtime_error {"workflow interpretation"});
          }
        }
        catch (...)
        {
          rts_failed_and_forget
            ( activity_data._id
            , fhg::util::current_exception_printer (": ").string()
            );
          continue;
        }

        if (activity)
        {
          const id_type child_id (_rts_id_generator());
          try
          {
            _running_jobs.started ( activity_data._id
                                  , child_id
                                  , activity->eureka_id()
                                  );
          }
          catch (...)
          {
            rts_failed_and_forget
              ( activity_data._id
              , fhg::util::current_exception_printer (": ").string()
              );
            continue;
          }
          _rts_submit (child_id, std::move (*activity));
          was_active = true;
        }

        if (  _running_jobs.contains (activity_data._id)
           || activity_data._activity->wait_for_output()
           )
        {
          id_type const id_ (activity_data._id);

          try
          {
            try
            {
              _nets_to_extract_from.put
                (std::move (activity_data), was_active);
            }
            catch (...)
            {
              std::throw_with_nested
                ( std::runtime_error
                  { "waiting for further events to continue workflow interpretation"
                  }
                );
            }
          }
          catch (...)
          {
            rts_failed_and_forget
              ( id_
              , fhg::util::current_exception_printer (": ").string()
              );
          }
        }
        else
        {
          rts_finished_and_forget
            (activity_data._id, std::move (*activity_data._activity).unwrap());
        }
      }
    }
    catch (async_remove_queue::interrupted const&)
    {
    }

    void layer::rts_finished_and_forget (id_type id, type::Activity activity)
    {
      _nets_to_extract_from.forget (id, "workflow finished");
      cancel_outstanding_responses (id, "workflow finished");
      _rts_finished (id, std::move (activity));
    }
    void layer::rts_failed_and_forget (id_type id, std::string message)
    {
      _nets_to_extract_from.forget (id, message);
      cancel_outstanding_responses (id, message);
      _rts_failed (id, message);
    }
    void layer::rts_canceled_and_forget (id_type id)
    {
      _nets_to_extract_from.forget (id, "workflow was canceled");
      cancel_outstanding_responses (id, "workflow was canceled");
      _rts_canceled (id);
    }

  void layer::cancel_outstanding_responses
    (id_type id, std::string const& reason)
  {
    std::lock_guard<std::mutex> const _ (_outstanding_responses_guard);
    auto responses (_outstanding_responses.find (id));
    if (responses != _outstanding_responses.end())
    {
      for (std::string const& response_id : responses->second)
      {
        _rts_workflow_response
          ( response_id
          , std::make_exception_ptr (std::runtime_error (reason))
          );
      }

      _outstanding_responses.erase (responses);
    }
  }

    // list_with_id_lookup

  void layer::async_remove_queue::RemovalFunction::operator()
    (activity_data_type& activity_data) &&
  {
    fhg::util::visit
      ( _function
      , [&] (std::function<void (activity_data_type&)> const& fun)
        {
          return fun (activity_data);
        }
      , [&] (ToFinish const& to_finish)
        {
          activity_data.child_finished
            ( std::move (to_finish._result)
            , [&] ( pnet::type::value::value_type const& description
                  , pnet::type::value::value_type const& value
                  )
              {
                to_finish._that->workflow_response
                  ( to_finish._parent
                  , get_response_id (description)
                  , value
                  );
              }
            , [&] (type::eureka_ids_type const& eureka_ids)
              {
                to_finish._that->eureka_response
                  ( to_finish._parent
                  , to_finish._id
                  , eureka_ids
                  );
              }
            );
          to_finish._that->_running_jobs.terminated
            ( to_finish._parent
            , to_finish._id
            );
        }
      );
  }
  template<typename Fun>
  layer::async_remove_queue::RemovalFunction::RemovalFunction
    (Callback, Fun&& fun)
      : _function (std::forward<Fun> (fun))
  {}
  layer::async_remove_queue::RemovalFunction::RemovalFunction
    (ToFinish to_finish)
      : _function (std::move (to_finish))
  {}
  layer::async_remove_queue::RemovalFunction::ToFinish::ToFinish
    (layer* that, id_type parent, type::Activity result, id_type id)
      : _that (that)
      , _parent (std::move (parent))
      , _result (std::move (result))
      , _id (std::move (id))
  {}

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
      std::unique_lock<std::recursive_mutex> lock (_container_mutex);

      _condition_not_empty_or_interrupted.wait
        (lock, [this] { return !_container.empty() || _interrupted; });

      if (_interrupted)
      {
        throw interrupted();
      }

      return _container.get_front();
    }

    void layer::async_remove_queue::put
      (activity_data_type activity_data, bool active)
    {
      std::lock_guard<std::recursive_mutex> const _ (_container_mutex);

      bool do_put (true);

      to_be_removed_type::iterator const pos
        (_to_be_removed.find (activity_data._id));

      if (pos != _to_be_removed.end())
      {
        auto fun_and_do_put_it
          (pos->second.begin());

        while (fun_and_do_put_it != pos->second.end())
        {
          auto fun_and_do_put (std::move (*fun_and_do_put_it));
          fun_and_do_put_it = pos->second.erase (fun_and_do_put_it);

          try
          {
            std::move (std::get<0> (fun_and_do_put)) (activity_data);
          }
          catch (...)
          {
            std::get<1> (fun_and_do_put) (std::current_exception());
          }
          do_put = do_put && (active = std::get<2> (fun_and_do_put));

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

          _condition_not_empty_or_interrupted.notify_one();
        }
        else
        {
          _container_inactive.push_back (std::move (activity_data));
        }
      }
    }

  template<typename Fun>
    void layer::async_remove_queue::remove_and_apply
      ( id_type id
      , Fun&& fun
      )
    {
      std::lock_guard<std::recursive_mutex> const _ (_container_mutex);

      list_with_id_lookup::iterator const pos_container (_container.find (id));
      list_with_id_lookup::iterator const pos_container_inactive
        (_container_inactive.find (id));

      if (pos_container != _container.end())
      {
        try
        {
          std::forward<Fun> (fun) (*pos_container->second);
        }
        catch (...)
        {
          std::rethrow_exception (std::current_exception());
        }
        _container.erase (pos_container);
      }
      else if (pos_container_inactive != _container_inactive.end())
      {
        try
        {
          std::forward<Fun> (fun) (*pos_container_inactive->second);
        }
        catch (...)
        {
          std::rethrow_exception (std::current_exception());
        }
        _container_inactive.erase (pos_container_inactive);
      }
      else
      {
        _to_be_removed[id].emplace_back
          ( RemovalFunction { RemovalFunction::Callback{}
                            , std::forward<Fun> (fun)
                            }
          , &std::rethrow_exception
          , false
          );
      }
    }

  template<typename Fun>
    void layer::async_remove_queue::apply
      ( id_type id
      , Fun&& fun
      , std::function<void (std::exception_ptr)> on_error
      )
  {
    return apply
      ( id
      , RemovalFunction { RemovalFunction::Callback{}
                        , std::forward<Fun> (fun)
                        }
      , std::move (on_error)
      );
  }

    void layer::async_remove_queue::apply
      ( id_type id
      , RemovalFunction fun
      , std::function<void (std::exception_ptr)> on_error
      )
    {
      std::lock_guard<std::recursive_mutex> const _ (_container_mutex);

      list_with_id_lookup::iterator const pos_container (_container.find (id));
      list_with_id_lookup::iterator const pos_container_inactive
        (_container_inactive.find (id));

      if (pos_container != _container.end())
      {
        try
        {
          std::move (fun) (*pos_container->second);
        }
        catch (...)
        {
          on_error (std::current_exception());
        }
      }
      else if (pos_container_inactive != _container_inactive.end())
      {
        activity_data_type activity_data
          (std::move (*pos_container_inactive->second));
        _container_inactive.erase (pos_container_inactive);
        try
        {
          std::move (fun) (activity_data);
        }
        catch (...)
        {
          on_error (std::current_exception());
        }
        _container.push_back (std::move (activity_data));

        _condition_not_empty_or_interrupted.notify_one();
      }
      else
      {
        _to_be_removed[id].emplace_back (std::move (fun), on_error, true);
      }
    }

    void layer::async_remove_queue::forget (id_type id, std::string reason)
    {
      std::lock_guard<std::recursive_mutex> const _ (_container_mutex);

      to_be_removed_type::iterator const pos
        (_to_be_removed.find (id));

      if (pos != _to_be_removed.end())
      {
        for (auto const& info : pos->second)
        {
          try
          {
            std::get<1> (info)
              ( std::make_exception_ptr
                  (std::runtime_error (reason))
              );
          }
          catch (...)
          {
            //! \todo instead have two handlers? before, on_error was
            //! always doing nothing on forget, but throwing on
            //! applying. to keep applying, most functions use
            //! std::rethrow_exception, which would break forget()
          }
        }

        _to_be_removed.erase (pos);
      }
    }

    void layer::async_remove_queue::interrupt()
    {
      std::lock_guard<std::recursive_mutex> const _ (_container_mutex);

      _interrupted = true;
      _condition_not_empty_or_interrupted.notify_all();
    }

    // activity_data_type

    void layer::activity_data_type::child_finished
      ( type::Activity child
      , we::workflow_response_callback const& workflow_response
      , we::eureka_response_callback const& eureka_response
      )
    {
      _activity->inject (child, workflow_response, eureka_response);
    }


    // locked_parent_child_relation_type

    void layer::locked_parent_child_relation_type::started
      ( id_type parent
      , id_type child
      , ::boost::optional<type::eureka_id_type> const& eureka_id
      )
    {
      std::lock_guard<std::mutex> const _ (_relation_mutex);

      if (eureka_id)
      {
        _eureka_in_progress.insert
          ({eureka_parent_id_type (*eureka_id, parent), child});
      }

      _relation.insert (relation_type::value_type (parent, child));
    }

    bool layer::locked_parent_child_relation_type::terminated
      (id_type parent, id_type child)
    {
      std::lock_guard<std::mutex> const _ (_relation_mutex);

      _eureka_in_progress.right.erase (child);

      _relation.erase (relation_type::value_type (parent, child));

      return _relation.left.find (parent) == _relation.left.end();
    }

    ::boost::optional<layer::id_type>
      layer::locked_parent_child_relation_type::parent (id_type child)
    {
      std::lock_guard<std::mutex> const _ (_relation_mutex);

      relation_type::right_map::const_iterator const pos
        (_relation.right.find (child));

      if (pos != _relation.right.end())
      {
        return pos->second;
      }

      return ::boost::none;
    }

    bool layer::locked_parent_child_relation_type::contains
      (id_type parent) const
    {
      std::lock_guard<std::mutex> const _ (_relation_mutex);

      return _relation.left.find (parent) != _relation.left.end();
    }

    void layer::locked_parent_child_relation_type::apply
      (id_type parent, std::function<void (id_type)> fun) const
    {
      std::lock_guard<std::mutex> const _ (_relation_mutex);

      for ( auto [child, end] {_relation.left.equal_range (parent)}
          ; child != end
          ; ++child
          )
      {
        fun (child->second);
      }
    }


    void layer::locked_parent_child_relation_type::apply_and_remove_eureka
      ( type::eureka_id_type const& eureka_id
      , id_type const& parent
      , ::boost::optional<id_type> const& eureka_caller
      , std::function<void (id_type)> cancel
      )
    {
      std::lock_guard<std::mutex> const lock_for_eureka (_relation_mutex);

      eureka_parent_id_type const eureka_by_parent (eureka_id, parent);

      for ( auto [child, end]
              {_eureka_in_progress.left.equal_range (eureka_by_parent)}
          ; child != end
          ; ++child
          )
      {
        if (eureka_caller != child->second)
        {
          cancel (child->second);
        }
      }

      _eureka_in_progress.left.erase (eureka_by_parent);
    }
}
