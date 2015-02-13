#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>
#include <we/layer.hpp>

#include <fhg/util/thread/queue.hpp>

#include <fhgcom/peer.hpp>

#include <we/type/value.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <fhg/util/first_then.hpp>

inline std::ostream& operator<<(std::ostream& os, const sdpa::discovery_info_t& disc_info)
{
  std::string state(disc_info.state() ? sdpa::status::show (disc_info.state().get()):"NONE");
  os<<"["<<disc_info.job_id()<<", "<<state<<", [";
  fhg::util::first_then<std::string> const sep ("", ", ");
  for (const sdpa::discovery_info_t& child_info : disc_info.children())
  {
    os<<sep<<child_info;
  }
  os<<"]]";

  return os;
}

namespace sdpa
{
  namespace client
  {
    typedef sdpa::job_result_t result_t;
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
             );
      ~Client();

      job_id_t submitJob(const job_desc_t &);
      void cancelJob(const job_id_t &);
      status::code queryJob(const job_id_t &);
      status::code queryJob(const job_id_t &, job_info_t &);
      void deleteJob(const job_id_t &);
      result_t retrieveResults(const job_id_t &);
      sdpa::discovery_info_t discoverJobStates(const we::layer::id_type& discover_id, const job_id_t &job_id);
      void put_token
        (job_id_t, std::string place_name, pnet::type::value::value_type);

      sdpa::status::code wait_for_terminal_state (job_id_t, job_info_t&);
      sdpa::status::code wait_for_terminal_state_polling (job_id_t, job_info_t&);

    private:
      fhg::thread::queue<sdpa::events::SDPAEvent::Ptr> m_incoming_events;

      template<typename Expected, typename Sent>
        Expected send_and_wait_for_reply (Sent event);

      void handle_recv (boost::system::error_code const & ec, boost::optional<fhg::com::p2p::address_t>);

      fhg::com::message_t m_message;
      bool _stopping;
      fhg::com::peer_t m_peer;
      struct peer_starter
      {
        peer_starter (fhg::com::peer_t& peer)
          : _peer (peer)
        {
          _peer.start();
        }
        fhg::com::peer_t& _peer;
      } _peer_starter = {m_peer};
      fhg::com::p2p::address_t _drts_entrypoint_address;
    };
  }
}

#endif
