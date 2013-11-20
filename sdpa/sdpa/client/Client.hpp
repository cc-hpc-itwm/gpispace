#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP 1

#include <sdpa/client/exceptions.hpp>
#include <sdpa/common.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>
#include <sdpa/util/Config.hpp>

#include <fhg/util/thread/queue.hpp>

#include <fhgcom/peer.hpp>

#include <cstdlib>
#include <string>

#include <boost/thread.hpp> // condition variables

namespace sdpa
{
  namespace client
  {
    typedef sdpa::util::NewConfig config_t;
    typedef sdpa::job_result_t result_t;
    struct job_info_t
    {
      int error_code;
      std::string error_message;
    };

    class Client : boost::noncopyable
    {
    public:
      Client (const config_t& cfg);
      ~Client();

      static config_t config()
      {
        using namespace sdpa::util;
        config_t cfg ("client", "SDPAC_");
        cfg.specific_opts().add_options()
          ( "orchestrator"
          , po::value<std::string>()->default_value ("orchestrator")
          , "name of the orchestrator"
          )
          ( "config,C"
          , po::value<std::string>()->default_value
            (std::getenv ("HOME") + std::string ("/.sdpa/configs/sdpac.rc"))
          , "path to the configuration file"
          );
        return cfg;
      }

      job_id_t submitJob(const job_desc_t &) throw (ClientException);
      void cancelJob(const job_id_t &) throw (ClientException);
      status::code queryJob(const job_id_t &) throw (ClientException);
      status::code queryJob(const job_id_t &, job_info_t &);
      void deleteJob(const job_id_t &) throw (ClientException);
      result_t retrieveResults(const job_id_t &) throw (ClientException);

      sdpa::status::code wait_for_terminal_state (job_id_t, job_info_t&);
      sdpa::status::code wait_for_terminal_state_polling (job_id_t, job_info_t&);

    private:
      typedef unsigned long long timeout_t;

      sdpa::events::SDPAEvent::Ptr wait_for_reply (bool use_timeout);

      std::string _name;

      fhg::thread::queue<sdpa::events::SDPAEvent::Ptr, std::list>
        m_incoming_events;

      // config variables
      timeout_t timeout_;
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
