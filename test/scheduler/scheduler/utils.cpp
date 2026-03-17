// Copyright (C) 2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/scheduler/scheduler/utils.hpp>

#include <gspc/scheduler/events/CancelJobEvent.hpp>
#include <gspc/scheduler/events/ErrorEvent.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/testing/printer/generic.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/util/threadsafe_queue.hpp>

#include <boost/asio/io_service.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iterator>
#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

void ::boost::test_tools::tt_detail::print_log_value<gspc::scheduler::Capability>
  ::operator() (std::ostream& os, gspc::scheduler::Capability const& capability) const
{
  os << "capability {name = " << capability.name() << "}";
}

namespace utils
{
  std::string random_peer_name()
  {
#define TEST_NO_HUMAN_READABLE_PEER_NAMES
#ifdef TEST_NO_HUMAN_READABLE_PEER_NAMES
    static gspc::testing::unique_random<std::string> peer_names;
    return peer_names();
#else
    static std::size_t i (0);
    return std::to_string (i++);
#endif
  }

  //! \todo unify with test/layer
  gspc::we::type::Activity module_call (std::string name)
  {
    gspc::we::type::Transition transition
      ( name
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , gspc::we::type::property::type()
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    auto const port_name (gspc::testing::random_string());
    transition.add_port ( gspc::we::type::Port ( port_name
                                           , gspc::we::type::port::direction::In{}
                                           , std::string ("string")
                                           , gspc::we::type::property::type()
                                           )
                        );
    gspc::we::type::Activity act (transition);
    act.add_input ( port_name
                  //! \todo Investigate why we can't take a random
                  //! string with \\ or \": parse error on deserialization
                  , gspc::testing::random_string_without ("\\\"")
                  );
    return act;
  }

  gspc::we::type::Activity module_call()
  {
    return module_call (gspc::testing::random_string());
  }

  gspc::we::type::Activity module_call (std::string name, bool might_use_multiple_workers)
  {
    gspc::we::type::property::type properties;

    if (might_use_multiple_workers)
    {
      properties.set
        ({"fhg", "drts", "schedule", "num_worker"}, std::to_string (1) + "UL");
    }

    gspc::we::type::Transition transition
      ( name
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , properties
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    auto const port_name (gspc::testing::random_string());
    transition.add_port ( gspc::we::type::Port ( port_name
                                           , gspc::we::type::port::direction::In{}
                                           , std::string ("string")
                                           , gspc::we::type::property::type()
                                           )
                        );
    gspc::we::type::Activity act (transition);
    act.add_input ( port_name
                  //! \todo Investigate why we can't take a random
                  //! string with \\ or \": parse error on deserialization
                  , gspc::testing::random_string_without ("\\\"")
                  );
    return act;
  }

  gspc::we::type::Activity module_call_with_max_num_retries
    (unsigned long maximum_number_of_retries)
  {
    gspc::we::type::property::type properties;

    properties.set
      ( {"fhg", "drts", "schedule", "maximum_number_of_retries"}
      , std::to_string (maximum_number_of_retries) + "UL"
      );

    gspc::we::type::Transition transition
      ( gspc::testing::random_string()
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , properties
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    auto const port_name (gspc::testing::random_string());
    transition.add_port ( gspc::we::type::Port ( port_name
                                         , gspc::we::type::port::direction::In{}
                                         , std::string ("string")
                                         , gspc::we::type::property::type()
                                         )
                        );
    gspc::we::type::Activity activity (transition);
    activity.add_input
      (port_name, gspc::testing::random_string_without ("\\\""));
    return activity;
  }

  gspc::we::type::Activity net_with_one_child_requiring_workers (unsigned long count)
  {
    gspc::we::type::property::type props;
    props.set ( {"fhg", "drts", "schedule", "num_worker"}
              , std::to_string (count) + "UL"
              );
    gspc::we::type::Transition transition
      ( gspc::testing::random_string()
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , props
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    auto const port_name (gspc::testing::random_string());
    auto const port_id_in
      ( transition.add_port ( gspc::we::type::Port ( port_name
                                               , gspc::we::type::port::direction::In{}
                                               , std::string ("string")
                                               , gspc::we::type::property::type()
                                               )
                            )
      );

    gspc::we::type::net_type net;

    auto const place_id_in
      ( net.add_place
          (gspc::we::type::place::type ( port_name
                       , std::string ("string")
                       , std::nullopt
                       , std::nullopt
                       , gspc::we::type::property::type{}
                       , gspc::we::type::place::type::Generator::No{}
                       )
          )
      );

    net.put_value
      (place_id_in, gspc::testing::random_string_without ("\\\""));

    auto const transition_id (net.add_transition (transition));

    net.add_connection ( gspc::we::edge::PT{}
                       , transition_id
                       , place_id_in
                       , port_id_in
                       , gspc::we::type::property::type()
                       );

    return gspc::we::type::Activity
      ( gspc::we::type::Transition ( gspc::testing::random_string()
                               , net
                               , std::nullopt
                               , gspc::we::type::property::type()
                               , gspc::we::priority_type()
                               , std::optional<gspc::we::type::eureka_id_type>{}
                               , std::list<gspc::we::type::Preference>{}
                               , gspc::we::type::track_shared{}
                               )
      );
  }

  gspc::we::type::Activity net_with_one_child_requiring_workers_and_num_retries
    (unsigned long count, unsigned long maximum_number_of_retries)
  {
    gspc::we::type::property::type properties;
    properties.set
      ( {"fhg", "drts", "schedule", "num_worker"}
        , std::to_string (count) + "UL"
      );

    properties.set
      ( {"fhg", "drts", "schedule", "maximum_number_of_retries"}
      , std::to_string (maximum_number_of_retries) + "UL"
      );

    gspc::we::type::Transition transition
      ( gspc::testing::random_string()
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , properties
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    auto const port_name (gspc::testing::random_string());
    auto const port_id_in
      ( transition.add_port ( gspc::we::type::Port ( port_name
                                               , gspc::we::type::port::direction::In{}
                                               , std::string ("string")
                                               , gspc::we::type::property::type()
                                               )
                            )
      );

    gspc::we::type::net_type net;

    auto const place_id_in
      ( net.add_place
          (gspc::we::type::place::type ( port_name
                       , std::string ("string")
                       , std::nullopt
                       , std::nullopt
                       , gspc::we::type::property::type{}
                       , gspc::we::type::place::type::Generator::No{}
                       )
          )
      );

    net.put_value
      (place_id_in, gspc::testing::random_string_without ("\\\""));

    auto const transition_id (net.add_transition (transition));

    net.add_connection ( gspc::we::edge::PT{}
                       , transition_id
                       , place_id_in
                       , port_id_in
                       , gspc::we::type::property::type()
                       );

    return gspc::we::type::Activity
      ( gspc::we::type::Transition ( gspc::testing::random_string()
                             , net
                             , std::nullopt
                             , gspc::we::type::property::type()
                             , gspc::we::priority_type()
                             , std::optional<gspc::we::type::eureka_id_type>{}
                             , std::list<gspc::we::type::Preference>{}
                             , gspc::we::type::track_shared{}
                             )
      );
  }

  gspc::we::type::Activity net_with_two_children_requiring_n_workers (unsigned long n)
  {
    gspc::testing::unique_random<std::string> transition_names;

    gspc::we::type::property::type props;
    props.set ( {"fhg", "drts", "schedule", "num_worker"}
              , std::to_string (n) + "UL"
              );
    gspc::we::type::Transition transition_0
      ( transition_names()
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , props
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    gspc::we::type::Transition transition_1
      ( transition_names()
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , props
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    auto const port_name (gspc::testing::random_string());
    auto const port_id_in_0
      ( transition_0.add_port ( gspc::we::type::Port ( port_name
                                                 , gspc::we::type::port::direction::In{}
                                                 , std::string ("string")
                                                 , gspc::we::type::property::type()
                                                 )
                              )
      );
    auto const port_id_in_1
      ( transition_1.add_port ( gspc::we::type::Port ( port_name
                                                 , gspc::we::type::port::direction::In{}
                                                 , std::string ("string")
                                                 , gspc::we::type::property::type()
                                                 )
                              )
      );

    gspc::we::type::net_type net;

    auto const place_id_in_0
      ( net.add_place
          (gspc::we::type::place::type ( port_name + "1"
                       , std::string ("string")
                       , std::nullopt
                       , std::nullopt
                       , gspc::we::type::property::type{}
                       , gspc::we::type::place::type::Generator::No{}
                       )
          )
      );
    auto const place_id_in_1
      ( net.add_place
          (gspc::we::type::place::type ( port_name + "2"
                       , std::string ("string")
                       , std::nullopt
                       , std::nullopt
                       , gspc::we::type::property::type{}
                       , gspc::we::type::place::type::Generator::No{}
                       )
          )
      );

    net.put_value
      (place_id_in_0, gspc::testing::random_string_without ("\\\""));
    net.put_value
      (place_id_in_1, gspc::testing::random_string_without ("\\\""));

    auto const transition_id_0 (net.add_transition (transition_0));
    auto const transition_id_1 (net.add_transition (transition_1));

    net.add_connection ( gspc::we::edge::PT{}
                       , transition_id_0
                       , place_id_in_0
                       , port_id_in_0
                       , gspc::we::type::property::type()
                       );
    net.add_connection ( gspc::we::edge::PT{}
                       , transition_id_1
                       , place_id_in_1
                       , port_id_in_1
                       , gspc::we::type::property::type()
                       );

    return gspc::we::type::Activity
      ( gspc::we::type::Transition ( gspc::testing::random_string()
                               , net
                               , std::nullopt
                               , gspc::we::type::property::type()
                               , gspc::we::priority_type()
                               , std::optional<gspc::we::type::eureka_id_type>{}
                               , std::list<gspc::we::type::Preference>{}
                               , gspc::we::type::track_shared{}
                               )
      );
  }

  gspc::logging::stdout_sink& log_to_stdout::sink()
  {
    static gspc::logging::stdout_sink _;
    return _;
  }
  log_to_stdout::log_to_stdout (gspc::scheduler::daemon::Agent& component)
  {
    sink().add_emitters_blocking ({component.logger_registration_endpoint()});
  }

  agent::agent (gspc::Certificates const& certificates)
    :  _ ( random_peer_name(), "127.0.0.1"
         , std::make_unique<::boost::asio::io_service>()
         , std::nullopt
         , certificates
         )
  {}

  std::string agent::name() const
  {
    return _.name();
  }
  gspc::com::host_t agent::host() const
  {
    return gspc::com::host_t ( gspc::util::connectable_to_address_string
                                (_.peer_local_endpoint().address())
                            );
  }
  gspc::com::port_t agent::port() const
  {
    return gspc::com::port_t {_.peer_local_endpoint().port()};
  }

  basic_drts_component_no_logic::basic_drts_component_no_logic
      (gspc::Certificates const& certificates)
    : basic_drts_component_no_logic
        (reused_component_name {random_peer_name()}, certificates)
  {}
  basic_drts_component_no_logic::basic_drts_component_no_logic
      (reused_component_name name, gspc::Certificates const& certificates)
    : _event_queue()
    , _name (name._name)
    , _network ( [this] ( gspc::com::p2p::address_t const& source
                        , gspc::scheduler::events::SchedulerEvent::Ptr e
                        )
                 {
                   _event_queue.put (source, std::move (e));
                 }
               , std::make_unique<::boost::asio::io_service>()
               , gspc::com::host_t ("127.0.0.1"), gspc::com::port_t (0)
               , certificates
               )
  {}

  std::string basic_drts_component_no_logic::name() const
  {
    return _name;
  }
  gspc::com::host_t basic_drts_component_no_logic::host() const
  {
    return gspc::com::host_t ( gspc::util::connectable_to_address_string
                                (_network.local_endpoint().address())
                            );
  }
  gspc::com::port_t basic_drts_component_no_logic::port() const
  {
    return gspc::com::port_t {_network.local_endpoint().port()};
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
  catch (decltype (_event_queue)::interrupted const& interrupted)
  {
    std::ignore = interrupted;
  }

  basic_drts_component_no_logic::event_thread::event_thread
      (basic_drts_component_no_logic& component)
    : _component (component)
    , _event_thread
        (&basic_drts_component_no_logic::event_thread_fun, &component)
    , _interrupt_thread (component._event_queue)
  {}

  basic_drts_component::basic_drts_component
      (gspc::Certificates const& certificates)
    : basic_drts_component_no_logic (certificates)
    , _parent (std::nullopt)
  {}

  basic_drts_component::basic_drts_component
      ( agent const& parent
      , CapabilityNames capability_names
      , gspc::Certificates const& certificates
      )
    : basic_drts_component (certificates)
  {
    _parent = _network.connect_to_TESTING_ONLY (parent.host(), parent.port());

    gspc::scheduler::Capabilities capabilities;
    for (auto& capability_name : capability_names)
    {
      capabilities.emplace (capability_name);
    }

    _network.perform<gspc::scheduler::events::WorkerRegistrationEvent>
      ( _parent.value()
      , _name
      , capabilities
      , gspc::testing::random<unsigned long>{}()
      , gspc::testing::random_string()
      );
  }

  basic_drts_component::basic_drts_component
      ( agent const& parent
      , gspc::scheduler::Capabilities capabilities
      , gspc::Certificates const& certificates
      )
    : basic_drts_component (certificates)
  {
    _parent = _network.connect_to_TESTING_ONLY (parent.host(), parent.port());

    _network.perform<gspc::scheduler::events::WorkerRegistrationEvent>
      ( _parent.value()
      , _name
      , capabilities
      , gspc::testing::random<unsigned long>{}()
      , gspc::testing::random_string()
      );
  }

  basic_drts_component::basic_drts_component
      ( reused_component_name name
      , agent const& parent
      , gspc::Certificates const& certificates
      )
    : basic_drts_component_no_logic (name, certificates)
    , _parent (std::nullopt)
  {
    _parent = _network.connect_to_TESTING_ONLY (parent.host(), parent.port());

    _network.perform<gspc::scheduler::events::WorkerRegistrationEvent>
      ( _parent.value()
      , _name
      , gspc::scheduler::Capabilities{}
      , gspc::testing::random<unsigned long>{}()
      , gspc::testing::random_string()
      );
  }

  basic_drts_component::~basic_drts_component()
  {
    wait_for_workers_to_shutdown();
  }

  void basic_drts_component::handle_worker_registration_response
    ( gspc::com::p2p::address_t const&
    , gspc::scheduler::events::worker_registration_response const* response
    )
  {
    _registration_responses.put (response->exception());
  }

  void basic_drts_component::wait_for_registration()
  {
    auto const exception (_registration_responses.get());
    if (exception.has_value())
    {
      std::rethrow_exception (exception.value());
    }
  }

  void basic_drts_component::handleWorkerRegistrationEvent
    ( gspc::com::p2p::address_t const& source
    , gspc::scheduler::events::WorkerRegistrationEvent const*
    )
  {
    if (!_accepted_workers.emplace (source).second)
    {
      _network.perform<gspc::scheduler::events::worker_registration_response>
        (source, std::make_exception_ptr (std::logic_error ("duplicate child")));
    }
    else
    {
      _network.perform<gspc::scheduler::events::worker_registration_response>
        (source, std::nullopt);
    }
  }

  void basic_drts_component::handleErrorEvent
    (gspc::com::p2p::address_t const& source, gspc::scheduler::events::ErrorEvent const* e)
  {
    if (e->error_code() == gspc::scheduler::events::ErrorEvent::SCHEDULER_ENODE_SHUTDOWN)
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
  {
    if (component._parent)
    {
      component.wait_for_registration();
    }
  }
  basic_drts_component::event_thread_and_worker_join::~event_thread_and_worker_join()
  {
    _component_logic.wait_for_workers_to_shutdown();
  }

  namespace no_thread
  {
    basic_drts_worker::basic_drts_worker
        ( agent const& parent
        , gspc::Certificates const& certificates
        )
      : basic_drts_worker (parent, {}, certificates)
    {}
    basic_drts_worker::basic_drts_worker
        ( agent const& parent
        , gspc::scheduler::Capabilities capabilities
        , gspc::Certificates const& certificates
        )
      : basic_drts_component
          (parent, std::move (capabilities), certificates)
    {}
    basic_drts_worker::basic_drts_worker
        ( reused_component_name name
        , agent const& parent
        , gspc::Certificates const& certificates
        )
      : basic_drts_component (name, parent, certificates)
    {}

    fake_drts_worker_notifying_module_call_submission
      ::fake_drts_worker_notifying_module_call_submission
        ( std::function<void (std::string)> announce_job
        , agent const& parent
        , gspc::Certificates const& certificates
        )
      : basic_drts_worker (parent, certificates)
      , _announce_job (announce_job)
    {}

    fake_drts_worker_notifying_module_call_submission
      ::fake_drts_worker_notifying_module_call_submission
        ( reused_component_name name
        , std::function<void (std::string)> announce_job
        , agent const& parent
        , gspc::Certificates const& certificates
        )
      : basic_drts_worker (std::move (name), parent, certificates)
      , _announce_job (announce_job)
    {}

    void fake_drts_worker_notifying_module_call_submission
      ::handleSubmitJobEvent ( gspc::com::p2p::address_t const& source
                             , gspc::scheduler::events::SubmitJobEvent const* e
                             )
    {
      auto const name (add_job (e->activity(), *e->job_id(), source));

      _network.perform<gspc::scheduler::events::SubmitJobAckEvent> (source, *e->job_id());

      announce_job (name);
    }

    void fake_drts_worker_notifying_module_call_submission::delete_job
      (gspc::scheduler::job_id_t const& job_id)
    {
      if (_jobs.erase (job_id) == 0)
      {
        throw std::runtime_error ("attempted to delete unknown job!");
      }
    }

    void fake_drts_worker_notifying_module_call_submission
      ::handleJobFinishedAckEvent ( gspc::com::p2p::address_t const&
                                  , gspc::scheduler::events::JobFinishedAckEvent const* e
                                  )
    {
      delete_job (e->job_id());
    }

    std::string fake_drts_worker_notifying_module_call_submission::add_job
      ( gspc::we::type::Activity const& activity
      , gspc::scheduler::job_id_t const& job_id
      , gspc::com::p2p::address_t const& owner
      )
    {
      _jobs.insert_or_assign (job_id, job_t {job_id, owner, activity});

      return job_id;
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
        , gspc::Certificates const& certificates
        )
      : fake_drts_worker_notifying_module_call_submission
          (announce_job, parent_agent, certificates)
    {}

    fake_drts_worker_waiting_for_finished_ack
      ::fake_drts_worker_waiting_for_finished_ack
        ( reused_component_name name
        , std::function<void (std::string)> announce_job
        , agent const& parent_agent
        , gspc::Certificates const& certificates
        )
      : fake_drts_worker_notifying_module_call_submission
          (std::move (name), announce_job, parent_agent, certificates)
    {}

    void fake_drts_worker_waiting_for_finished_ack::handleJobFinishedAckEvent
      ( gspc::com::p2p::address_t const&
      , gspc::scheduler::events::JobFinishedAckEvent const* e
      )
    {
      _finished_ack.put (e->job_id());
      delete_job (e->job_id());
    }

    void fake_drts_worker_waiting_for_finished_ack::finish_and_wait_for_ack
      (std::string name)
    {
      auto const job (_jobs.at (name));

      _network.perform<gspc::scheduler::events::JobFinishedEvent>
        (job._owner, job._id, job._activity);

      BOOST_REQUIRE_EQUAL (_finished_ack.get(), job._id);
    }
  }

  basic_drts_worker::basic_drts_worker
      (agent const& parent, gspc::Certificates const& certificates)
    : no_thread::basic_drts_worker (parent, certificates)
  {}
  basic_drts_worker::basic_drts_worker
      ( agent const& parent
      , gspc::scheduler::Capabilities capabilities
      , gspc::Certificates const& certificates
      )
    : no_thread::basic_drts_worker (parent, std::move (capabilities), certificates)
  {}

  fake_drts_worker_notifying_module_call_submission
    ::fake_drts_worker_notifying_module_call_submission
      ( std::function<void (std::string)> announce_job
      , agent const& parent
      , gspc::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, parent, certificates)
  {}

  fake_drts_worker_notifying_module_call_submission
    ::fake_drts_worker_notifying_module_call_submission
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& parent
      , gspc::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_notifying_module_call_submission
        (std::move (name), announce_job, parent, certificates)
  {}

  fake_drts_worker_waiting_for_finished_ack
    ::fake_drts_worker_waiting_for_finished_ack
      ( std::function<void (std::string)> announce_job
      , agent const& parent_agent
      , gspc::Certificates const& certificates
      )
  : no_thread::fake_drts_worker_waiting_for_finished_ack
      (std::move (announce_job), parent_agent, certificates)
  {}

  fake_drts_worker_waiting_for_finished_ack
    ::fake_drts_worker_waiting_for_finished_ack
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& parent_agent
      , gspc::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (std::move (name), announce_job, parent_agent, certificates)
  {}

  fake_drts_worker_notifying_cancel::fake_drts_worker_notifying_cancel
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& parent_agent
      , gspc::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (announce_job, parent_agent, certificates)
    , _announce_cancel (announce_cancel)
  {}

  void fake_drts_worker_notifying_cancel::handleCancelJobEvent
    ( gspc::com::p2p::address_t const& source
    , gspc::scheduler::events::CancelJobEvent const* pEvt
    )
  {
    std::lock_guard<std::mutex> const _lock_cancels (_cancels_mutex);

    auto const job_id (pEvt->job_id());
    if (_jobs.find (job_id) == _jobs.end())
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

    _network.perform<gspc::scheduler::events::CancelJobAckEvent> (parent, job_id);
    delete_job (job_id);
  }

  fake_drts_worker_notifying_cancel_but_never_replying
    ::fake_drts_worker_notifying_cancel_but_never_replying
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& parent_agent
      , gspc::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (announce_job, parent_agent, certificates)
    , _announce_cancel (announce_cancel)
  {}

  void fake_drts_worker_notifying_cancel_but_never_replying
    ::handleCancelJobEvent ( gspc::com::p2p::address_t const&
                           , gspc::scheduler::events::CancelJobEvent const* pEvt
                           )
  {
    _announce_cancel (pEvt->job_id());
  }

  client::client ( agent const& parent_agent
                 , gspc::Certificates const& certificates
                 )
    : _ ( parent_agent.host()
        , parent_agent.port()
        , std::make_unique<::boost::asio::io_service>()
        , certificates
        )
  {}

  gspc::scheduler::job_id_t client::submit_job (gspc::we::type::Activity workflow)
  {
    return _.submitJob (workflow);
  }

  gspc::scheduler::status::code client::wait_for_terminal_state (gspc::scheduler::job_id_t const& id)
  {
    gspc::scheduler::client::job_info_t UNUSED_job_info;
    return wait_for_terminal_state (id, UNUSED_job_info);
  }

  gspc::scheduler::status::code client::wait_for_terminal_state
    (gspc::scheduler::job_id_t const& id, gspc::scheduler::client::job_info_t& job_info)
  {
    return _.wait_for_terminal_state (id, job_info);
  }

  gspc::scheduler::status::code client::wait_for_terminal_state_and_cleanup
    (gspc::scheduler::job_id_t const& id)
  {
    auto const ret (wait_for_terminal_state (id));
    delete_job (id);
    return ret;
  }

  void client::delete_job (gspc::scheduler::job_id_t const& id)
  {
    return _.deleteJob (id);
  }

  void client::cancel_job (gspc::scheduler::job_id_t const& id)
  {
    return _.cancelJob (id);
  }

  gspc::we::type::Activity client::result (gspc::scheduler::job_id_t const& job) const
  {
    return _.result (job);
  }
}
