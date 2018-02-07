// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <sdpa/client.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <fhglog/Configuration.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/ref.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

struct setup_logging
{
  boost::asio::io_service io_service;
  setup_logging()
    : _logger()
  {
    fhg::util::syscall::setenv ("FHGLOG_level", "TRACE", true);
    fhg::log::configure (io_service, _logger);
  }
  fhg::log::Logger _logger;
};

FHG_BOOST_TEST_LOG_VALUE_PRINTER (fhg::com::p2p::address_t, os, address)
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
      , we::type::module_call_t ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , std::unordered_map<std::string, std::string>()
                                , std::list<we::type::memory_transfer>()
                                , std::list<we::type::memory_transfer>()
                                )
      , boost::none
      , we::type::property::type()
      , we::priority_type()
      );
    const std::string port_name (fhg::util::testing::random_string());
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
      , we::type::module_call_t ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , std::unordered_map<std::string, std::string>()
                                , std::list<we::type::memory_transfer>()
                                , std::list<we::type::memory_transfer>()
                                )
      , boost::none
      , props
      , we::priority_type()
      );
    const std::string port_name (fhg::util::testing::random_string());
    we::port_id_type const port_id_in
      ( transition.add_port ( we::type::port_t ( port_name
                                               , we::type::PORT_IN
                                               , std::string ("string")
                                               , we::type::property::type()
                                               )
                            )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in
      (net.add_place (place::type (port_name, std::string ("string"), boost::none)));

    net.put_value (place_id_in, fhg::util::testing::random_string_without ("\\\""));

    we::transition_id_type const transition_id
      (net.add_transition (transition));

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
    props.set ({"fhg", "drts", "schedule", "num_worker"}, std::to_string (n) + "UL");
    we::type::transition_t transition_0
      ( fhg::util::testing::random_string()
      , we::type::module_call_t ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , std::unordered_map<std::string, std::string>()
                                , std::list<we::type::memory_transfer>()
                                , std::list<we::type::memory_transfer>()
                                )
      , boost::none
      , props
      , we::priority_type()
      );
    we::type::transition_t transition_1
      ( fhg::util::testing::random_string()
      , we::type::module_call_t ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , std::unordered_map<std::string, std::string>()
                                , std::list<we::type::memory_transfer>()
                                , std::list<we::type::memory_transfer>()
                                )
      , boost::none
      , props
      , we::priority_type()
      );
    const std::string port_name (fhg::util::testing::random_string());
    we::port_id_type const port_id_in_0
      ( transition_0.add_port ( we::type::port_t ( port_name
                                                 , we::type::PORT_IN
                                                 , std::string ("string")
                                                 , we::type::property::type()
                                                 )
                              )
      );
    we::port_id_type const port_id_in_1
      ( transition_1.add_port ( we::type::port_t ( port_name
                                                 , we::type::PORT_IN
                                                 , std::string ("string")
                                                 , we::type::property::type()
                                                 )
                              )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in_0
      (net.add_place (place::type (port_name + "1", std::string ("string"), boost::none)));
    we::place_id_type const place_id_in_1
      (net.add_place (place::type (port_name + "2", std::string ("string"), boost::none)));

    net.put_value (place_id_in_0, fhg::util::testing::random_string_without ("\\\""));
    net.put_value (place_id_in_1, fhg::util::testing::random_string_without ("\\\""));

    we::transition_id_type const transition_id_0
      (net.add_transition (transition_0));
    we::transition_id_type const transition_id_1
      (net.add_transition (transition_1));

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

  struct orchestrator : boost::noncopyable
  {
    orchestrator (fhg::log::Logger& logger)
      : _ ( random_peer_name(), "127.0.0.1"
          , fhg::util::cxx14::make_unique<boost::asio::io_service>()
          , boost::none
          , {}
          , logger
          , boost::none
          , false
          )
    {}

    sdpa::daemon::GenericDaemon _;
    std::string name() const { return _.name(); }
    fhg::com::host_t host() const
    {
      return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                  (_.peer_local_endpoint().address())
                              );
    }
    fhg::com::port_t port() const
    {
      return fhg::com::port_t
        (std::to_string (_.peer_local_endpoint().port()));
    }
  };

  namespace
  {
    template<typename Master>
      std::tuple<std::string, fhg::com::host_t, fhg::com::port_t>
        make_master_info_tuple (Master const& master)
    {
      return std::make_tuple (master.name(), master.host(), master.port());
    }
  }

  struct agent : boost::noncopyable
  {
    template <typename T, typename U>
      agent (const T& master_0, const U& master_1, fhg::log::Logger& logger)
      : boost::noncopyable ()
      , _ ( random_peer_name(), "127.0.0.1"
          , fhg::util::cxx14::make_unique<boost::asio::io_service>()
          , boost::none
          , {make_master_info_tuple (master_0), make_master_info_tuple (master_1)}
          , logger
          , boost::none
          , true
          )
    {}
    template <typename T>
      agent (const T& master, fhg::log::Logger& logger)
      : boost::noncopyable ()
      , _ ( random_peer_name(), "127.0.0.1"
          , fhg::util::cxx14::make_unique<boost::asio::io_service>()
          , boost::none
          , {make_master_info_tuple (master)}
          , logger
          , boost::none
          , true
          )
    {}
    agent (const agent& master, fhg::log::Logger& logger)
      : boost::noncopyable ()
      , _ ( random_peer_name(), "127.0.0.1"
          , fhg::util::cxx14::make_unique<boost::asio::io_service>()
          , boost::none
          , {make_master_info_tuple (master)}
          , logger
          , boost::none
          , true
          )
    {}
    sdpa::daemon::GenericDaemon _;
    std::string name() const { return _.name(); }
    fhg::com::host_t host() const
    {
      return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                  (_.peer_local_endpoint().address())
                              );
    }
    fhg::com::port_t port() const
    {
      return fhg::com::port_t
        (std::to_string (_.peer_local_endpoint().port()));
    }
  };

  class basic_drts_component : sdpa::events::EventHandler
  {
  public:
    basic_drts_component
      (std::string name, bool accept_workers)
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
                 , fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
                 )
    {}

    basic_drts_component ( std::string name
                         , utils::agent const& master
                         , sdpa::capabilities_set_t capabilities
                         , bool accept_workers
                         )
      : basic_drts_component (name, accept_workers)
    {
      _master = _network.connect_to (master.host(), master.port());

      _network.perform<sdpa::events::WorkerRegistrationEvent>
        ( _master.get()
        , _name
        , capabilities
        , fhg::util::testing::random<unsigned long>()
        , accept_workers
        , fhg::util::testing::random_string()
        );
    }

    ~basic_drts_component()
    {
      wait_for_workers_to_shutdown();
    }

    virtual void handle_worker_registration_response
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::worker_registration_response const* response
      ) override
    {
      BOOST_REQUIRE (_master);
      BOOST_REQUIRE_EQUAL (source, _master.get());

      response->get();
    }

    virtual void handleWorkerRegistrationEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::WorkerRegistrationEvent*
      ) override
    {
      BOOST_REQUIRE (_accept_workers);
      BOOST_REQUIRE (_accepted_workers.insert (source).second);

      _network.perform<sdpa::events::worker_registration_response>
        (source, boost::none);
    }

    virtual void handleErrorEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::ErrorEvent* e) override
    {
      if (e->error_code() == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN)
      {
        BOOST_REQUIRE (_accept_workers);
        std::lock_guard<std::mutex> const _ (_mutex_workers_shutdown);
        BOOST_REQUIRE (_accepted_workers.erase (source));
        if(_accepted_workers.empty())
          _cond_workers_shutdown.notify_all();
      }
      else
      {
        throw std::runtime_error ("UNHANDLED EVENT: ErrorEvent");
      }
    }

    void wait_for_workers_to_shutdown()
    {
      std::unique_lock<std::mutex> lock (_mutex_workers_shutdown);
      _cond_workers_shutdown.wait
        (lock, [&] { return _accepted_workers.empty(); });
    }

    std::string name() const { return _name; }
    fhg::com::host_t host() const
    {
      return fhg::com::host_t ( fhg::util::connectable_to_address_string
                                  (_network.local_endpoint().address())
                              );
    }
    fhg::com::port_t port() const
    {
      return fhg::com::port_t (std::to_string (_network.local_endpoint().port()));
    }

  protected:
    std::string _name;
    boost::optional<fhg::com::p2p::address_t> _master;
    bool _accept_workers;
    std::unordered_set<fhg::com::p2p::address_t> _accepted_workers;

  private:
    fhg::util::interruptible_threadsafe_queue
        <std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr>>
      _event_queue;

  protected:
    sdpa::com::NetworkStrategy _network;

    void event_thread()
    try
    {
      for (;;)
      {
        std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr> event
          (_event_queue.get());
        event.second->handleBy (event.first, this);
      }
    }
    catch (decltype (_event_queue)::interrupted const&)
    {
    }

    struct event_thread_and_worker_join
    {
      event_thread_and_worker_join (basic_drts_component& component)
        : _component (component)
        , _event_thread (&basic_drts_component::event_thread, &component)
        , _interrupt_thread (component._event_queue)
      {}
      ~event_thread_and_worker_join()
      {
        _component.wait_for_workers_to_shutdown();
      }

      basic_drts_component& _component;
      boost::strict_scoped_thread<> _event_thread;
      decltype (basic_drts_component::_event_queue)::interrupt_on_scope_exit
        _interrupt_thread;
    };

  private:
      std::mutex _mutex_workers_shutdown;
      std::condition_variable _cond_workers_shutdown;
  };

  namespace no_thread
  {
    class basic_drts_worker : public basic_drts_component
    {
    public:
      basic_drts_worker (utils::agent const& master)
        : basic_drts_component
          (random_peer_name(), master, sdpa::capabilities_set_t(), false)
      {}
      basic_drts_worker
        (std::string name, utils::agent const& master)
        : basic_drts_component
          (name, master, sdpa::capabilities_set_t(), false)
      {}
      basic_drts_worker ( std::string name
                        , utils::agent const& master
                        , sdpa::capabilities_set_t capabilities
                        )
        : basic_drts_component (name, master, capabilities, false)
      {}
    };

    class fake_drts_worker_notifying_module_call_submission
      : public basic_drts_worker
    {
    public:
      fake_drts_worker_notifying_module_call_submission
          ( std::function<void (std::string)> announce_job
          , utils::agent const& master
          )
        : basic_drts_worker (master)
        , _announce_job (announce_job)
      {}
      fake_drts_worker_notifying_module_call_submission
          ( std::string name
          , std::function<void (std::string)> announce_job
          , utils::agent const& master
          )
        : basic_drts_worker (name, master)
        , _announce_job (announce_job)
      {}

      virtual void handleSubmitJobEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent* e) override
      {
        const std::string name (e->activity().transition().name());

        add_job (name, *e->job_id(), source);

        _network.perform<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());

        announce_job (name);
      }
      virtual void handleJobFinishedAckEvent
        (fhg::com::p2p::address_t const&, const sdpa::events::JobFinishedAckEvent*) override
      {
        // can be ignored as we clean up in finish() already
      }

      void finish (std::string name)
      {
        const job_t job (_jobs.at (name));
        _jobs.erase (name);

        _network.perform<sdpa::events::JobFinishedEvent>
          (job._owner, job._id, we::type::activity_t());
      }

      sdpa::job_id_t job_id (std::string name)
      {
        return _jobs.at (name)._id;
      }

      void add_job ( const std::string& name
                   , const sdpa::job_id_t& job_id
                   , const fhg::com::p2p::address_t& owner
                   )
      {
        _jobs.emplace (name, job_t (job_id, owner));
      }

      void announce_job (const std::string& name)
      {
        _announce_job (name);
      }

    protected:
      struct job_t
      {
        sdpa::job_id_t _id;
        fhg::com::p2p::address_t _owner;
        job_t (sdpa::job_id_t id, fhg::com::p2p::address_t owner)
          : _id (id)
          , _owner (owner)
        {}
      };
      std::map<std::string, job_t> _jobs;

    private:
      std::function<void (std::string)> _announce_job;
    };

    class fake_drts_worker_waiting_for_finished_ack
      : public no_thread::fake_drts_worker_notifying_module_call_submission
    {
    public:
      fake_drts_worker_waiting_for_finished_ack
        ( std::function<void (std::string)> announce_job
        , const utils::agent& master_agent
        )
        : no_thread::fake_drts_worker_notifying_module_call_submission
            (announce_job, master_agent)
      {}

      virtual void handleJobFinishedAckEvent
        (fhg::com::p2p::address_t const&, const sdpa::events::JobFinishedAckEvent* e) override
      {
        _finished_ack.notify (e->job_id());
      }

      void finish_and_wait_for_ack (std::string name)
      {
        const std::string expected_id (_jobs.at (name)._id);

        finish (name);

        BOOST_REQUIRE_EQUAL (_finished_ack.wait(), expected_id);
      }

    private:
      fhg::util::thread::event<std::string> _finished_ack;
    };
  }

  struct basic_drts_worker final : public no_thread::basic_drts_worker
  {
    basic_drts_worker (utils::agent const& master)
      : no_thread::basic_drts_worker (master)
    {}
    basic_drts_worker (std::string name, utils::agent const& master)
      : no_thread::basic_drts_worker (std::move (name), master)
    {}
    basic_drts_worker ( std::string name
                      , utils::agent const& master
                      , sdpa::capabilities_set_t capabilities
                      )
      : no_thread::basic_drts_worker (std::move (name), master, std::move (capabilities))
    {}
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_notifying_module_call_submission final
    : public no_thread::fake_drts_worker_notifying_module_call_submission
  {
    fake_drts_worker_notifying_module_call_submission
      ( std::function<void (std::string)> announce_job
      , utils::agent const& master
      )
      : no_thread::fake_drts_worker_notifying_module_call_submission (announce_job, master)
    {}
    fake_drts_worker_notifying_module_call_submission
      ( std::string name
      , std::function<void (std::string)> announce_job
      , utils::agent const& master
      )
      : no_thread::fake_drts_worker_notifying_module_call_submission (name, announce_job, master)
    {}

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_directly_finishing_jobs final
    : public no_thread::basic_drts_worker
  {
    fake_drts_worker_directly_finishing_jobs (utils::agent const& master)
      : no_thread::basic_drts_worker (master)
    {}
    fake_drts_worker_directly_finishing_jobs (std::string name, utils::agent const& master)
      : no_thread::basic_drts_worker (std::move (name), master)
    {}

    virtual void handleSubmitJobEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent* e) override
    {
      _network.perform<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());

      _network.perform<sdpa::events::JobFinishedEvent>
        (source, *e->job_id(), we::type::activity_t());
    }
    virtual void handleJobFinishedAckEvent
      (fhg::com::p2p::address_t const&, const sdpa::events::JobFinishedAckEvent*) override
    {
      // can be ignored as we don't have any state
    }

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_waiting_for_finished_ack final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
    fake_drts_worker_waiting_for_finished_ack
      ( std::function<void (std::string)> announce_job
      , const utils::agent& master_agent
      )
    : no_thread::fake_drts_worker_waiting_for_finished_ack
        (std::move (announce_job), master_agent)
    {}
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_notifying_cancel final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
  public:
    fake_drts_worker_notifying_cancel
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , const utils::agent& master_agent
      )
      : no_thread::fake_drts_worker_waiting_for_finished_ack
          (announce_job, master_agent)
      , _announce_cancel (announce_cancel)
    {}
    ~fake_drts_worker_notifying_cancel()
    {
      BOOST_REQUIRE (_cancels.empty());
    }

    void handleCancelJobEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::CancelJobEvent* pEvt
      ) override
    {
      std::lock_guard<std::mutex> const _ (_cancels_mutex);

      _cancels.emplace (pEvt->job_id(), source);
      _announce_cancel (pEvt->job_id());
    }

    void canceled (std::string job_id)
    {
      std::lock_guard<std::mutex> const _ (_cancels_mutex);

      const fhg::com::p2p::address_t master (_cancels.at (job_id));
      _cancels.erase (job_id);

      _network.perform<sdpa::events::CancelJobAckEvent> (master, job_id);
    }

  private:
    std::function<void (std::string)> _announce_cancel;
    mutable std::mutex _cancels_mutex;
    std::map<std::string, fhg::com::p2p::address_t> _cancels;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_notifying_cancel_but_never_replying final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
  public:
    fake_drts_worker_notifying_cancel_but_never_replying
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , const utils::agent& master_agent
      )
      : no_thread::fake_drts_worker_waiting_for_finished_ack
          (announce_job, master_agent)
      , _announce_cancel (announce_cancel)
    {}

    void handleCancelJobEvent
      ( fhg::com::p2p::address_t const&
      , const sdpa::events::CancelJobEvent* pEvt
      ) override
    {
      _announce_cancel (pEvt->job_id());
    }

  private:
    std::function<void (std::string)> _announce_cancel;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct client : boost::noncopyable
  {
    client (orchestrator const& orch)
      : _ ( orch.host(), orch.port()
          , fhg::util::cxx14::make_unique<boost::asio::io_service>()
          )
    {}

    sdpa::job_id_t submit_job (we::type::activity_t workflow)
    {
      return _.submitJob (workflow);
    }

    sdpa::status::code query_job_status (const sdpa::job_id_t& id)
    {
      return _.queryJob (id);
    }

    sdpa::status::code wait_for_terminal_state_polling (const sdpa::job_id_t& id)
    {
      sdpa::client::job_info_t UNUSED_job_info;
      return _.wait_for_terminal_state_polling (id, UNUSED_job_info);
    }

    sdpa::status::code wait_for_terminal_state (const sdpa::job_id_t& id)
    {
      sdpa::client::job_info_t UNUSED_job_info;
      return _.wait_for_terminal_state (id, UNUSED_job_info);
    }

    sdpa::status::code wait_for_terminal_state_and_cleanup_polling
      (const sdpa::job_id_t& id)
    {
      const sdpa::status::code ret (wait_for_terminal_state_polling (id));
      retrieve_job_results (id);
      delete_job (id);
      return ret;
    }
    sdpa::status::code wait_for_terminal_state_and_cleanup
      (const sdpa::job_id_t& id)
    {
      const sdpa::status::code ret (wait_for_terminal_state (id));
      retrieve_job_results (id);
      delete_job (id);
      return ret;
    }

    sdpa::discovery_info_t discover (const sdpa::job_id_t& id)
    {
      static std::size_t i (0);
      const std::string discover_id
        ((boost::format ("%1%%2%") % fhg::util::testing::random_string() % i++).str());
      return _.discoverJobStates (discover_id, id);
    }

    we::type::activity_t retrieve_job_results (const sdpa::job_id_t& id)
    {
      return _.retrieveResults (id);
    }

    void delete_job (const sdpa::job_id_t& id)
    {
      return _.deleteJob (id);
    }

    void cancel_job (const sdpa::job_id_t& id)
    {
      return _.cancelJob (id);
    }

    sdpa::client::Client _;


    static sdpa::status::code submit_job_and_wait_for_termination
      (we::type::activity_t workflow, const orchestrator& orch)
    {
      client c (orch);

      return c.wait_for_terminal_state_and_cleanup_polling
        (c.submit_job (workflow));
    }

    static sdpa::status::code submit_job_and_wait_for_termination_as_subscriber
      (we::type::activity_t workflow, const orchestrator& orch)
    {
      client c (orch);

      return c.wait_for_terminal_state_and_cleanup (c.submit_job (workflow));
    }

    struct submitted_job : boost::noncopyable
    {
      submitted_job (we::type::activity_t workflow, orchestrator const& orch)
        : _client (new client (orch))
        , _job_id (_client->submit_job (workflow))
      {}

      ~submitted_job()
      {
        _client->wait_for_terminal_state (_job_id);
        _client->retrieve_job_results (_job_id);
        _client->delete_job (_job_id);
      }

      sdpa::discovery_info_t discover()
      {
        return _client->discover (_job_id);
      }

    private:
      std::unique_ptr<client> _client;
      sdpa::job_id_t _job_id;
    };
  };
}
