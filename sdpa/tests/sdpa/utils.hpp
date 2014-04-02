// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include <sdpa/client.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/tcp_server.hpp>

#include <fhg/util/random_string.hpp>

#include <fhglog/fhglog.hpp>

#include <plugin/core/kernel.hpp>
#include <plugin/plugin.hpp>

#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

struct setup_logging
{
  setup_logging()
  {
    setenv ("FHGLOG_level", "TRACE", true);
    FHGLOG_SETUP();
  }
};

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
    + fhg::util::random_string()
#endif
      ;
  }

  //! \todo unify with test/layer
  we::type::activity_t module_call (std::string name)
  {
    we::type::transition_t transition
      ( name
      , we::type::module_call_t
        (fhg::util::random_string(), fhg::util::random_string())
      , boost::none
      , true
      , we::type::property::type()
      , we::priority_type()
      );
    const std::string port_name (fhg::util::random_string());
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
                  , fhg::util::random_string_without ("\\\"")
                  );
    return act;
  }

  std::string module_call()
  {
    return module_call (fhg::util::random_string()).to_string();
  }

  we::type::activity_t net_with_one_child_requiring_workers (unsigned long count)
  {
    we::type::property::type props;
    props.set ( "fhg.drts.schedule.num_worker"
              , boost::lexical_cast<std::string> (count) + "UL"
              );
    we::type::transition_t transition
      ( fhg::util::random_string()
      , we::type::module_call_t
        (fhg::util::random_string(), fhg::util::random_string())
      , boost::none
      , true
      , props
      , we::priority_type()
      );
    const std::string port_name (fhg::util::random_string());
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
      (net.add_place (place::type (port_name, std::string ("string"))));

    net.put_value (place_id_in, fhg::util::random_string_without ("\\\""));

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    net.add_connection ( we::edge::PT
                       , transition_id
                       , place_id_in
                       , port_id_in
                       , we::type::property::type()
                       );

    return we::type::activity_t
      ( we::type::transition_t ( fhg::util::random_string()
                               , net
                               , boost::none
                               , true
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , boost::none
      );
  }

  struct kvs_server : boost::noncopyable
  {
    kvs_server()
      : _io_service_pool (1)
      , _kvs_daemon (boost::none)
      , _tcp_server (_io_service_pool, _kvs_daemon, "localhost", "0", true)
      , _io_thread (&fhg::com::io_service_pool::run, &_io_service_pool)
    {}
    ~kvs_server()
    {
      _tcp_server.stop();
      _io_service_pool.stop();
    }

    std::string kvs_host() const
    {
      return "localhost";
    }
    std::string kvs_port() const
    {
      return boost::lexical_cast<std::string> (_tcp_server.port());
    }

  private:
    fhg::com::io_service_pool _io_service_pool;
    fhg::com::kvs::server::kvsd _kvs_daemon;
    fhg::com::tcp_server _tcp_server;
    boost::scoped_thread<boost::join_if_joinable> _io_thread;
  };

  struct orchestrator : boost::noncopyable
  {
    orchestrator (const kvs_server& kvs)
      : _kvs_host (kvs.kvs_host())
      , _kvs_port (kvs.kvs_port())
      , _ (random_peer_name(), "127.0.0.1", _kvs_host, _kvs_port)
    {}

    std::string _kvs_host;
    std::string _kvs_port;
    sdpa::daemon::Orchestrator _;
    std::string name() const { return _.name(); }
    std::string kvs_host() const { return _kvs_host; }
    std::string kvs_port() const { return _kvs_port; }
  };

  namespace
  {
    template<typename T, typename U>
    sdpa::master_info_list_t assemble_master_info_list
      (const T& master_0, const U& master_1)
    {
      sdpa::master_info_list_t result;
      result.push_back (sdpa::MasterInfo (master_0.name()));
      result.push_back (sdpa::MasterInfo (master_1.name()));
      return result;
    }
  }

  struct agent : boost::noncopyable
  {
    template <typename T, typename U>
    agent (const T& master_0, const U& master_1)
      : _kvs_host (master_0.kvs_host())
      , _kvs_port (master_0.kvs_port())
      , _ ( random_peer_name(), "127.0.0.1"
          , _kvs_host, _kvs_port
          , assemble_master_info_list (master_0, master_1)
          , boost::none
          )
    {}
    template <typename T>
    agent (const T& master)
      : _kvs_host (master.kvs_host())
      , _kvs_port (master.kvs_port())
      , _ ( random_peer_name(), "127.0.0.1"
          , _kvs_host, _kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
          , boost::none
          )
    {}
    agent (const agent& master)
      : _kvs_host (master.kvs_host())
      , _kvs_port (master.kvs_port())
      , _ ( random_peer_name(), "127.0.0.1"
          , _kvs_host, _kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
          , boost::none
          )
    {}
    std::string _kvs_host;
    std::string _kvs_port;
    sdpa::daemon::Agent _;
    std::string name() const { return _.name(); }
    std::string kvs_host() const { return _kvs_host; }
    std::string kvs_port() const { return _kvs_port; }
  };

  class basic_drts_component : sdpa::events::EventHandler
  {
  public:
    basic_drts_component
        (std::string name, kvs_server const& kvs, bool accept_workers)
      : _name (name)
      , _kvs_host (kvs.kvs_host())
      , _kvs_port (kvs.kvs_port())
      , _master_name (boost::none)
      , _accept_workers (accept_workers)
      , _kvs_client
        ( new fhg::com::kvs::client::kvsc
          (_kvs_host, _kvs_port, true, boost::posix_time::seconds (120), 1)
        )
      , _event_queue()
      , _network
        ( [this] (sdpa::events::SDPAEvent::Ptr e) { _event_queue.put (e); }
        , _name, fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
        , _kvs_client
        )
      , _event_thread (&basic_drts_component::event_thread, this)
    {}

    basic_drts_component ( std::string name
                         , utils::agent const& master
                         , sdpa::capabilities_set_t capabilities
                         , bool accept_workers
                         )
      : _name (name)
      , _kvs_host (master.kvs_host())
      , _kvs_port (master.kvs_port())
      , _master_name (master.name())
      , _accept_workers (accept_workers)
      , _kvs_client
        ( new fhg::com::kvs::client::kvsc
          (_kvs_host, _kvs_port, true, boost::posix_time::seconds (120), 1)
        )
      , _event_queue()
      , _network
        ( [this] (sdpa::events::SDPAEvent::Ptr e) { _event_queue.put (e); }
        , _name, fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
        , _kvs_client
        )
      , _event_thread (&basic_drts_component::event_thread, this)
    {
      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          ( new sdpa::events::WorkerRegistrationEvent
            (_name, *_master_name, 1, capabilities)
          )
        );
    }

    ~basic_drts_component()
    {
      BOOST_REQUIRE (_accepted_workers.empty());
    }

    virtual void handleWorkerRegistrationAckEvent
      (const sdpa::events::WorkerRegistrationAckEvent* e)
    {
      BOOST_REQUIRE (_master_name);
      BOOST_REQUIRE_EQUAL (e->from(), _master_name);
    }

    virtual void handleWorkerRegistrationEvent
      (const sdpa::events::WorkerRegistrationEvent* e)
    {
      BOOST_REQUIRE (_accept_workers);
      BOOST_REQUIRE (_accepted_workers.insert (e->from()).second);

      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          (new sdpa::events::WorkerRegistrationAckEvent (_name, e->from()))
        );
    }

    virtual void handleErrorEvent (const sdpa::events::ErrorEvent* e)
    {
      if (e->error_code() == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN)
      {
        BOOST_REQUIRE (_accept_workers);
        BOOST_REQUIRE (_accepted_workers.erase (e->from()));
      }
      else
      {
        throw std::runtime_error ("UNHANDLED EVENT: ErrorEvent");
      }
    }

    std::string name() const { return _name; }
    std::string kvs_host() const { return _kvs_host; }
    std::string kvs_port() const { return _kvs_port; }

  protected:
    std::string _name;
    std::string _kvs_host;
    std::string _kvs_port;
    boost::optional<std::string> _master_name;
    bool _accept_workers;
    std::set<std::string> _accepted_workers;

  private:
    fhg::com::kvs::kvsc_ptr_t _kvs_client;

    fhg::thread::queue<sdpa::events::SDPAEvent::Ptr> _event_queue;

  protected:
    sdpa::com::NetworkStrategy _network;

  private:
    boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      _event_thread;
    void event_thread()
    {
      for (;;)
      {
        _event_queue.get()->handleBy (this);
      }
    }
  };

  class basic_drts_worker : public basic_drts_component
  {
  public:
    basic_drts_worker (utils::agent const& master)
      : basic_drts_component
        (random_peer_name(), master, sdpa::capabilities_set_t(), false)
    {}
    basic_drts_worker (std::string name, utils::agent const& master)
      : basic_drts_component (name, master, sdpa::capabilities_set_t(), false)
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

    virtual void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* e)
    {
      const std::string name
        (we::type::activity_t (e->description()).transition().name());

      _jobs.insert (std::make_pair (name, job_t (*e->job_id(), e->from())));

      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          (new sdpa::events::SubmitJobAckEvent (_name, e->from(), *e->job_id()))
        );

      _announce_job (name);
    }
    virtual void handleJobFinishedAckEvent
      (const sdpa::events::JobFinishedAckEvent*)
    {
      // can be ignored as we clean up in finish() already
    }

    void finish (std::string name)
    {
      const job_t job (_jobs.at (name));
      _jobs.erase (name);

      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          ( new sdpa::events::JobFinishedEvent
            (_name, job._owner, job._id, we::type::activity_t().to_string())
          )
        );
    }

  protected:
    struct job_t
    {
      sdpa::job_id_t _id;
      std::string _owner;
      job_t (sdpa::job_id_t id, std::string owner) : _id (id), _owner (owner) {}
    };
    std::map<std::string, job_t> _jobs;

  private:
    std::function<void (std::string)> _announce_job;
  };

  class fake_drts_worker_directly_finishing_jobs : public basic_drts_worker
  {
  public:
    fake_drts_worker_directly_finishing_jobs (utils::agent const& master)
      : basic_drts_worker (master)
    {}
    fake_drts_worker_directly_finishing_jobs
        (std::string name, utils::agent const& master)
      : basic_drts_worker (name, master)
    {}

    virtual void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* e)
    {
      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          (new sdpa::events::SubmitJobAckEvent (_name, e->from(), *e->job_id()))
        );

      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          ( new sdpa::events::JobFinishedEvent
            (_name, e->from(), *e->job_id(), we::type::activity_t().to_string())
          )
        );
    }
    virtual void handleJobFinishedAckEvent
      (const sdpa::events::JobFinishedAckEvent*)
    {
      // can be ignored as we don't have any state
    }
  };

  struct client : boost::noncopyable
  {
    client (orchestrator const& orch)
      : _ (orch.name(), orch.kvs_host(), orch.kvs_port())
    {}

    sdpa::job_id_t submit_job (std::string workflow)
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
        ((boost::format ("%1%%2%") % fhg::util::random_string() % i++).str());
      return _.discoverJobStates (discover_id, id);
    }

    sdpa::client::result_t retrieve_job_results (const sdpa::job_id_t& id)
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
      (std::string workflow, const orchestrator& orch)
    {
      client c (orch);

      return c.wait_for_terminal_state_and_cleanup_polling
        (c.submit_job (workflow));
    }

    static sdpa::status::code submit_job_and_wait_for_termination_as_subscriber
      (std::string workflow, const orchestrator& orch)
    {
      client c (orch);

      return c.wait_for_terminal_state_and_cleanup (c.submit_job (workflow));
    }

    struct submitted_job : boost::noncopyable
    {
      submitted_job (we::type::activity_t workflow, orchestrator const& orch)
        : _client (new client (orch))
        , _job_id (_client->submit_job (workflow.to_string()))
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

#endif
