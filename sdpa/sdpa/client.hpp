#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP

#include <sdpa/common.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>

#include <fhg/util/thread/queue.hpp>

#include <fhgcom/peer.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>

namespace sdpa
{
  namespace client
  {
    typedef sdpa::job_result_t result_t;
    struct job_info_t
    {
      int error_code;
      std::string error_message;
    };

    class Client : boost::noncopyable
    {
    public:
      Client ( std::string orchestrator
             , boost::optional<boost::posix_time::time_duration> = boost::none
             );
      ~Client();

      job_id_t submitJob(const job_desc_t &);
      void cancelJob(const job_id_t &);
      status::code queryJob(const job_id_t &);
      status::code queryJob(const job_id_t &, job_info_t &);
      void deleteJob(const job_id_t &);
      result_t retrieveResults(const job_id_t &);

      sdpa::status::code wait_for_terminal_state (job_id_t, job_info_t&);
      sdpa::status::code wait_for_terminal_state_polling (job_id_t, job_info_t&);

    private:
      sdpa::events::SDPAEvent::Ptr wait_for_reply (bool use_timeout);

      std::string _name;

      fhg::thread::queue<sdpa::events::SDPAEvent::Ptr, std::list>
        m_incoming_events;

      // config variables
      boost::posix_time::time_duration timeout_;
      std::string orchestrator_;

      fhg::com::message_t message_for_event (const sdpa::events::SDPAEvent*);

      template<typename Expected, typename Sent>
        Expected send_and_wait_for_reply (Sent event);

      void handle_recv (boost::system::error_code const & ec);

      fhg::com::peer_t m_peer;
      fhg::com::message_t m_message;
      boost::thread _peer_thread;
      bool _stopping;
    };
  }
}

#endif
