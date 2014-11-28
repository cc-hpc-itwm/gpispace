// bernd.loerwald@itwm.fraunhofer.de
#define BOOST_TEST_MODULE Message flow between client/orchestrator/agent and \
                          children/parents

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

namespace
{
  struct network_strategy
  {
    network_strategy (std::string name, utils::kvs_server const& kvs_server)
      : _event_received()
      , _kvs_client
        ( new fhg::com::kvs::client::kvsc
          ( _kvs_client_io_service
          , kvs_server.kvs_host(), kvs_server.kvs_port()
          , true, boost::posix_time::seconds (120), 1
          )
        )
      , _network
        ( [this] (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr e)
          {
            _event_received.notify (e);
          }
        , _peer_io_service
        , name, fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
        , _kvs_client
        )
    {}

    fhg::com::p2p::address_t connect_to
      (fhg::com::host_t const& host, fhg::com::port_t const& port)
    {
      return _network.connect_to (host, port);
    }

    void send (fhg::com::p2p::address_t const& destination, sdpa::events::SDPAEvent* event)
    {
      _network.perform (destination, sdpa::events::SDPAEvent::Ptr (event));
    }

    template<typename T> boost::shared_ptr<T> wait_for_event()
    {
      sdpa::events::SDPAEvent::Ptr raw_event (_event_received.wait());
      return boost::dynamic_pointer_cast<T> (raw_event);
    }

  private:
    fhg::util::thread::event<sdpa::events::SDPAEvent::Ptr> _event_received;
    boost::asio::io_service _kvs_client_io_service;
    fhg::com::kvs::kvsc_ptr_t _kvs_client;
    boost::asio::io_service _peer_io_service;
    sdpa::com::NetworkStrategy _network;
  };
}

BOOST_AUTO_TEST_SUITE (client)

//! \todo retreiveResults() sends RetrieveJobResults, receives JobResultsReply

//! \todo queryJob() sends QueryJobStatus, receives JobStatusReply

//! \todo discoverJobStates() sends DiscoverJobStates, receives DiscoverJobStatesReply

//! \todo cancelJob() sends CancelJob, receives CancelJobAck

//! \todo subscription sends Subscribe, receives SubscribeAck, takes
//! next JobFinished/JobFailed/CancelJobAck

//! \todo throws on any method, if ErrorEvent received

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE (agent)

//! \todo JobFinished: invalid job, aborted job, no wfe (impossible?),
//! partial results done, partial results missing, already canceling
//! job, partial job failed, wrong child

//! \todo JobFailed: invalid job, aborted job, no wfe (impossible?),
//! partial results done, partial results missing, job already
//! canceling, wrong child

//! \todo CancelJob: invalid job, aborted job, already canceling job,
//! terminal job, running job, pending job, other submitter

//! \todo CancelJobAck: invalid job, aborted job, wrong child
//! reporting canceled, no wfe, job in state not being able to
//! cancelack, agent is top, parent is notified?, partial job
//! completed, partial job not completed

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE (orchestrator)

//! \todo CancelJob: invalid job, canceling job, terminal job,
//! subscribing, submitted, non-submitted, subscribers notified?

//! \todo CancelJobAck: invalid job, valid job, wrong child
//! responding, subscribers notified?, job not in cancelackable state

//! \todo DeleteJob: invalid job, non-terminal job, terminal job

//! \todo JobFinished: invalid job, valid job, subscribers notified?,
//! wrong child responding

//! \todo JobFailed: invalid job, valid job, subscribers notified?,
//! wrong child responding

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE (generic)

BOOST_AUTO_TEST_CASE (job_finished_ack_fails_with_bad_job_id)
{
  const std::string orchestrator_name (utils::random_peer_name());
  const std::string child_name (utils::random_peer_name());

  const utils::kvs_server kvs_server;

  boost::asio::io_service peer_io_service;
  boost::asio::io_service kvs_client_io_service;
  boost::asio::io_service rpc_io_service;
  const sdpa::daemon::Orchestrator orchestrator
    ( orchestrator_name
    , "localhost"
    , peer_io_service
    , kvs_client_io_service
    , kvs_server.kvs_host(), kvs_server.kvs_port()
    , rpc_io_service
    );

  network_strategy child (child_name, kvs_server);

  child.send ( child.connect_to
                 ( fhg::com::host_t
                     (orchestrator.peer_local_endpoint().address().to_string())
                 , fhg::com::port_t
                     (std::to_string (orchestrator.peer_local_endpoint().port()))
                 )
             , new sdpa::events::JobFinishedAckEvent (fhg::util::random_string())
             );

  sdpa::events::ErrorEvent::Ptr event
    (child.wait_for_event<sdpa::events::ErrorEvent>());

  BOOST_REQUIRE (event);
  BOOST_REQUIRE_EQUAL (event->reason(), "Couldn't find the job!");
  BOOST_REQUIRE_EQUAL
    (event->error_code(), sdpa::events::ErrorEvent::SDPA_EUNKNOWN);
  BOOST_REQUIRE_EQUAL (event->job_id(), boost::none);
}

//! \todo Analyse control flow in all GenericDaemon event handlers

BOOST_AUTO_TEST_SUITE_END()
