// {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/requirement.hpp>
#include <we/type/user_data.hpp>
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

      //! \todo pass in functions, not RTS
      template <class RTS>
        layer ( RTS* runtime_system
              , boost::function<id_type()> gen
              )

          // external activities from submitted net -> child jobs
          : _rts_submit_IMPL (boost::bind (&RTS::submit, runtime_system, _1, _2, _3, _4, _5))

          // reply to cancel (parent)/on failure (child) -> child jobs
          , _rts_cancel (boost::bind (&RTS::cancel, runtime_system, _1, _2))

          // reply to submit -> top level
          , _rts_finished (boost::bind (&RTS::finished, runtime_system, _1, _2))

          // reply to submit (on failure of child) -> top level
          , _rts_failed (boost::bind (&RTS::failed, runtime_system, _1, _2, _3))

          // reply to cancel (parent) -> child jobs
          , _rts_canceled (boost::bind (&RTS::canceled, runtime_system, _1))

          , _extract_from_nets_thread (&layer::extract_from_nets, this)
          , _id_generator (gen)
      {}

      ~layer();


      // initial from exec_layer -> top level
      void submit ( const id_type&
                  , const type::activity_t&
                  , const we::type::user_data &
                  );

      // initial from exec_layer -> top level
      void cancel (const id_type&, const reason_type&);

      // reply to _rts_submit -> childs only
      void finished ( const id_type&
                    , const type::activity_t&
                    );

      // reply to _rts_submit -> childs only
      void failed ( const id_type&
                  , const int error_code
                  , const std::string&
                  );

      // reply to _rts_cancel (after top level canceled/failure) -> childs only
      void canceled (const id_type&);


    private:
      boost::function<void ( id_type const &
                           , type::activity_t const &
                           , const std::list<we::type::requirement_t>&
                           , const we::type::schedule_data&
                           , const we::type::user_data &
                           )> _rts_submit_IMPL;
      void _rts_submit ( id_type const & id
                       , type::activity_t const & act
                       , we::type::user_data const& user_data
                       )
      {
        we::type::schedule_data schedule_data
          ( act.transition().get_schedule_data<long> (act.input(), "num_worker")
          , act.transition().get_schedule_data<long> (act.input(), "vmem")
          );

        _rts_submit_IMPL ( id
                         , act
                         , act.transition().requirements()
                         , schedule_data
                         , user_data
                         );
      }

      boost::function<void ( id_type const &
                           , reason_type const &
                           )>  _rts_cancel;

      boost::function<void ( id_type const &
                           , type::activity_t const &
                           )> _rts_finished;

      boost::function<void ( id_type const &
                           , const int error_code
                           , std::string const & reason
                           )> _rts_failed;

      boost::function<void (id_type const &)> _rts_canceled;

      //! \todo test the embedded data structures
      struct activity_data_type
      {
        activity_data_type ( id_type id
                           , type::activity_t activity
                           , we::type::user_data user
                           )
          : _id (id)
          , _activity (activity)
          , _user_data (user)
        {}

        boost::optional<type::activity_t>
          fire_internally_and_extract_external();
        void child_finished (const type::activity_t&);

        id_type _id;
        type::activity_t _activity;
        we::type::user_data _user_data;

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

      void request_cancel ( const id_type&
                          , boost::function<void()> after
                          , reason_type const&
                          );
      void cancel_child_jobs ( activity_data_type
                             , boost::function<void()> after
                             , reason_type const&
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

      boost::function<id_type()> _id_generator;

      void update_activity
        (id_type, type::activity_t result, boost::function<void()>);
      void do_update_activity
        (activity_data_type&, type::activity_t, boost::function<void()>);

      void finalize_finished (id_type parent, id_type child);
    };
  }
}

#endif
