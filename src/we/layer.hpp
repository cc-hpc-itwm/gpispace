// {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

#pragma once

#include <util-generic/finally.hpp>

#include <we/plugin/Plugins.hpp>
#include <we/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/value.hpp>
#include <we/workflow_response.hpp>

#include <sdpa/discovery_info.hpp>
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
              // reply to cancel (parent)/on failure (child) -> child jobs
            , std::function<void (id_type)> rts_cancel
              // reply to submit on success -> top level
            , std::function<void (id_type, type::activity_t)> rts_finished
              // reply to submit on failure (of child) -> top level
            , std::function<void (id_type, std::string reason)> rts_failed
              // reply to cancel (parent) -> top level
            , std::function<void (id_type)> rts_canceled
              // reply to discover (parent) -> child jobs
            , std::function<void (id_type discover_id, id_type)> rts_discover
              // result of discover (parent) -> top level
            , std::function<void (id_type discover_id, sdpa::discovery_info_t)> rts_discovered
              // result of put_token (parent) -> top level
            , std::function<void (std::string put_token_id, boost::optional<std::exception_ptr>)> rts_token_put
              //result of workflow_response (parent) -> top level
            , std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> rts_workflow_response
            , std::function<id_type()> rts_id_generator
            , std::mt19937& random_extraction_engine
            );

      // initial from exec_layer -> top level
      void submit (id_type, type::activity_t);

      // initial from exec_layer -> top level
      void cancel (id_type);

      // reply to _rts_submit -> childs only
      void finished (id_type, type::activity_t);

      // reply to _rts_submit -> childs only
      void failed (id_type, std::string reason);

      // reply to _rts_cancel (after top level canceled/failure) -> childs only
      // shall not be called from within rts_cancel!
      void canceled (id_type);

      // initial from exec_layer -> top level, unique discover_id
      void discover (id_type discover_id, id_type);

      // reply to _rts_discover (after top level discovered/failure) -> childs only
      // shall not be called from within rts_discover!
      void discovered (id_type discover_id, sdpa::discovery_info_t);

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
      std::function<void (id_type, type::activity_t)> _rts_finished;
      std::function<void (id_type, std::string)> _rts_failed;
      std::function<void (id_type)> _rts_canceled;
      std::function<void (id_type, id_type)> _rts_discover;
      std::function<void (id_type, sdpa::discovery_info_t)> _rts_discovered;
      std::function<void (std::string, boost::optional<std::exception_ptr>)> _rts_token_put;
      std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> _rts_workflow_response;
      std::function<id_type()> _rts_id_generator;

      void rts_finished_and_forget (id_type, type::activity_t);
      void rts_failed_and_forget (id_type, std::string);
      void rts_canceled_and_forget (id_type);

      void workflow_response ( id_type
                             , std::string const& response_id
                             , boost::variant<std::exception_ptr, pnet::type::value::value_type> const&
                             );
      void cancel_outstanding_responses (id_type, std::string const& reason);

      std::mutex _outstanding_responses_guard;
      std::unordered_map <id_type, std::unordered_set<std::string>>
        _outstanding_responses;

      gspc::we::plugin::Plugins _plugins;


      struct activity_data_type
      {
        activity_data_type ( id_type id
                           , std::unique_ptr<type::activity_t> activity
                           )
          : _id (id)
          , _activity (std::move (activity))
        {}

        void child_finished
          (type::activity_t, we::workflow_response_callback const&);

        id_type _id;
        std::unique_ptr<type::activity_t> _activity;
      };

      struct async_remove_queue
      {
        activity_data_type get();
        void put (activity_data_type, bool was_active);

        void remove_and_apply
          ( id_type
          , std::function<void (activity_data_type const&)>
          , std::function<void (std::exception_ptr)> = &std::rethrow_exception
          );
        void apply
          ( id_type
          , std::function<void (activity_data_type&)>
          , std::function<void (std::exception_ptr)> = &std::rethrow_exception
          );

        void forget (id_type);

        struct interrupted{};
        void interrupt();

      private:
        struct list_with_id_lookup
        {
          typedef std::unordered_map< id_type
                                    , std::list<activity_data_type>::iterator
                                    > position_in_container_type;
          typedef position_in_container_type::iterator iterator;

          activity_data_type get_front();
          void push_back (activity_data_type);
          iterator find (id_type);
          iterator end();
          void erase (iterator);
          bool empty() const;
        private:
          std::list<activity_data_type> _container;
          position_in_container_type _position_in_container;
        };

        mutable std::recursive_mutex _container_mutex;
        list_with_id_lookup _container;
        list_with_id_lookup _container_inactive;

        bool _interrupted = false;
        std::condition_variable_any _condition_not_empty_or_interrupted;

        typedef std::unordered_map
          < id_type
          , std::list<std::tuple< std::function<void (activity_data_type&)>
                                , std::function<void (std::exception_ptr)>
                                , bool
                                >
                     >
          > to_be_removed_type;
        to_be_removed_type _to_be_removed;
      } _nets_to_extract_from;

      std::mt19937& _random_extraction_engine;
      void extract_from_nets();

      std::unordered_map<id_type, std::function<void()>>
        _finalize_job_cancellation;

      mutable std::mutex _discover_state_mutex;
      std::unordered_map
        < id_type, std::pair<std::size_t, sdpa::discovery_info_t >
        > _discover_state;

      struct locked_parent_child_relation_type
      {
        void started (id_type parent, id_type child);
        bool terminated (id_type parent, id_type child);

        boost::optional<id_type> parent (id_type child);
        bool contains (id_type parent) const;

        void apply (id_type parent, std::function<void (id_type)>) const;

      private:
        mutable std::mutex _relation_mutex;
        typedef boost::bimaps::bimap
          < boost::bimaps::unordered_multiset_of<id_type>
          , boost::bimaps::unordered_set_of<id_type>
          , boost::bimaps::set_of_relation<>
          > relation_type;
        relation_type _relation;
      } _running_jobs;

      boost::strict_scoped_thread<> _extract_from_nets_thread;
      fhg::util::finally_t<std::function<void()>> _stop_extracting;
    };
}
