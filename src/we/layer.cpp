// mirko.rahn@itwm.fraunhofer.de

#include <we/layer.hpp>
#include <we/type/value/show.hpp>

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
          . at (p.first).first
          );

        for ( const pnet::type::value::value_type& token
            : net.get_token (place_id) | boost::adaptors::map_values
            )
        {
          activity_inner.add_output (p.first, token);
        }
      }

      //! \todo copy input too
      return activity_inner;
    }
  }


  layer::layer
        ( std::function<void (id_type, type::activity_t)> rts_submit
        , std::function<void (id_type)> rts_cancel
        , std::function<void (sdpa::finished_reason_t const&)> rts_finished
        , std::function<void (std::string, boost::optional<std::exception_ptr>)> rts_token_put
        , std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> rts_workflow_response
        , std::function<id_type()> rts_id_generator
        , std::mt19937& random_extraction_engine
        , const type::activity_t& wf
        )
      : _rts_submit (rts_submit)
      , _rts_cancel (rts_cancel)
      , _rts_finished (rts_finished)
      , _rts_token_put (rts_token_put)
      , _rts_workflow_response (rts_workflow_response)
      , _rts_id_generator (rts_id_generator)
      , _random_extraction_engine (random_extraction_engine)
      , _workflow (wf.transition().net() ? wf : wrap (wf))
      , _extract_from_nets_thread (&layer::extract_from_nets, this)
      , _wf_state(RUNNING)
    {}


  void layer::finished_correctly(id_type id, sdpa::task_completed_reason_t const& result)
  {
    try {
      _command_queue.put
        ( [this, id, result] ()
        {
          _running_tasks.erase(id);

          boost::get<we::type::net_type>(_workflow.transition().data()).inject
          ( result
          , [this] ( pnet::type::value::value_type const& description
                   , pnet::type::value::value_type const& value
                   )
            {
              workflow_response (get_response_id (description), value);
            }
          );
        }
    );
    }
    catch (queue_interrupted const&)
    {
      if (_wf_state == RUNNING)
      {
        // internal error
        throw std::logic_error ( "task finished after the workflow has finished");
      }

      // ignore task finished after the workflow failed/was canceled
    }
    catch (...)
    {
      throw std::runtime_error ("error in layer::finished_correctly");
    }
  }

  void layer::finished_failure(id_type id, sdpa::task_failed_reason_t const& error)
  {
    try
    {
      _command_queue.put
        ( [this, id, error] ()
        {
          _running_tasks.erase(id);
          if (_wf_state != CANCELED)
          {
            _error = error;
            _wf_state = ERROR;
            cancel_remaining_tasks();
          }
        }
        );
    }
    catch (queue_interrupted const&)
    {
      if (_wf_state == RUNNING)
      {
        // internal error
        throw std::logic_error ( "task failed after the workflow has finished");
      }

      // ignore task failed after the workflow failed/was canceled
    }
    catch (...)
    {
      throw std::runtime_error ("error in layer::finished_failure");
    }
  }

  void layer::finished_canceled(id_type id, sdpa::task_canceled_reason_t const& reason)
  {
    try
    {
      _command_queue.put
        ( [this, id, reason] ()
        {
          _running_tasks.erase(id);
        }
        );
    }
    catch (queue_interrupted const&)
    {
      if (_wf_state == RUNNING)
      {
        // internal error
        throw std::logic_error ( "task canceled after the workflow has finished");
      }
      // ignore task canceled after the workflow failed/was canceled
    }
    catch (...)
    {
      throw std::runtime_error ("error in layer::canceled");
    }
  }

  void layer::finished (id_type id, sdpa::finished_reason_t reason)
  {
    struct
    {
      using result_type = void;
      void operator() (sdpa::task_completed_reason_t const& result) const
      {
        _this->finished_correctly(_id, result);
      }
      void operator() (sdpa::task_failed_reason_t const& error) const
      {
        _this->finished_failure(_id, error);
      }
      void operator() (sdpa::task_canceled_reason_t const& error) const
      {
        _this->finished_canceled(_id, error);
      }

      layer* _this;
      id_type _id;
    } visitor = {this, id};
    boost::apply_visitor (visitor, reason);
  }

  void layer::cancel (id_type)
  {
    try
    {
      _command_queue.put
        ( [this] ()
        {
          if (_wf_state != ERROR) // nothing to do if workflow already failed
          {
            _wf_state = CANCELED;
            cancel_remaining_tasks();
          }
        }
        );
    }
    catch (queue_interrupted const&)
    {
      if (_wf_state == RUNNING)
      {
        // logic error
        throw std::logic_error ("cancel called after the workflow has finished");
      }
      // ignore task cancel after the workflow failed/was canceled
    }
    catch (...)
    {
      throw std::runtime_error ("error in layer::cancel");
    }
  }

  void layer::request_workflow_response
    ( id_type
    , std::string workflow_response_id
    , std::string place_name
    , pnet::type::value::value_type value
    )
  {
    try
    {
      _command_queue.put
        ( [this, workflow_response_id, place_name, value] ()
        {
          _outstanding_responses.emplace(workflow_response_id);
          if (_wf_state == RUNNING)
          {
            try
            {
              boost::get<we::type::net_type>(_workflow.transition().data()).put_token
                  ( place_name
                  , make_response_description(workflow_response_id, value)
                  );
            }
            catch (...)
            {
              workflow_response ( workflow_response_id, std::current_exception());
            }
          }
          else
          {
            workflow_response ( workflow_response_id, "workflow failed");
          }
        }
        );
    }
    catch (queue_interrupted const&)
    {
      // ill-behaved client
      throw std::logic_error ("response request after workflow finished");
    }
    catch (...)
    {
      _rts_workflow_response ( workflow_response_id, "workflow failed");
    }
  }

  void layer::put_token ( id_type
                        , std::string put_token_id
                        , std::string place_name
                        , pnet::type::value::value_type value
                        )
  {
    try
    {
      _command_queue.put
        ( [this, put_token_id, place_name, value] ()
        {
          boost::get<we::type::net_type>( _workflow.transition().data())
              .put_token (place_name, value);
          _rts_token_put (put_token_id, boost::none);
        }
        );
    }
    catch (queue_interrupted const&)
    {
      // ill-behaved client
      throw std::logic_error ("put token after workflow finished");
    }
    catch (...)
    {
      throw std::runtime_error ("error in put_token");
    }
  }

  void layer::workflow_response
    ( std::string const& response_id
    , boost::variant<std::exception_ptr, pnet::type::value::value_type> const& response
    )
  {
    _rts_workflow_response (response_id, response);
    _outstanding_responses.erase (response_id);
  }

  void layer::cancel_outstanding_responses
    (std::string const& reason)
  {
    for (std::string const& response_id : _outstanding_responses)
    {
      _rts_workflow_response( response_id
                            , std::make_exception_ptr (std::runtime_error (reason))
                            );
    }
    _outstanding_responses.clear();
  }

  void layer::extract_from_nets()
  try
  {
    while(true)
    {
      boost::optional<type::activity_t> activity;
      try
      {
        fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            //! \note We wrap all input activites in a net.
            activity = boost::get<we::type::net_type>
                        ( _workflow.transition().data())
                        . fire_expressions_and_extract_activity_random
                          ( _random_extraction_engine
                          , [this] ( pnet::type::value::value_type const& description
                                   , pnet::type::value::value_type const& value
                                   )
                              {
                                workflow_response ( get_response_id (description)
                                                  , value
                                                  );
                              }
                          );
          }
        , "workflow interpretation"
        );
      }
      catch (...)
      {
        rts_workflow_finished (fhg::util::current_exception_printer (": ").string());
        break;
      }

      if (activity)
      {
        const id_type child_id (_rts_id_generator());
        _rts_submit (child_id, *activity);
        _running_tasks.emplace(child_id);
      }
      else
      {
        if (_wf_state == RUNNING && _running_tasks.empty()
           && ( !_workflow.transition().prop().is_true ({"drts", "wait_for_output"})
              ||!_workflow.output_missing()
              )
           )
        {
          rts_workflow_finished ( fhg::util::starts_with
              ( wrapped_activity_prefix()
              , _workflow.transition().name()
              )
              ? unwrap (_workflow)
              : _workflow
              );
          _command_queue.interrupt(queue_interrupted());
        }
        else if (_wf_state == CANCELED && _running_tasks.empty())
        {
          _command_queue.interrupt(queue_interrupted());
          rts_workflow_finished(sdpa::task_canceled_reason_t{});
        }
        else  if (_wf_state == ERROR && _running_tasks.empty())
        {
          _command_queue.interrupt(queue_interrupted());
          rts_workflow_finished(_error);
        }

        try
        {
          _command_queue.get()();
        }
        catch(queue_interrupted const&)
        {
          // queue was interrupted and all commands received before the interruption were processed
          // can exit the thread
          break;
        }
        catch(std::runtime_error& )
        {
          rts_workflow_finished (fhg::util::current_exception_printer (": ").string());
          break;
        }
      }
    }
  }
  catch (...)
  {
    rts_workflow_finished (_error);
  }


  void layer::rts_workflow_finished (sdpa::finished_reason_t const& reason)
  {
    struct
    {
      using result_type = void;
      void operator() (sdpa::task_completed_reason_t const& result) const
      {
        _this->cancel_outstanding_responses ("workflow finished");
        _this->_rts_finished (result);
      }
      void operator() (sdpa::task_failed_reason_t const& error) const
      {
        _this->cancel_outstanding_responses ("workflow failed");
        _this->_rts_finished (error);
      }
      void operator() (sdpa::task_canceled_reason_t const& reason) const
      {
        _this->cancel_outstanding_responses ("workflow canceled");
        _this->_rts_finished (reason);
      }

      layer* _this;
    } visitor = {this};
    boost::apply_visitor (visitor, reason);
  }

  void layer::cancel_remaining_tasks()
  {
    for (auto& task_id : _running_tasks)
    {
      if (_canceling_tasks.count(task_id)==0)
      {
        _rts_cancel(task_id);
        _canceling_tasks.emplace(task_id);
      }
    }
  }
}

