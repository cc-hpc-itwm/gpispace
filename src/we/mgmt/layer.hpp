// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/mgmt/bits/descriptor.hpp>
#include <we/mgmt/bits/execution_policy.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <fhg/assert.hpp>
#include <fhg/error_codes.hpp>
#include <fhg/util/show.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/threadname.hpp>

#include <fhglog/fhglog.hpp>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
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

      typedef std::string external_id_type;
      typedef petri_net::activity_id_type internal_id_type;

      typedef std::string encoded_type;
      typedef std::string reason_type;
      typedef std::string result_type;

    public:

      void submit (const external_id_type&, const encoded_type&, const we::type::user_data &);
      void submit (const external_id_type&, const we::mgmt::type::activity_t&, const we::type::user_data &);
      void cancel (const external_id_type&, const reason_type&);
      void finished (const external_id_type&, const result_type&);
      void failed ( const external_id_type&
                  , const result_type&
                  , const int error_code
                  , const std::string&
                  );
      void canceled (const external_id_type&);

    private:
      // handle execution layer
      boost::function<void ( external_id_type const &
                           , encoded_type const &
                           , const std::list<we::type::requirement_t>&
                           , const we::type::schedule_data&
                           , const we::type::user_data &
                           )> ext_submit;
      boost::function<void ( external_id_type const &
                           , reason_type const &
                           )>  ext_cancel;
      boost::function<void ( external_id_type const &
                           , result_type const &
                           )>  ext_finished;
      boost::function<void ( external_id_type const &
                           , result_type const &
                           , const int error_code
                           , std::string const & reason
                           )>  ext_failed;
      boost::function<void (external_id_type const &)> ext_canceled;

    public:
      template <class E>
        layer (E* exec_layer, boost::function<external_id_type()> gen)
          : ext_submit (boost::bind (&E::submit, exec_layer, _1, _2, _3, _4, _5))
          , ext_cancel (boost::bind (&E::cancel, exec_layer, _1, _2))
          , ext_finished (boost::bind (&E::finished, exec_layer, _1, _2))
          , ext_failed (boost::bind (&E::failed, exec_layer, _1, _2, _3, _4))
          , ext_canceled (boost::bind (&E::canceled, exec_layer, _1))
          , external_id_gen_ (gen)
          , internal_id_gen_ (&petri_net::activity_id_generate)
          , executor_ (boost::bind (&layer::executor, this))
      {
        fhg::util::set_threadname (executor_, "[we-execute]");
      }

      ~layer()
      {
        executor_.interrupt();
        if (executor_.joinable())
        {
          executor_.join();
        }
      }

    private:
      void executor ();

    private:
      boost::function<external_id_type()> external_id_gen_;
      boost::function<internal_id_type()> internal_id_gen_;

      boost::thread executor_;
      fhg::util::thread::queue<net_t> _nets_to_extract_from;
      boost::thread_safe::unordered_map
        < net_t
        , boost::unordered_set<net_t>
        > _awaiting_termination;
      fhg::util::thread::set<set<childids>> _waiting_for_cancelation;
      map<child_id->waiting_for_cancellation.it>

      external_id_type generate_external_id (void) const
      {
        boost::unique_lock<boost::recursive_mutex> lock (id_gen_mutex_);
        return external_id_gen_();
      }

      internal_id_type generate_internal_id (void) const
      {
        boost::unique_lock<boost::recursive_mutex> lock (id_gen_mutex_);
        return internal_id_gen_();
      }

      void activity_failed (internal_id_type const internal_id);
      void activity_canceled (internal_id_type const internal_id);
      void cancel_activity (internal_id_type const internal_id);
      void insert_activity(const detail::descriptor & desc);
      void remove_activity(const detail::descriptor & desc);
      detail::descriptor& lookup (const internal_id_type& id);
    };
  }
}

#endif
