// {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/schedule_data.hpp>

#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace we
{
  namespace mgmt
  {
    class layer
    {
    public:
      typedef std::string id_type;
      typedef std::string encoded_type;
      typedef std::string reason_type;
      typedef std::string result_type;

      layer ( // external activities from submitted net -> child jobs
              boost::function<void ( id_type
                                   , type::activity_t
                                   , id_type parent
                                   )> rts_submit
              // reply to cancel (parent)/on failure (child) -> child jobs
            , boost::function<void ( id_type
                                   , reason_type
                                   )> rts_cancel
              // reply to submit -> top level
            , boost::function<void ( id_type
                                   , type::activity_t
                                   )> rts_finished
              // reply to submit (on failure of child) -> top level
            , boost::function<void ( id_type
                                   , int error_code
                                   , std::string reason
                                   )> rts_failed
              // reply to cancel (parent) -> top level
            , boost::function<void (id_type)> rts_canceled
            , boost::function<id_type()> rts_id_generator
            );
      ~layer();

      // initial from exec_layer -> top level
      void submit (id_type, type::activity_t);

      // initial from exec_layer -> top level
      void cancel (id_type, reason_type);

      // reply to _rts_submit -> childs only
      void finished (id_type, type::activity_t);

      // reply to _rts_submit -> childs only
      void failed (id_type, int error_code, std::string reason);

      // reply to _rts_cancel (after top level canceled/failure) -> childs only
      void canceled (id_type);


    private:
      boost::function<void ( id_type
                           , type::activity_t
                           , id_type parent
                           )> _rts_submit;

      boost::function<void ( id_type
                           , reason_type
                           )> _rts_cancel;

      boost::function<void ( id_type
                           , type::activity_t
                           )> _rts_finished;

      boost::function<void ( id_type
                           , int error_code
                           , std::string reason
                           )> _rts_failed;

      boost::function<void (id_type)> _rts_canceled;

      boost::function<id_type()> _rts_id_generator;


      struct activity_data_type
      {
        activity_data_type (id_type id, type::activity_t activity)
          : _id (id)
          , _activity (activity)
        {}

        boost::optional<type::activity_t>
          fire_internally_and_extract_external();
        void child_finished (type::activity_t);

        id_type _id;
        type::activity_t _activity;

        boost::mt19937 _random_extraction_engine;
      };

      struct async_remove_queue
      {
        activity_data_type get();
        void put (activity_data_type activity_data, bool);

        void remove_and_apply
          (id_type, boost::function<void (activity_data_type)>);
        void apply (id_type, boost::function<void (activity_data_type&)>);

      private:
        mutable boost::mutex _container_mutex;
        //! \todo measure performance, remove uses std::find_if,
        //! possibly accelerate with additional map<id_type, list::iterator>
        std::list<activity_data_type> _container;
        std::list<activity_data_type> _container_inactive;

        boost::condition_variable_any _condition_non_empty;

        mutable boost::mutex _to_be_removed_mutex;
        typedef boost::unordered_multimap< id_type
                                         , boost::function<void (activity_data_type)>
                                         > to_be_removed_type;
        to_be_removed_type _to_be_removed;

        void apply_callback
          (activity_data_type, boost::function<void (activity_data_type&)>);
      } _nets_to_extract_from;

      void extract_from_nets();
      boost::thread _extract_from_nets_thread;

      void request_cancel ( id_type
                          , boost::function<void()> after
                          , reason_type
                          );
      void cancel_child_jobs ( activity_data_type
                             , boost::function<void()> after
                             , reason_type
                             );

      boost::unordered_map<id_type, boost::function<void()> >
        _finalize_job_cancellation;

      struct locked_parent_child_relation_type
      {
        void started (id_type parent, id_type child);
        bool terminated (id_type parent, id_type child);

        boost::optional<id_type> parent (id_type child);
        bool contains (id_type parent) const;

        void apply (id_type parent, boost::function<void (id_type)>) const;

      private:
        mutable boost::mutex _relation_mutex;
        //! \todo measure performance, parent is reverse, boost::bimap
        typedef boost::unordered_map<id_type, boost::unordered_set<id_type> >
          relation_type;
        relation_type _relation;
      } _running_jobs;

      void finalize_finished
        (activity_data_type&, type::activity_t, id_type parent, id_type child);
    };
  }
}

#endif
