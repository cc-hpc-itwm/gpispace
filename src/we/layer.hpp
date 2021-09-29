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

#pragma once

#include <util-generic/finally.hpp>

#include <we/plugin/Plugins.hpp>
#include <we/type/Activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/value.hpp>
#include <we/workflow_response.hpp>
#include <we/eureka_response.hpp>

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
#include <unordered_set>
#include <list>
#include <string>
#include <utility>

namespace we
{
    class layer
    {
    public:
      typedef sdpa::job_id_t id_type;

      layer ( // submit: external activities from submitted net -> child jobs
              std::function<void (id_type, type::Activity)> rts_submit
              // reply to cancel (parent)/on failure (child) -> child jobs
            , std::function<void (id_type)> rts_cancel
              // reply to submit on success -> top level
            , std::function<void (id_type, type::Activity)> rts_finished
              // reply to submit on failure (of child) -> top level
            , std::function<void (id_type, std::string reason)> rts_failed
              // reply to cancel (parent) -> top level
            , std::function<void (id_type)> rts_canceled
              // result of put_token (parent) -> top level
            , std::function<void (std::string put_token_id, boost::optional<std::exception_ptr>)> rts_token_put
              //result of workflow_response (parent) -> top level
            , std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> rts_workflow_response
            , std::function<id_type()> rts_id_generator
            , std::mt19937& random_extraction_engine
            );

      // initial from exec_layer -> top level
      void submit (id_type, type::Activity);

      // initial from exec_layer -> top level
      void cancel (id_type);

      // reply to _rts_submit -> childs only
      void finished (id_type, type::Activity);

      // reply to _rts_submit -> childs only
      void failed (id_type, std::string reason);

      // reply to _rts_cancel (after top level canceled/failure) -> childs only
      // shall not be called from within rts_cancel!
      void canceled (id_type);

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
      std::function<void (id_type, type::Activity)> _rts_submit;
      std::function<void (id_type)> _rts_cancel;
      std::function<void (id_type, type::Activity)> _rts_finished;
      std::function<void (id_type, std::string)> _rts_failed;
      std::function<void (id_type)> _rts_canceled;
      std::function<void (std::string, boost::optional<std::exception_ptr>)> _rts_token_put;
      std::function<void (std::string workflow_response_id, boost::variant<std::exception_ptr, pnet::type::value::value_type>)> _rts_workflow_response;
      std::function<id_type()> _rts_id_generator;

      void rts_finished_and_forget (id_type, type::Activity);
      void rts_failed_and_forget (id_type, std::string);
      void rts_canceled_and_forget (id_type);

      void workflow_response ( id_type
                             , std::string const& response_id
                             , boost::variant<std::exception_ptr, pnet::type::value::value_type> const&
                             );
      void cancel_outstanding_responses (id_type, std::string const& reason);

      void eureka_response ( id_type
                           , boost::optional<id_type>
                           , type::eureka_ids_type const& ids
                           );

      std::mutex _outstanding_responses_guard;
      std::unordered_map <id_type, std::unordered_set<std::string>>
        _outstanding_responses;

      gspc::we::plugin::Plugins _plugins;


      struct activity_data_type
      {
        activity_data_type ( id_type id
                           , std::unique_ptr<type::Activity> activity
                           )
          : _id (id)
          , _activity (std::move (activity))
        {}

        void child_finished
          ( type::Activity
          , we::workflow_response_callback const&
          , we::eureka_response_callback const&
          );

        id_type _id;
        std::unique_ptr<type::Activity> _activity;
      };

      struct async_remove_queue
      {
        struct RemovalFunction
        {
          void operator() (activity_data_type&) &&;

          template<typename Fun>
            RemovalFunction (Fun&&);

          struct ToFinish
          {
            ToFinish (layer*, id_type, type::Activity, id_type);

            layer* _that;
            id_type _parent;
            type::Activity _result;
            id_type _id;
          };

          RemovalFunction (ToFinish);

          RemovalFunction (RemovalFunction const&) = delete;
          RemovalFunction (RemovalFunction&&) = default;
          RemovalFunction& operator= (RemovalFunction const&) = delete;
          RemovalFunction& operator= (RemovalFunction&&) = delete;

          boost::variant
            < std::function<void (activity_data_type&)>
            , ToFinish
            > _function;
        };

        activity_data_type get();
        void put (activity_data_type, bool was_active);

        void remove_and_apply
          ( id_type
          , RemovalFunction
          );
        void apply
          ( id_type
          , RemovalFunction
          , std::function<void (std::exception_ptr)> = &std::rethrow_exception
          );

        void forget (id_type, std::string reason);

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

        std::recursive_mutex _container_mutex;
        list_with_id_lookup _container;
        list_with_id_lookup _container_inactive;

        bool _interrupted = false;
        std::condition_variable_any _condition_not_empty_or_interrupted;

        typedef std::unordered_map
          < id_type
          , std::list<std::tuple< RemovalFunction
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

      struct locked_parent_child_relation_type
      {
        void started
          ( id_type parent
          , id_type child
          , boost::optional<type::eureka_id_type> const& eureka_id
          );
        bool terminated (id_type parent, id_type child);

        boost::optional<id_type> parent (id_type child);
        bool contains (id_type parent) const;

        void apply (id_type parent, std::function<void (id_type)>) const;

        void apply_and_remove_eureka ( type::eureka_id_type const&
                                     , id_type const&
                                     , boost::optional<id_type> const&
                                     , std::function<void (id_type)> cancel
                                     );

      private:
        mutable std::mutex _relation_mutex;
        typedef boost::bimaps::bimap
          < boost::bimaps::unordered_multiset_of<id_type>
          , boost::bimaps::unordered_set_of<id_type>
          , boost::bimaps::set_of_relation<>
          > relation_type;
        relation_type _relation;

        using eureka_parent_id_type =
          std::tuple < type::eureka_id_type
                     , id_type
                     >;
        using eureka_in_progress_type =
        boost::bimaps::bimap
          < boost::bimaps::unordered_multiset_of<eureka_parent_id_type>
          , boost::bimaps::unordered_set_of<id_type>
          , boost::bimaps::set_of_relation<>
          >;
        eureka_in_progress_type _eureka_in_progress;
      } _running_jobs;

      std::unordered_set<id_type> _ignore_canceled_by_eureka;

      boost::strict_scoped_thread<> _extract_from_nets_thread;
      fhg::util::finally_t<std::function<void()>> _stop_extracting;
    };
}
