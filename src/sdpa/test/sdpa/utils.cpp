#include <sdpa/test/sdpa/utils.hpp>

#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/master_network_info.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
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
  os << fhg::com::p2p::to_string (address);
}

namespace utils
{
  std::string require_and_read_file (std::string filename)
  {
    std::ifstream f (filename.c_str());
    BOOST_REQUIRE (f.is_open());

    std::noskipws (f);

    return std::string ( std::istream_iterator<char> (f)
                       , std::istream_iterator<char>()
                       );
  }

  std::string random_peer_name()
  {
    static std::size_t i (0);
    return boost::lexical_cast<std::string> (i++)
#define TEST_NO_HUMAN_READABLE_PEER_NAMES
#ifdef TEST_NO_HUMAN_READABLE_PEER_NAMES
    + fhg::util::testing::random_string()
#endif
      ;
  }

  //! \todo unify with test/layer
  we::type::activity_t module_call (std::string name)
  {
    we::type::transition_t transition
      ( name
      , we::type::module_call_t
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::memory_buffer_info_t>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          )
      , boost::none
      , we::type::property::type()
      , we::priority_type()
      );
    auto const port_name (fhg::util::testing::random_string());
    transition.add_port ( we::type::port_t ( port_name
                                           , we::type::PORT_IN
                                           , std::string ("string")
                                           , we::type::property::type()
                                           )
                        );
    we::type::activity_t act (transition, boost::none);
    act.add_input ( transition.input_port_by_name (port_name)
                  //! \todo Investigate why we can't take a random
                  //! string with \\ or \": parse error on deserialization
                  , fhg::util::testing::random_string_without ("\\\"")
                  );
    return act;
  }

  we::type::activity_t module_call()
  {
    return module_call (fhg::util::testing::random_string());
  }

  we::type::activity_t net_with_one_child_requiring_workers (unsigned long count)
  {
    we::type::property::type props;
    props.set ( {"fhg", "drts", "schedule", "num_worker"}
              , boost::lexical_cast<std::string> (count) + "UL"
              );
    we::type::transition_t transition
      ( fhg::util::testing::random_string()
      , we::type::module_call_t
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::memory_buffer_info_t>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          )
      , boost::none
      , props
      , we::priority_type()
      );
    auto const port_name (fhg::util::testing::random_string());
    auto const port_id_in
      ( transition.add_port ( we::type::port_t ( port_name
                                               , we::type::PORT_IN
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

    return we::type::activity_t
      ( we::type::transition_t ( fhg::util::testing::random_string()
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , boost::none
      );
  }

  we::type::activity_t net_with_two_children_requiring_n_workers (unsigned long n)
  {
    we::type::property::type props;
    props.set ( {"fhg", "drts", "schedule", "num_worker"}
              , std::to_string (n) + "UL"
              );
    we::type::transition_t transition_0
      ( fhg::util::testing::random_string()
      , we::type::module_call_t
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::memory_buffer_info_t>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          )
      , boost::none
      , props
      , we::priority_type()
      );
    we::type::transition_t transition_1
      ( fhg::util::testing::random_string()
      , we::type::module_call_t
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::memory_buffer_info_t>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          )
      , boost::none
      , props
      , we::priority_type()
      );
    auto const port_name (fhg::util::testing::random_string());
    auto const port_id_in_0
      ( transition_0.add_port ( we::type::port_t ( port_name
                                                 , we::type::PORT_IN
                                                 , std::string ("string")
                                                 , we::type::property::type()
                                                 )
                              )
      );
    auto const port_id_in_1
      ( transition_1.add_port ( we::type::port_t ( port_name
                                                 , we::type::PORT_IN
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

    return we::type::activity_t
      ( we::type::transition_t ( fhg::util::testing::random_string()
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , boost::none
      );
  }

  fhg::logging::stdout_sink& log_to_stdout::sink()
  {
    static fhg::logging::stdout_sink _;
    return _;
  }
  log_to_stdout::log_to_stdout (sdpa::daemon::GenericDaemon& component)
  {
    sink().add_emitters ({component.logger_registration_endpoint()});
  }

  orchestrator::orchestrator (fhg::com::Certificates const& certificates)
    : _ ( random_peer_name(), "127.0.0.1"
        , fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , boost::none
        , sdpa::master_info_t()
        , false
        , certificates
        )
  {}

  std::string orchestrator::name() const
  {
    return _.name();
  }
  fhg::com::host_t orchestrator::host() const
  {
    return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                (_.peer_local_endpoint().address())
                            );
  }
  fhg::com::port_t orchestrator::port() const
  {
    return fhg::com::port_t (std::to_string (_.peer_local_endpoint().port()));
  }

  namespace
  {
    template<typename Master>
      sdpa::master_network_info make_master_network_info (Master const& master)
    {
      return {master.host(), master.port()};
    }
  }

  agent::agent ( basic_drts_component const& master
               , fhg::com::Certificates const& certificates
               )
    : _ ( random_peer_name(), "127.0.0.1"
        , fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , boost::none
        , {make_master_network_info (master)}
        , true
        , certificates
        )
  {}

  agent::agent ( orchestrator const& master
               , fhg::com::Certificates const& certificates
               )
    : _ ( random_peer_name(), "127.0.0.1"
        , fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , boost::none
        , {make_master_network_info (master)}
        , true
        , certificates
        )
  {}

  agent::agent (agent const& master, fhg::com::Certificates const& certificates)
    : _ ( random_peer_name(), "127.0.0.1"
        , fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , boost::none
        , {make_master_network_info (master)}
        , true
        , certificates
        )
  {}

  agent::agent ( agent const& master_0
               , agent const& master_1
               , fhg::com::Certificates const& certificates
               )
    : _ ( random_peer_name(), "127.0.0.1"
        , fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , boost::none
        , { make_master_network_info (master_0)
          , make_master_network_info (master_1)
          }
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

  basic_drts_component::basic_drts_component
      ( std::string name
      , bool accept_workers
      , fhg::com::Certificates const& certificates
      )
    : _name (name)
    , _master (boost::none)
    , _accept_workers (accept_workers)
    , _event_queue()
    , _network ( [this] ( fhg::com::p2p::address_t const& source
                        , sdpa::events::SDPAEvent::Ptr e
                        )
                 {
                   _event_queue.put (source, e);
                 }
               , fhg::util::cxx14::make_unique<boost::asio::io_service>()
               , fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0"), certificates
               )
  {}

  basic_drts_component::basic_drts_component
      ( std::string name
      , agent const& master
      , sdpa::capabilities_set_t capabilities
      , bool accept_workers
      , fhg::com::Certificates const& certificates
      )
    : basic_drts_component (name, accept_workers, certificates)
  {
    _master = _network.connect_to (master.host(), master.port());

    _network.perform<sdpa::events::WorkerRegistrationEvent>
      ( _master.get()
      , _name
      , capabilities
      , fhg::util::testing::random<unsigned long>{}()
      , accept_workers
      , fhg::util::testing::random_string()
      );
  }

  basic_drts_component::~basic_drts_component()
  {
    wait_for_workers_to_shutdown();
  }

  void basic_drts_component::handle_worker_registration_response
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::worker_registration_response const* response
    )
  {
    BOOST_REQUIRE (_master);
    BOOST_REQUIRE_EQUAL (source, _master.get());

    response->get();
  }

  void basic_drts_component::handleWorkerRegistrationEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::WorkerRegistrationEvent const*
    )
  {
    BOOST_REQUIRE (_accept_workers);
    BOOST_REQUIRE (_accepted_workers.insert (source).second);

    _network.perform<sdpa::events::worker_registration_response>
      (source, boost::none);
  }

  void basic_drts_component::handleErrorEvent
    (fhg::com::p2p::address_t const& source, sdpa::events::ErrorEvent const* e)
  {
    if ( e->error_code() == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
      || e->error_code() == sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
       )
    {
      BOOST_REQUIRE (_accept_workers);
      std::lock_guard<std::mutex> const _ (_mutex_workers_shutdown);
      BOOST_REQUIRE (_accepted_workers.erase (source));
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

  std::string basic_drts_component::name() const
  {
    return _name;
  }
  fhg::com::host_t basic_drts_component::host() const
  {
    return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                (_network.local_endpoint().address())
                            );
  }
  fhg::com::port_t basic_drts_component::port() const
  {
    return fhg::com::port_t (std::to_string (_network.local_endpoint().port()));
  }

  void basic_drts_component::event_thread()
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

  basic_drts_component::event_thread_and_worker_join::event_thread_and_worker_join (basic_drts_component& component)
    : _component (component)
    , _event_thread (&basic_drts_component::event_thread, &component)
    , _interrupt_thread (component._event_queue)
  {}
  basic_drts_component::event_thread_and_worker_join::~event_thread_and_worker_join()
  {
    _component.wait_for_workers_to_shutdown();
  }

  namespace no_thread
  {
    basic_drts_worker::basic_drts_worker
        ( agent const& master
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker ( random_peer_name()
                          , master
                          , certificates
                          )
    {}
    basic_drts_worker::basic_drts_worker
        ( std::string name
        , agent const& master
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker ( name
                          , master
                          , sdpa::capabilities_set_t()
                          , certificates
                          )
    {}
    basic_drts_worker::basic_drts_worker
        ( std::string name
        , agent const& master
        , sdpa::capabilities_set_t capabilities
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component
          (name, master, capabilities, false, certificates)
    {}

    fake_drts_worker_notifying_module_call_submission
      ::fake_drts_worker_notifying_module_call_submission
        ( std::function<void (std::string)> announce_job
        , agent const& master
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker (master, certificates)
      , _announce_job (announce_job)
    {}
    fake_drts_worker_notifying_module_call_submission
      ::fake_drts_worker_notifying_module_call_submission
        ( std::string name
        , std::function<void (std::string)> announce_job
        , agent const& master
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_worker (name, master, certificates)
      , _announce_job (announce_job)
    {}

    void fake_drts_worker_notifying_module_call_submission
      ::handleSubmitJobEvent ( fhg::com::p2p::address_t const& source
                             , sdpa::events::SubmitJobEvent const* e
                             )
    {
      auto const name (e->activity().transition().name());

      add_job (name, *e->job_id(), source);

      _network.perform<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());

      announce_job (name);
    }
    void fake_drts_worker_notifying_module_call_submission
      ::handleJobFinishedAckEvent ( fhg::com::p2p::address_t const&
                                  , sdpa::events::JobFinishedAckEvent const*
                                  )
    {
      // can be ignored as we clean up in finish() already
    }

    void fake_drts_worker_notifying_module_call_submission::finish
      (std::string name)
    {
      auto const job (_jobs.at (name));
      _jobs.erase (name);

      _network.perform<sdpa::events::JobFinishedEvent>
        (job._owner, job._id, we::type::activity_t());
    }

    sdpa::job_id_t fake_drts_worker_notifying_module_call_submission::job_id
      (std::string name)
    {
      return _jobs.at (name)._id;
    }

    void fake_drts_worker_notifying_module_call_submission::add_job
      ( std::string const& name
      , sdpa::job_id_t const& job_id
      , fhg::com::p2p::address_t const& owner
      )
    {
      _jobs.emplace (name, job_t (job_id, owner));
    }

    void fake_drts_worker_notifying_module_call_submission::announce_job
      (std::string const& name)
    {
      _announce_job (name);
    }

    fake_drts_worker_notifying_module_call_submission::job_t::job_t
        (sdpa::job_id_t id, fhg::com::p2p::address_t owner)
      : _id (id)
      , _owner (owner)
    {}

    fake_drts_worker_waiting_for_finished_ack
      ::fake_drts_worker_waiting_for_finished_ack
        ( std::function<void (std::string)> announce_job
        , agent const& master_agent
        , fhg::com::Certificates const& certificates
        )
      : fake_drts_worker_notifying_module_call_submission
          (announce_job, master_agent, certificates)
    {}

    void fake_drts_worker_waiting_for_finished_ack::handleJobFinishedAckEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::JobFinishedAckEvent const* e
      )
    {
      _finished_ack.notify (e->job_id());
    }

    void fake_drts_worker_waiting_for_finished_ack::finish_and_wait_for_ack
      (std::string name)
    {
      auto const expected_id (_jobs.at (name)._id);

      finish (name);

      BOOST_REQUIRE_EQUAL (_finished_ack.wait(), expected_id);
    }
  }

  basic_drts_worker::basic_drts_worker
      (agent const& master, fhg::com::Certificates const& certificates)
    : no_thread::basic_drts_worker (master, certificates)
  {}
  basic_drts_worker::basic_drts_worker
      ( std::string name
      , agent const& master
      , fhg::com::Certificates const& certificates
      )
    : no_thread::basic_drts_worker (std::move (name), master, certificates)
  {}
  basic_drts_worker::basic_drts_worker
      ( std::string name
      , agent const& master
      , sdpa::capabilities_set_t capabilities
      , fhg::com::Certificates const& certificates
      )
    : no_thread::basic_drts_worker (std::move (name), master, std::move (capabilities), certificates)
  {}

  fake_drts_worker_notifying_module_call_submission
    ::fake_drts_worker_notifying_module_call_submission
      ( std::function<void (std::string)> announce_job
      , agent const& master
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, master, certificates)
  {}
  fake_drts_worker_notifying_module_call_submission
    ::fake_drts_worker_notifying_module_call_submission
      ( std::string name
      , std::function<void (std::string)> announce_job
      , agent const& master
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_notifying_module_call_submission
        (name, announce_job, master, certificates)
  {}

  fake_drts_worker_directly_finishing_jobs
    ::fake_drts_worker_directly_finishing_jobs
      ( agent const& master
      , fhg::com::Certificates const& certificates
      )
    : no_thread::basic_drts_worker (master, certificates)
  {}
  fake_drts_worker_directly_finishing_jobs
    ::fake_drts_worker_directly_finishing_jobs
      ( std::string name, agent const& master
      , fhg::com::Certificates const& certificates
      )
    : no_thread::basic_drts_worker (std::move (name), master, certificates)
  {}

  void fake_drts_worker_directly_finishing_jobs::handleSubmitJobEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::SubmitJobEvent const* e
    )
  {
    _network.perform<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());

    _network.perform<sdpa::events::JobFinishedEvent>
      (source, *e->job_id(), we::type::activity_t());
  }
  void fake_drts_worker_directly_finishing_jobs::handleJobFinishedAckEvent
    (fhg::com::p2p::address_t const&, sdpa::events::JobFinishedAckEvent const*)
  {
    // can be ignored as we don't have any state
  }

  fake_drts_worker_waiting_for_finished_ack
    ::fake_drts_worker_waiting_for_finished_ack
      ( std::function<void (std::string)> announce_job
      , agent const& master_agent
      , fhg::com::Certificates const& certificates
      )
  : no_thread::fake_drts_worker_waiting_for_finished_ack
      (std::move (announce_job), master_agent, certificates)
  {}

  fake_drts_worker_notifying_cancel::fake_drts_worker_notifying_cancel
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& master_agent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (announce_job, master_agent, certificates)
    , _announce_cancel (announce_cancel)
  {}
  fake_drts_worker_notifying_cancel::~fake_drts_worker_notifying_cancel()
  {
    BOOST_REQUIRE (_cancels.empty());
  }

  void fake_drts_worker_notifying_cancel::handleCancelJobEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::CancelJobEvent const* pEvt
    )
  {
    std::lock_guard<std::mutex> const _ (_cancels_mutex);

    _cancels.emplace (pEvt->job_id(), source);
    _announce_cancel (pEvt->job_id());
  }

  void fake_drts_worker_notifying_cancel::canceled (std::string job_id)
  {
    std::lock_guard<std::mutex> const _ (_cancels_mutex);

    auto const master (_cancels.at (job_id));
    _cancels.erase (job_id);

    _network.perform<sdpa::events::CancelJobAckEvent> (master, job_id);
  }

  fake_drts_worker_notifying_cancel_but_never_replying
    ::fake_drts_worker_notifying_cancel_but_never_replying
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& master_agent
      , fhg::com::Certificates const& certificates
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (announce_job, master_agent, certificates)
    , _announce_cancel (announce_cancel)
  {}

  void fake_drts_worker_notifying_cancel_but_never_replying
    ::handleCancelJobEvent ( fhg::com::p2p::address_t const&
                           , sdpa::events::CancelJobEvent const* pEvt
                           )
  {
    _announce_cancel (pEvt->job_id());
  }

  client::client ( orchestrator const& orch
                 , fhg::com::Certificates const& certificates
                 )
    : _ ( orch.host()
        , orch.port()
        , fhg::util::cxx14::make_unique<boost::asio::io_service>()
        , certificates
        )
  {}

  sdpa::job_id_t client::submit_job (we::type::activity_t workflow)
  {
    return _.submitJob (workflow);
  }

  sdpa::status::code client::query_job_status (sdpa::job_id_t const& id)
  {
    return _.queryJob (id);
  }

  sdpa::status::code client::wait_for_terminal_state_polling
    (sdpa::job_id_t const& id)
  {
    sdpa::client::job_info_t UNUSED_job_info;
    return _.wait_for_terminal_state_polling (id, UNUSED_job_info);
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

  sdpa::status::code client::wait_for_terminal_state_and_cleanup_polling
    (sdpa::job_id_t const& id)
  {
    auto const ret (wait_for_terminal_state_polling (id));
    retrieve_job_results (id);
    delete_job (id);
    return ret;
  }
  sdpa::status::code client::wait_for_terminal_state_and_cleanup
    (sdpa::job_id_t const& id)
  {
    auto const ret (wait_for_terminal_state (id));
    retrieve_job_results (id);
    delete_job (id);
    return ret;
  }

  sdpa::discovery_info_t client::discover (sdpa::job_id_t const& id)
  {
    static std::size_t i (0);
    auto const discover_id ( ( boost::format ("%1%%2%")
                             % fhg::util::testing::random_string()
                             % i++
                             ).str()
                           );
    return _.discoverJobStates (discover_id, id);
  }

  we::type::activity_t client::retrieve_job_results (sdpa::job_id_t const& id)
  {
    return _.retrieveResults (id);
  }

  void client::delete_job (sdpa::job_id_t const& id)
  {
    return _.deleteJob (id);
  }

  void client::cancel_job (sdpa::job_id_t const& id)
  {
    return _.cancelJob (id);
  }

  sdpa::status::code client::submit_job_and_wait_for_termination
    ( we::type::activity_t workflow
    , orchestrator const& orch
    , fhg::com::Certificates const& certificates
    )
  {
    client c (orch, certificates);

    return c.wait_for_terminal_state_and_cleanup_polling
      (c.submit_job (workflow));
  }

  sdpa::status::code client::submit_job_and_wait_for_termination_as_subscriber
    ( we::type::activity_t workflow
    , orchestrator const& orch
    , fhg::com::Certificates const& certificates
    )
  {
    client c (orch, certificates);

    return c.wait_for_terminal_state_and_cleanup (c.submit_job (workflow));
  }

  client::submitted_job::submitted_job
      ( we::type::activity_t workflow
      , orchestrator const& orch
      , fhg::com::Certificates const& certificates
      )
    : _client (fhg::util::cxx14::make_unique<client> (orch, certificates))
    , _job_id (_client->submit_job (workflow))
  {}

  client::submitted_job::~submitted_job()
  {
    _client->wait_for_terminal_state (_job_id);
    _client->retrieve_job_results (_job_id);
    _client->delete_job (_job_id);
  }

  sdpa::discovery_info_t client::submitted_job::discover()
  {
    return _client->discover (_job_id);
  }
}