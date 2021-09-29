// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/daemon/Agent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <utility>

namespace
{
  struct network_strategy
  {
    network_strategy (fhg::com::Certificates const& certificates)
      : _event_received()
      , _network
        ( [this] (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr e)
          {
            _event_received.notify (e);
          }
        , std::make_unique<boost::asio::io_service>()
        , fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
        , certificates
        )
    {}

    fhg::com::p2p::address_t connect_to_TESTING_ONLY
      (fhg::com::host_t const& host, fhg::com::port_t const& port)
    {
      return _network.connect_to_TESTING_ONLY (host, port);
    }

    template<typename Event, typename... Args>
    void send (fhg::com::p2p::address_t const& destination, Args... args)
    {
      _network.perform<Event> (destination, std::forward<Args> (args)...);
    }

    template<typename T> boost::shared_ptr<T> wait_for_event()
    {
      sdpa::events::SDPAEvent::Ptr raw_event (_event_received.wait());
      return boost::dynamic_pointer_cast<T> (raw_event);
    }

  private:
    fhg::util::thread::event<sdpa::events::SDPAEvent::Ptr> _event_received;
    sdpa::com::NetworkStrategy _network;
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
  const std::string child_name (utils::random_peer_name());

  const sdpa::daemon::Agent agent
    ( agent_name
    , "localhost"
    , std::make_unique<boost::asio::io_service>()
    , boost::none
    , true
    , certificates
    );

  network_strategy child (certificates);

  child.send<sdpa::events::JobFinishedAckEvent>
    ( child.connect_to_TESTING_ONLY
      ( fhg::com::host_t
          ( fhg::util::connectable_to_address_string
              (agent.peer_local_endpoint().address())
          )
      , fhg::com::port_t
          ( std::to_string
              (agent.peer_local_endpoint().port())
          )
      )
    , fhg::util::testing::random_string()
    );

  sdpa::events::ErrorEvent::Ptr event
    (child.wait_for_event<sdpa::events::ErrorEvent>());

  BOOST_REQUIRE (event);
  BOOST_REQUIRE_EQUAL (event->reason(), "Couldn't find the job!");
  BOOST_REQUIRE_EQUAL
    (event->error_code(), sdpa::events::ErrorEvent::SDPA_EUNKNOWN);
  BOOST_REQUIRE_EQUAL (event->job_id(), boost::none);
}
//! \todo Analyse control flow in all Agent event handlers

BOOST_AUTO_TEST_SUITE_END()
