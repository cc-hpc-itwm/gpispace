#pragma once

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/types.hpp>

#include <we/layer.hpp>
#include <we/type/activity.hpp>
#include <we/type/value.hpp>

#include <fhgcom/peer.hpp>

#include <fhg/util/thread/queue.hpp>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace sdpa
{
  namespace client
  {
    struct job_info_t
    {
      std::string error_message;
    };

    class Client : boost::noncopyable
    {
    public:
      Client ( fhg::com::host_t const& orchestrator_host
             , fhg::com::port_t const& orchestrator_port
             , std::unique_ptr<boost::asio::io_service> peer_io_service
             , fhg::com::Certificates const& certificates
             );
      ~Client();

      job_id_t submitJob(we::type::activity_t);
      void cancelJob(const job_id_t &);
      status::code queryJob(const job_id_t &);
      status::code queryJob(const job_id_t &, job_info_t &);
      void deleteJob(const job_id_t &);
      we::type::activity_t retrieveResults(const job_id_t &);
      sdpa::discovery_info_t discoverJobStates(const we::layer::id_type& discover_id, const job_id_t &job_id);
      void put_token
        (job_id_t, std::string place_name, pnet::type::value::value_type);
      pnet::type::value::value_type workflow_response
        (job_id_t, std::string place_name, pnet::type::value::value_type);

      sdpa::status::code wait_for_terminal_state (job_id_t, job_info_t&);
      sdpa::status::code wait_for_terminal_state_polling (job_id_t, job_info_t&);

    private:
      std::mutex _make_client_thread_safe;

      fhg::thread::queue<sdpa::events::SDPAEvent::Ptr> m_incoming_events;

      template<typename Expected, typename Sent>
        Expected send_and_wait_for_reply (Sent event);

      void handle_recv ( boost::system::error_code const& ec
                       , boost::optional<fhg::com::p2p::address_t>
                       , fhg::com::message_t message
                       );

      bool _stopping;
      fhg::com::peer_t m_peer;
      fhg::com::p2p::address_t _drts_entrypoint_address;
    };
  }
}
