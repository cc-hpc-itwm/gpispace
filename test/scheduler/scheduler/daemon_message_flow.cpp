// Copyright (C) 2014-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/daemon/Agent.hpp>
#include <gspc/scheduler/events/ErrorEvent.hpp>
#include <gspc/scheduler/events/JobFinishedAckEvent.hpp>
#include <test/scheduler/NetworkStrategy.hpp>
#include <test/scheduler/scheduler/utils.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/testing/certificates_data.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/asio/io_service.hpp>
#include <optional>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <string>
#include <utility>

namespace
{
  struct network_strategy
  {
    network_strategy (gspc::Certificates const& certificates)
      : _event_received()
      , _network
        ( [this] (gspc::com::p2p::address_t const&, gspc::scheduler::events::SchedulerEvent::Ptr e)
          {
            _event_received.set_value (e);
          }
        , std::make_unique<::boost::asio::io_service>()
        , gspc::com::host_t ("127.0.0.1"), gspc::com::port_t (0)
        , certificates
        )
    {}

    gspc::com::p2p::address_t connect_to_TESTING_ONLY
      (gspc::com::host_t const& host, gspc::com::port_t const& port)
    {
      return _network.connect_to_TESTING_ONLY (host, port);
    }

    template<typename Event, typename... Args>
    void send (gspc::com::p2p::address_t const& destination, Args... args)
    {
      _network.perform<Event> (destination, std::forward<Args> (args)...);
    }

    auto wait_for_event() -> gspc::scheduler::events::SchedulerEvent::Ptr
    {
      return _event_received.get_future().get();
    }

  private:
    std::promise<gspc::scheduler::events::SchedulerEvent::Ptr> _event_received;
    gspc::scheduler::test::NetworkStrategy _network;
  };
}

BOOST_AUTO_TEST_SUITE (client)

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

BOOST_AUTO_TEST_SUITE (agent_without_workflow_engine)

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

BOOST_DATA_TEST_CASE
  (job_finished_ack_fails_with_bad_job_id, certificates_data, certificates)
{
  const std::string agent_name (utils::random_peer_name());

  const gspc::scheduler::daemon::Agent agent
    ( agent_name
    , "localhost"
    , std::make_unique<::boost::asio::io_service>()
    , std::nullopt
    , certificates
    );

  network_strategy child (certificates);

  child.send<gspc::scheduler::events::JobFinishedAckEvent>
    ( child.connect_to_TESTING_ONLY
      ( gspc::com::host_t
          ( gspc::util::connectable_to_address_string
              (agent.peer_local_endpoint().address())
          )
      , gspc::com::port_t {agent.peer_local_endpoint().port()}
      )
    , gspc::testing::random_string()
    );

  auto generic_event {child.wait_for_event()};
  auto* error_event
    { dynamic_cast<gspc::scheduler::events::ErrorEvent*> (generic_event.get())
    };

  BOOST_REQUIRE (error_event);
  BOOST_REQUIRE_EQUAL (error_event->reason(), "Couldn't find the job!");
  BOOST_REQUIRE_EQUAL
    (error_event->error_code(), gspc::scheduler::events::ErrorEvent::SCHEDULER_EUNKNOWN);
  BOOST_REQUIRE_EQUAL (error_event->job_id(), std::nullopt);
}
//! \todo Analyse control flow in all Agent event handlers

BOOST_AUTO_TEST_SUITE_END()
