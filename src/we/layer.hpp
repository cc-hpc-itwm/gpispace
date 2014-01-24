// {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/value.hpp>

#include <sdpa/types.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace we
{
    class layer
    {
    public:
      typedef sdpa::job_id_t id_type;

      layer ( // submit: external activities from submitted net -> child jobs
              boost::function<void (id_type, type::activity_t)> rts_submit
              // reply to cancel (parent)/on failure (child) -> child jobs
            , boost::function<void (id_type)> rts_cancel
              // reply to submit on success -> top level
            , boost::function<void (id_type, type::activity_t)> rts_finished
              // reply to submit on failure (of child) -> top level
            , boost::function<void (id_type, int errorcode, std::string reason)> rts_failed
              // reply to cancel (parent) -> top level
            , boost::function<void (id_type)> rts_canceled
              // reply to discover (parent) -> child jobs
            , boost::function<void (id_type discover_id, id_type)> rts_discover
              // result of discover (parent) -> top level
            , boost::function<void (id_type discover_id, sdpa::discovery_info_t)> rts_discovered
            , boost::function<id_type()> rts_id_generator
            , boost::mt19937& random_extraction_engine
            );
      ~layer();

      // initial from exec_layer -> top level
      void submit (id_type, type::activity_t);

      // initial from exec_layer -> top level
      void cancel (id_type);

      // reply to _rts_submit -> childs only
      void finished (id_type, type::activity_t);

      // reply to _rts_submit -> childs only
      void failed (id_type, int error_code, std::string reason);

      // reply to _rts_cancel (after top level canceled/failure) -> childs only
      void canceled (id_type);

      // initial from exec_layer -> top level, unique discover_id
      void discover (id_type discover_id, id_type);

      // reply to _rts_discover (after top level discovered/failure) -> childs only
      void discovered (id_type discover_id, sdpa::discovery_info_t);

    private:
      boost::function<void (id_type, type::activity_t)> _rts_submit;
      boost::function<void (id_type)> _rts_cancel;
      boost::function<void (id_type, type::activity_t)> _rts_finished;
      boost::function<void (id_type, int, std::string)> _rts_failed;
      boost::function<void (id_type)> _rts_canceled;
      boost::function<void (id_type, id_type)> _rts_discover;
      boost::function<void (id_type, sdpa::discovery_info_t)> _rts_discovered;
      boost::function<id_type()> _rts_id_generator;

      void rts_finished_and_forget (id_type, type::activity_t);
      void rts_failed_and_forget (id_type, int ec, std::string);
      void rts_canceled_and_forget (id_type);


      struct activity_data_type
      {
        activity_data_type (id_type id, type::activity_t activity)
          : _id (id)
          , _activity (activity)
        {}

        void child_finished (type::activity_t);

        id_type _id;
        type::activity_t _activity;
      };

      struct async_remove_queue
      {
        activity_data_type get();
        void put (activity_data_type, bool was_active);

        void remove_and_apply
          (id_type, boost::function<void (activity_data_type)>);
        void apply (id_type, boost::function<void (activity_data_type&)>);

        void forget (id_type);

      private:
        struct list_with_id_lookup
        {
          typedef boost::unordered_map< id_type
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

        mutable boost::mutex _container_mutex;
        list_with_id_lookup _container;
        list_with_id_lookup _container_inactive;

        boost::condition_variable_any _condition_non_empty;

        typedef boost::unordered_map
          < id_type
          , std::list<std::pair<boost::function<void (activity_data_type&)>, bool> >
          > to_be_removed_type;
        to_be_removed_type _to_be_removed;
      } _nets_to_extract_from;

      boost::mt19937& _random_extraction_engine;
      void extract_from_nets();
      boost::thread _extract_from_nets_thread;

      void request_cancel (id_type, boost::function<void()> after);
      void cancel_child_jobs (activity_data_type, boost::function<void()> after);

      boost::unordered_map<id_type, boost::function<void()> >
        _finalize_job_cancellation;

      mutable boost::mutex _discover_state_mutex;
      boost::unordered_map
        < id_type, std::pair<std::size_t, sdpa::discovery_info_t >
        > _discover_state;

      struct locked_parent_child_relation_type
      {
        void started (id_type parent, id_type child);
        bool terminated (id_type parent, id_type child);

        boost::optional<id_type> parent (id_type child);
        bool contains (id_type parent) const;

        void apply (id_type parent, boost::function<void (id_type)>) const;

      private:
        mutable boost::mutex _relation_mutex;
        typedef boost::bimaps::bimap
          < boost::bimaps::unordered_multiset_of<id_type>
          , boost::bimaps::unordered_set_of<id_type>
          , boost::bimaps::set_of_relation<>
          > relation_type;
        relation_type _relation;
      } _running_jobs;

      void finalize_finished
        (activity_data_type&, type::activity_t, id_type parent, id_type child);
    };
}

#endif
