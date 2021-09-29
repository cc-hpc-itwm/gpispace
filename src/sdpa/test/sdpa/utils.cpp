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

#include <sdpa/test/sdpa/utils.hpp>

#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/asio/io_service.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iterator>
#include <list>
#include <stdexcept>
#include <unordered_map>

void boost::test_tools::tt_detail::print_log_value<fhg::com::p2p::address_t>
  ::operator() (std::ostream& os, fhg::com::p2p::address_t const& address) const
{
  os << fhg::com::p2p::to_string_TESTING_ONLY (address);
}

void boost::test_tools::tt_detail::print_log_value<sdpa::Capability>
  ::operator() (std::ostream& os, sdpa::Capability const& capability) const
{
  os << "capability {name = " << capability.name() << "}";
}

namespace utils
{
  std::string random_peer_name()
  {
#define TEST_NO_HUMAN_READABLE_PEER_NAMES
#ifdef TEST_NO_HUMAN_READABLE_PEER_NAMES
    static fhg::util::testing::unique_random<std::string> peer_names;
    return peer_names();
#else
    static std::size_t i (0);
    return std::to_string (i++);
#endif
  }

  //! \todo unify with test/layer
  we::type::Activity module_call (std::string name)
  {
    we::type::Transition transition
      ( name
      , we::type::ModuleCall
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
      , boost::none
      , we::type::property::type()
      , we::priority_type()
      , boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    auto const port_name (fhg::util::testing::random_string());
    transition.add_port ( we::type::Port ( port_name
                                           , we::type::port::direction::In{}
                                           , std::string ("string")
                                           , we::type::property::type()
                                           )
                        );
    we::type::Activity act (transition);
    act.add_input ( port_name
                  //! \todo Investigate why we can't take a random
                  //! string with \\ or \": parse error on deserialization
                  , fhg::util::testing::random_string_without ("\\\"")
                  );
    return act;
  }

  we::type::Activity module_call()
  {
    return module_call (fhg::util::testing::random_string());
  }

  we::type::Activity net_with_one_child_requiring_workers (unsigned long count)
  {
    we::type::property::type props;
    props.set ( {"fhg", "drts", "schedule", "num_worker"}
              , boost::lexical_cast<std::string> (count) + "UL"
              );
    we::type::Transition transition
      ( fhg::util::testing::random_string()
      , we::type::ModuleCall
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
      , boost::none
      , props
      , we::priority_type()
      , boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    auto const port_name (fhg::util::testing::random_string());
    auto const port_id_in
      ( transition.add_port ( we::type::Port ( port_name
                                               , we::type::port::direction::In{}
                                               , std::string ("string")
                                               , we::type::property::type()
                                               )
                            )
      );

    we::type::net_type net;

    auto const place_id_in
      ( net.add_place
          (place::type (port_name, std::string ("string"), boost::none))
      );

    net.put_value
      (place_id_in, fhg::util::testing::random_string_without ("\\\""));

    auto const transition_id (net.add_transition (transition));

    net.add_connection ( we::edge::PT
                       , transition_id
                       , place_id_in
                       , port_id_in
                       , we::type::property::type()
                       );

    return we::type::Activity
      ( we::type::Transition ( fhg::util::testing::random_string()
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               , boost::optional<we::type::eureka_id_type>{}
                               , std::list<we::type::Preference>{}
                               )
      );
  }

  we::type::Activity net_with_two_children_requiring_n_workers (unsigned long n)
  {
    fhg::util::testing::unique_random<std::string> transition_names;

    we::type::property::type props;
    props.set ( {"fhg", "drts", "schedule", "num_worker"}
              , std::to_string (n) + "UL"
              );
    we::type::Transition transition_0
      ( transition_names()
      , we::type::ModuleCall
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
      , boost::none
      , props
      , we::priority_type()
      , boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    we::type::Transition transition_1
      ( transition_names()
      , we::type::ModuleCall
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
      , boost::none
      , props
      , we::priority_type()
      , boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    auto const port_name (fhg::util::testing::random_string());
    auto const port_id_in_0
      ( transition_0.add_port ( we::type::Port ( port_name
                                                 , we::type::port::direction::In{}
                                                 , std::string ("string")
                                                 , we::type::property::type()
                                                 )
                              )
      );
    auto const port_id_in_1
      ( transition_1.add_port ( we::type::Port ( port_name
                                                 , we::type::port::direction::In{}
                                                 , std::string ("string")
                                                 , we::type::property::type()
                                                 )
                              )
      );

    we::type::net_type net;

    auto const place_id_in_0
      ( net.add_place
          (place::type (port_name + "1", std::string ("string"), boost::none))
      );
    auto const place_id_in_1
      ( net.add_place
          (place::type (port_name + "2", std::string ("string"), boost::none))
      );

    net.put_value
      (place_id_in_0, fhg::util::testing::random_string_without ("\\\""));
    net.put_value
      (place_id_in_1, fhg::util::testing::random_string_without ("\\\""));

    auto const transition_id_0 (net.add_transition (transition_0));
    auto const transition_id_1 (net.add_transition (transition_1));

    net.add_connection ( we::edge::PT
                       , transition_id_0
                       , place_id_in_0
                       , port_id_in_0
                       , we::type::property::type()
                       );
    net.add_connection ( we::edge::PT
                       , transition_id_1
                       , place_id_in_1
                       , port_id_in_1
                       , we::type::property::type()
                       );

    return we::type::Activity
      ( we::type::Transition ( fhg::util::testing::random_string()
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               , boost::optional<we::type::eureka_id_type>{}
                               , std::list<we::type::Preference>{}
                               )
      );
  }

  fhg::logging::stdout_sink& log_to_stdout::sink()
  {
    static fhg::logging::stdout_sink _;
    return _;
  }
  log_to_stdout::log_to_stdout (sdpa::daemon::Agent& component)
  {
    sink().add_emitters_blocking ({component.logger_registration_endpoint()});
  }

  agent::agent (fhg::com::Certificates const& certificates)
    :  _ ( random_peer_name(), "127.0.0.1"
         , std::make_unique<boost::asio::io_service>()
         , boost::none
         , true
         , certificates
         )
  {}

  std::string agent::name() const
  {
    return _.name();
  }
  fhg::com::host_t agent::host() const
  {
    return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                (_.peer_local_endpoint().address())
                            );
  }
  fhg::com::port_t agent::port() const
  {
    return fhg::com::port_t (std::to_string (_.peer_local_endpoint().port()));
  }

  basic_drts_component_no_logic::basic_drts_component_no_logic
      (fhg::com::Certificates const& certificates)
    : basic_drts_component_no_logic
        (reused_component_name {random_peer_name()}, certificates)
  {}
  basic_drts_component_no_logic::basic_drts_component_no_logic
      (reused_component_name name, fhg::com::Certificates const& certificates)
    : _event_queue()
    , _name (name._name)
    , _network ( [this] ( fhg::com::p2p::address_t const& source
                        , sdpa::events::SDPAEvent::Ptr e
                        )
                 {
                   _event_queue.put (source, std::move (e));
                 }
               , std::make_unique<boost::asio::io_service>()
               , fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
               , certificates
               )
  {}

  std::string basic_drts_component_no_logic::name() const
  {
    return _name;
  }
  fhg::com::host_t basic_drts_component_no_logic::host() const
  {
    return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                (_network.local_endpoint().address())
                            );
  }
  fhg::com::port_t basic_drts_component_no_logic::port() const
  {
    return fhg::com::port_t (std::to_string (_network.local_endpoint().port()));
  }

  void basic_drts_component_no_logic::event_thread_fun()
  try
  {
    for (;;)
    {
      auto const event (_event_queue.get());
      event.second->handleBy (event.first, this);
    }
  }
  catch (decltype (_event_queue)::interrupted const&)
  {
  }

  basic_drts_component_no_logic::event_thread::event_thread
      (basic_drts_component_no_logic& component)
    : _component (component)
    , _event_thread
        (&basic_drts_component_no_logic::event_thread_fun, &component)
    , _interrupt_thread (component._event_queue)
  {}

  basic_drts_component::basic_drts_component
      (fhg::com::Certificates const& certificates)
    : basic_drts_component_no_logic (certificates)
    , _parent (boost::none)
  {}

  basic_drts_component::basic_drts_component
      ( agent const& parent
      , CapabilityNames capability_names
      , fhg::com::Certificates const& certificates
      )
    : basic_drts_component (certificates)
  {
    _parent = _network.connect_to_TESTING_ONLY (parent.host(), parent.port());

    sdpa::capabilities_set_t capabilities;
    for (auto& capability_name : capability_names)
    {
      capabilities.emplace (capability_name);
    }

    _network.perform<sdpa::events::WorkerRegistrationEvent>
      ( _parent.get()
      , _name
      , capabilities
      , fhg::util::testing::random<unsigned long>{}()
      , fhg::util::testing::random_string()
      );
  }

  basic_drts_component::basic_drts_component
      ( agent const& parent
      , sdpa::capabilities_set_t capabilities
      , fhg::com::Certificates const& certificates
      )
    : basic_drts_component (certificates)
  {
    _parent = _network.connect_to_TESTING_ONLY (parent.host(), parent.port());

    _network.perform<sdpa::events::WorkerRegistrationEvent>
      ( _parent.get()
      , _name
      , capabilities
      , fhg::util::testing::random<unsigned long>{}()
      , fhg::util::testing::random_string()
      );
  }

  basic_drts_component::basic_drts_component
      ( reused_component_name name
      , agent const& parent
      , fhg::com::Certificates const& certificates
      )
    : basic_drts_component_no_logic (name, certificates)
    , _parent (boost::none)
  {
    _parent = _network.connect_to_TESTING_ONLY (parent.host(), parent.port());

    _network.perform<sdpa::events::WorkerRegistrationEvent>
      ( _parent.get()
      , _name
      , sdpa::capabilities_set_t{}
      , fhg::util::testing::random<unsigned long>{}()
      , fhg::util::testing::random_string()
      );
  }

  basic_drts_component::~basic_drts_component()
  {
    wait_for_workers_to_shutdown();
  }

  void basic_drts_component::handle_worker_registration_response
    ( fhg::com::p2p::address_t const&
    , sdpa::events::worker_registration_response const* response
    )
  {
    response->get();
  }

  void basic_drts_component::handleWorkerRegistrationEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::WorkerRegistrationEvent const*
    )
  {
    if (!_accepted_workers.emplace (source).second)
    {
      _network.perform<sdpa::events::worker_registration_response>
        (source, std::make_exception_ptr (std::logic_error ("duplicate child")));
    }
    else
    {
      _network.perform<sdpa::events::worker_registration_response>
        (source, boost::none);
    }
  }

  void basic_drts_component::handleErrorEvent
    (fhg::com::p2p::address_t const& source, sdpa::events::ErrorEvent const* e)
  {
    if (e->error_code() == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN)
    {
      std::lock_guard<std::mutex> const _ (_mutex_workers_shutdown);
      _accepted_workers.erase (source);
      if (_accepted_workers.empty())
      {
        _cond_workers_shutdown.notify_all();
      }
    }
    else
    {
      throw std::runtime_error ("UNHANDLED EVENT: ErrorEvent");
    }
  }

  void basic_drts_component::wait_for_workers_to_shutdown()
  {
    std::unique_lock<std::mutex> lock (_mutex_workers_shutdown);
    _cond_workers_shutdown.wait
      (lock, [&] { return _accepted_workers.empty(); });
  }

  basic_drts_component::event_thread_and_worker_join::event_thread_and_worker_join (basic_drts_component& component)
    : event_thread (component)
    , _component_logic (component)
  {}
  basic_drts_component::event_thread_and_worker_join::~event_thread_and_worker_join()
  {
    _component_logic.wait_for_workers_to_shutdown();
  }

  namespace no_thread
  {
    basic_drts_worker::basic_drts_worker
        ( agent const& parent
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker (parent, {}, certificates)
    {}
    basic_drts_worker::basic_drts_worker
        ( agent const& parent
        , sdpa::capabilities_set_t capabilities
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component
          (parent, std::move (capabilities), certificates)
    {}
    basic_drts_worker::basic_drts_worker
        ( reused_component_name name
        , agent const& parent
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component (name, parent, certificates)
    {}

    fake_drts_worker_notifying_module_call_submission
      ::fake_drts_worker_notifying_module_call_submission
        ( std::function<void (std::string)> announce_job
        , agent const& parent
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker (parent, certificates)
      , _announce_job (announce_job)
    {}

    fake_drts_worker_notifying_module_call_submission
      ::fake_drts_worker_notifying_module_call_submission
        ( reused_component_name name
        , std::function<void (std::string)> announce_job
        , agent const& parent
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker (std::move (name), parent, certificates)
      , _announce_job (announce_job)
    {}

    void fake_drts_worker_notifying_module_call_submission
      ::handleSubmitJobEvent ( fhg::com::p2p::address_t const& source
                             , sdpa::events::SubmitJobEvent const* e
                             )
    {
      auto const name (add_job (e->activity(), *e->job_id(), source));

      _network.perform<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());

      announce_job (name);
    }

    void fake_drts_worker_notifying_module_call_submission::delete_job
      (sdpa::job_id_t const& job_id)
    {
      auto const job
        ( std::find_if ( _jobs.begin()
                       , _jobs.end()
                       , [&job_id] (decltype (_jobs)::value_type const name_and_job)
                         { return name_and_job.second._id == job_id; }
                       )
        );

      if (job == _jobs.end())
      {
        throw std::runtime_error ("attempted to delete unknown job!");
      }

      _jobs.erase (job);
    }

    void fake_drts_worker_notifying_module_call_submission
      ::handleJobFinishedAckEvent ( fhg::com::p2p::address_t const&
                                  , sdpa::events::JobFinishedAckEvent const* e
                                  )
    {
      delete_job (e->job_id());
    }

    sdpa::job_id_t fake_drts_worker_notifying_module_call_submission::job_id
      (std::string name)
    {
      return _jobs.at (name)._id;
    }

    std::string fake_drts_worker_notifying_module_call_submission::add_job
      ( we::type::Activity const& activity
      , sdpa::job_id_t const& job_id
      , fhg::com::p2p::address_t const& owner
      )
    {
      return _jobs.emplace
        (activity.name(), job_t {job_id, owner, activity}).first->first;
    }

    void fake_drts_worker_notifying_module_call_submission::announce_job
      (std::string const& name)
    {
      _announce_job (name);
    }

    fake_drts_worker_waiting_for_finished_ack
      ::fake_drts_worker_waiting_for_finished_ack
        ( std::function<void (std::string)> announce_job
        , agent const& parent_agent
        , fhg::com::Certificates const& certificates
        )
      : fake_drts_worker_notifying_module_call_submission
          (announce_job, parent_agent, certificates)
    {}

    fake_drts_worker_waiting_for_finished_ack
      ::fake_drts_worker_waiting_for_finished_ack
        ( reused_component_name name
        , std::function<void (std::string)> announce_job
        , agent const& parent_agent
        , fhg::com::Certificates const& certificates
        )
      : fake_drts_worker_notifying_module_call_submission
          (std::move (name), announce_job, parent_agent, certificates)
    {}

    void fake_drts_worker_waiting_for_finished_ack::handleJobFinishedAckEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::JobFinishedAckEvent const* e
      )
    {
      _finished_ack.notify (e->job_id());
      delete_job (e->job_id());
    }

    void fake_drts_worker_waiting_for_finished_ack::finish_and_wait_for_ack
      (std::string name)
    {
      auto const job (_jobs.at (name));

      _network.perform<sdpa::events::JobFinishedEvent>
        (job._owner, job._id, job._activity);

      BOOST_REQUIRE_EQUAL (_finished_ack.wait(), job._id);
    }
  }

  basic_drts_worker::basic_drts_worker
      (agent const& parent, fhg::com::Certificates const& certificates)
    : no_thread::basic_drts_worker (parent, certificates)
  {}
  basic_drts_worker::basic_drts_worker
      ( agent const& parent
      , sdpa::capabilities_set_t capabilities
      , fhg::com::Certificates const& certificates
      )
    : no_thread::basic_drts_worker (parent, std::move (capabilities), certificates)
  {}

  fake_drts_worker_notifying_module_call_submission
    ::fake_drts_worker_notifying_module_call_submission
      ( std::function<void (std::string)> announce_job
      , agent const& parent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, parent, certificates)
  {}

  fake_drts_worker_notifying_module_call_submission
    ::fake_drts_worker_notifying_module_call_submission
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& parent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_notifying_module_call_submission
        (std::move (name), announce_job, parent, certificates)
  {}

  fake_drts_worker_waiting_for_finished_ack
    ::fake_drts_worker_waiting_for_finished_ack
      ( std::function<void (std::string)> announce_job
      , agent const& parent_agent
      , fhg::com::Certificates const& certificates
      )
  : no_thread::fake_drts_worker_waiting_for_finished_ack
      (std::move (announce_job), parent_agent, certificates)
  {}

  fake_drts_worker_waiting_for_finished_ack
    ::fake_drts_worker_waiting_for_finished_ack
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& parent_agent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (std::move (name), announce_job, parent_agent, certificates)
  {}

  fake_drts_worker_notifying_cancel::fake_drts_worker_notifying_cancel
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& parent_agent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (announce_job, parent_agent, certificates)
    , _announce_cancel (announce_cancel)
  {}

  void fake_drts_worker_notifying_cancel::handleCancelJobEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::CancelJobEvent const* pEvt
    )
  {
    std::lock_guard<std::mutex> const _lock_cancels (_cancels_mutex);

    auto const job_id (pEvt->job_id());
    if ( std::find_if ( _jobs.begin()
                      , _jobs.end()
                      , [&job_id] (decltype (_jobs)::value_type const& p)
                        {
                          return job_id == p.second._id;
                        }
                      )
         == _jobs.end()
       )
    {
      throw std::runtime_error ("received cancel request for unknown job!");
    }

    _cancels.emplace (job_id, source);
    _announce_cancel (job_id);
  }

  void fake_drts_worker_notifying_cancel::canceled (std::string job_id)
  {
    std::lock_guard<std::mutex> const _lock_cancels (_cancels_mutex);

    auto const parent (_cancels.at (job_id));
    _cancels.erase (job_id);

    _network.perform<sdpa::events::CancelJobAckEvent> (parent, job_id);
    delete_job (job_id);
  }

  fake_drts_worker_notifying_cancel_but_never_replying
    ::fake_drts_worker_notifying_cancel_but_never_replying
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& parent_agent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (announce_job, parent_agent, certificates)
    , _announce_cancel (announce_cancel)
  {}

  void fake_drts_worker_notifying_cancel_but_never_replying
    ::handleCancelJobEvent ( fhg::com::p2p::address_t const&
                           , sdpa::events::CancelJobEvent const* pEvt
                           )
  {
    _announce_cancel (pEvt->job_id());
  }

  client::client ( agent const& parent_agent
                 , fhg::com::Certificates const& certificates
                 )
    : _ ( parent_agent.host()
        , parent_agent.port()
        , std::make_unique<boost::asio::io_service>()
        , certificates
        )
  {}

  sdpa::job_id_t client::submit_job (we::type::Activity workflow)
  {
    return _.submitJob (workflow);
  }

  sdpa::status::code client::wait_for_terminal_state (sdpa::job_id_t const& id)
  {
    sdpa::client::job_info_t UNUSED_job_info;
    return wait_for_terminal_state (id, UNUSED_job_info);
  }

  sdpa::status::code client::wait_for_terminal_state
    (sdpa::job_id_t const& id, sdpa::client::job_info_t& job_info)
  {
    return _.wait_for_terminal_state (id, job_info);
  }

  sdpa::status::code client::wait_for_terminal_state_and_cleanup
    (sdpa::job_id_t const& id)
  {
    auto const ret (wait_for_terminal_state (id));
    delete_job (id);
    return ret;
  }

  void client::delete_job (sdpa::job_id_t const& id)
  {
    return _.deleteJob (id);
  }

  void client::cancel_job (sdpa::job_id_t const& id)
  {
    return _.cancelJob (id);
  }
}
