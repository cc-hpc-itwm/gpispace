// {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

#pragma once

#include <util-generic/finally.hpp>

#include <we/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/value.hpp>
#include <we/workflow_response.hpp>
#include <we/threadsafe_queue.hpp>

#include <sdpa/types.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/optional.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <random>
#include <unordered_map>

namespace we
{
    class layer
    {
    public:
      typedef sdpa::job_id_t id_type;

      layer ( // submit: external activities from submitted net -> child jobs
              std::function<void (id_type, type::activity_t)> rts_submit
              // reply to cancel (cancel running jobs) -> top level
            , std::function<void (id_type)> rts_cancel
              // reply to submit on success -> top level
            , std::function<void (sdpa::finished_reason_t const&)> rts_finished
              // result of put_token (parent) -> top level
            , std::function<void (std::string put_token_id, boost::optional<std::exception_ptr>)> rts_token_put
              //result of workflow_response (parent) -> top level
            , std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> rts_workflow_response
            , std::function<id_type()> rts_id_generator
            , std::mt19937& random_extraction_engine
            , const type::activity_t& wf
            );

      // reply to _rts_submit -> childs only
      void finished (id_type, sdpa::finished_reason_t);

      // cancel workflow
      void cancel (id_type);

      // initial from exec_layer -> top level, unique put_token_id
      void put_token ( id_type
                     , std::string put_token_id
                     , std::string place_name
                     , pnet::type::value::value_type
                     );

      // initial from exec_layer -> top level, unique workflow_response_id
      void request_workflow_response ( id_type
                                     , std::string workflow_response_id
                                     , std::string place_name
                                     , pnet::type::value::value_type
                                     );

    private:
      std::function<void (id_type, type::activity_t)> _rts_submit;
      std::function<void (id_type)> _rts_cancel;
      std::function<void (sdpa::finished_reason_t const&)> _rts_finished;
      std::function<void (std::string, boost::optional<std::exception_ptr>)> _rts_token_put;
      std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> _rts_workflow_response;
      std::function<id_type()> _rts_id_generator;

      std::mt19937& _random_extraction_engine;
      we::interruptible_threadsafe_queue
          < std::list<std::function<void()>>> _command_queue;

      // lists of tasks (only accessed from the extraction thread)
      std::set <we::layer::id_type> _running_tasks;
      std::set <we::layer::id_type> _canceling_tasks;
      std::unordered_set<std::string> _outstanding_responses;

      type::activity_t _workflow;
      boost::strict_scoped_thread<> _extract_from_nets_thread;

      enum {
        RUNNING = 0,
        CANCELED,
        ERROR
      }  _wf_state;
      std::string _error;

      void finished_correctly (id_type id, sdpa::task_completed_reason_t const& );
      void finished_failure (id_type id, sdpa::task_failed_reason_t const& );
      void finished_canceled(id_type id, sdpa::task_canceled_reason_t const& );

      void extract_from_nets();
      void workflow_response ( std::string const&
                             , boost::variant<std::exception_ptr, pnet::type::value::value_type> const&
                             );
      void cancel_outstanding_responses (std::string const& );
      void rts_workflow_finished (sdpa::finished_reason_t const&);
      void cancel_remaining_tasks();

      class queue_interrupted : public std::runtime_error
      {
      public:
        queue_interrupted()
          : std::runtime_error ("command queue interrupted")
          {}
      };

    };
}

