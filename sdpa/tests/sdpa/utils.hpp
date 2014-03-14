// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include "tests_config.hpp"

#include <sdpa/client.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>

#include <drts/worker/drts.hpp>

#include <fhg/util/random_string.hpp>

#include <plugin/core/kernel.hpp>
#include <plugin/plugin.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <fstream>
#include <sstream>
#include <string>

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
#define TEST_NO_HUMAN_READABLE_PEER_NAMES
#ifndef TEST_NO_HUMAN_READABLE_PEER_NAMES
    static std::size_t i (0);
    return boost::lexical_cast<std::string> (i++);
#else
    return fhg::util::random_string();
#endif
  }

  //! \todo unify with test/layer
  we::type::activity_t dummy_module_call (std::string name)
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

  std::string simple_module_call()
  {
    return dummy_module_call (fhg::util::random_string()).to_string();
  }

  struct orchestrator : boost::noncopyable
  {
    orchestrator ( const std::string& name, const std::string& url
                 , std::string kvs_host, std::string kvs_port
                 )
      : _kvs_host (kvs_host)
      , _kvs_port (kvs_port)
      , _ (name, url, kvs_host, kvs_port)
    {}
    orchestrator (std::string kvs_host, std::string kvs_port)
      : _kvs_host (kvs_host)
      , _kvs_port (kvs_port)
      , _ (random_peer_name(), "127.0.0.1", kvs_host, kvs_port)
    {}

    std::string _kvs_host;
    std::string _kvs_port;
    sdpa::daemon::Orchestrator _;
    std::string name() const { return _.name(); }
    std::string kvs_host() const { return _kvs_host; }
    std::string kvs_port() const { return _kvs_port; }
  };

  struct agent;

  typedef std::vector<boost::reference_wrapper<const utils::agent> > agents_t;

  namespace
  {
    sdpa::master_info_list_t assemble_master_info_list (const agents_t& masters);
  }

  struct agent : boost::noncopyable
  {
    agent ( const std::string& name
          , const std::string& url
          , std::string kvs_host, std::string kvs_port
          , const agents_t& masters
          )
      : _ ( name, url
          , kvs_host, kvs_port
          , assemble_master_info_list (masters)
          , boost::none
          )
    {}
    agent ( const std::string& name
          , const std::string& url
          , std::string kvs_host, std::string kvs_port
          , const orchestrator& orchestrator
          )
      : _ ( name, url
          , kvs_host, kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (orchestrator.name()))
          , boost::none
          )
    {}

    template <typename T>
    agent ( const std::string& name
             , const std::string& url
             , std::string kvs_host, std::string kvs_port
             , const T& master
             )
         : _ ( name, url
             , kvs_host, kvs_port
             , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
             , boost::none
             )
       {}
    agent ( const std::string& name
          , const std::string& url
          , std::string kvs_host, std::string kvs_port
          , const agent& master
          )
      : _ ( name, url
          , kvs_host, kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
          , boost::none
          )
    {}
    agent ( std::string kvs_host, std::string kvs_port
          , const orchestrator& master
          )
      : _ ( random_peer_name(), "127.0.0.1"
          , kvs_host, kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
          , boost::none
          )
    {}
    agent ( std::string kvs_host, std::string kvs_port
          , const agent& master
          )
      : _ ( random_peer_name(), "127.0.0.1"
          , kvs_host, kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
          , boost::none
          )
    {}
    sdpa::daemon::Agent _;
    std::string name() const { return _.name(); }
  };

  namespace
  {
    std::string assemble_string (const agents_t& masters)
    {
      std::string result;
      BOOST_FOREACH (const utils::agent& agent, masters)
      {
        result += agent.name() + ",";
      }
      return result.substr (0, result.size() - 1);
    }

    sdpa::master_info_list_t assemble_master_info_list (const agents_t& masters)
    {
      sdpa::master_info_list_t result;
      BOOST_FOREACH (const utils::agent& agent, masters)
      {
        result.push_back (sdpa::MasterInfo (agent.name()));
      }
      return result;
    }

    boost::shared_ptr<fhg::core::kernel_t>
      createDRTSWorker ( const std::string& drtsName
                       , const std::string& masterName
                       , const std::string& cpbList
                       , const std::string& strModulesPath
                       , const std::string& kvsHost
                       , const std::string& kvsPort
                       , boost::function<void()> request_stop
                       , std::map<std::string, std::string>& config_variables
                       )
    {

      config_variables ["plugin.drts.kvs_host"] = kvsHost;
      config_variables ["plugin.drts.kvs_port"] = kvsPort;

      //see ~/.sdpa/configs/sdpa.rc

      config_variables["kernel_name"] = drtsName;
      config_variables ["plugin.drts.master"] = masterName;
      config_variables ["plugin.drts.backlog"] = "2";

      if(!cpbList.empty())
        config_variables ["plugin.drts.capabilities"] = cpbList;

      config_variables ["plugin.drts.library_path"] = strModulesPath;

      boost::shared_ptr<fhg::core::kernel_t> kernel
        (new fhg::core::kernel_t (fhg::core::kernel_t::search_path_t(), request_stop, config_variables));

      kernel->load_plugin_from_file (TESTS_FVM_FAKE_PLUGIN_PATH);

      return kernel;
    }
  }

  struct drts_worker : boost::noncopyable
  {
    drts_worker ( std::string name
                , const agent& master
                , std::string capabilities
                , std::string modules_path
                , std::string kvs_host
                , std::string kvs_port
                )
      : _waiter()
      , _config_variables()
      , _kernel ( createDRTSWorker
                  (name, master.name(), capabilities, modules_path, kvs_host, kvs_port, _waiter.make_request_stop(), _config_variables)
                )
      , _thread (&drts_worker::thread, this)
    {
    }
    drts_worker ( std::string name
                , const agents_t& masters
                , std::string capabilities
                , std::string modules_path
                , std::string kvs_host
                , std::string kvs_port
                )
      : _waiter()
      , _config_variables()
      , _kernel ( createDRTSWorker
                  (name, assemble_string (masters), capabilities, modules_path, kvs_host, kvs_port, _waiter.make_request_stop(), _config_variables)
                )
      , _thread (&drts_worker::thread, this)
    {
    }
    drts_worker ( std::string name
                  , std::string master_name
                  , std::string capabilities
                  , std::string modules_path
                  , std::string kvs_host
                  , std::string kvs_port
                  )
      : _waiter()
      , _config_variables()
      , _kernel ( createDRTSWorker
                  (name, master_name, capabilities, modules_path, kvs_host, kvs_port, _waiter.make_request_stop(), _config_variables)
                )
      , _thread (&drts_worker::thread, this)
    {
    }

    ~drts_worker()
    {
      _waiter.stop();
    }

    void thread()
    {
      DRTSImpl const plugin (_waiter.make_request_stop(), _config_variables);
      _waiter.wait();
    }

    fhg::core::wait_until_stopped _waiter;
    std::map<std::string, std::string> _config_variables;
    boost::shared_ptr<fhg::core::kernel_t> _kernel;
    boost::thread _thread;
  };

  class BasicAgent : public sdpa::events::EventHandler,
                      boost::noncopyable
  {
  public:
    BasicAgent( std::string name
                 , boost::optional<const utils::agent&> master_agent
                 , std::string cpb_name = ""
                 )
     : _name (name)
     , _master_name(master_agent?master_agent.get().name():"")
     , _kvs_client
       ( new fhg::com::kvs::client::kvsc
         (kvs_host(), kvs_port(), true, boost::posix_time::seconds(120), 1)
       )
     , _network_strategy
       ( new sdpa::com::NetworkStrategy ( boost::bind (&BasicAgent::sendEventToSelf, this, _1)
                                        , name
                                        , fhg::com::host_t ("127.0.0.1")
                                        , fhg::com::port_t ("0")
                                        , _kvs_client
                                        )
       )
    , _event_handling_allowed(true)
    {
      if(!cpb_name.empty())
      {
          sdpa::capability_t cpb(cpb_name, name);
          _capabilities.insert(cpb);
      }

      if(master_agent)
      {
        sdpa::events::WorkerRegistrationEvent::Ptr
          pEvtWorkerReg (new sdpa::events::WorkerRegistrationEvent( _name
                                                                    , _master_name
                                                                    , boost::none
                                                                    , _capabilities ));
        _network_strategy->perform (pEvtWorkerReg);
      }
    }

    virtual ~BasicAgent() { _event_handling_allowed = false; }

    virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& pEvt)
    {
      if(_event_handling_allowed)
        pEvt->handleBy (this);
    }

    void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*){}
  protected:
    std::string _name;
    std::string _master_name;
    sdpa::capabilities_set_t _capabilities;
    fhg::com::kvs::kvsc_ptr_t _kvs_client;
    boost::shared_ptr<sdpa::com::NetworkStrategy> _network_strategy;
    bool _event_handling_allowed;
  };

  class basic_drts_worker : sdpa::events::EventHandler
  {
  public:
    basic_drts_worker
        (std::string kvs_host, std::string kvs_port, utils::agent const& master)
      : _name (random_peer_name())
      , _kvs_client
        ( new fhg::com::kvs::client::kvsc
          (kvs_host, kvs_port, true, boost::posix_time::seconds (120), 1)
        )
      , _event_queue()
      , _network
        ( boost::bind
          (&fhg::thread::queue<sdpa::events::SDPAEvent::Ptr>::put, &_event_queue, _1)
        , _name, fhg::com::host_t ("127.0.0.1"), fhg::com::port_t ("0")
        , _kvs_client
        )
      , _event_thread (&basic_drts_worker::event_thread, this)
      , _master_name (master._.name())
    {
      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          (new sdpa::events::WorkerRegistrationEvent (_name, master._.name(), 1))
        );
    }

    virtual void handleWorkerRegistrationAckEvent
      (const sdpa::events::WorkerRegistrationAckEvent* e)
    {
      BOOST_REQUIRE_EQUAL (e->from(), _master_name);
    }

  protected:
    std::string _name;

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

    std::string _master_name;
  };

  class fake_drts_worker_notifying_module_call_submission
    : public basic_drts_worker
  {
  public:
    fake_drts_worker_notifying_module_call_submission
        ( boost::function<void (std::string)> announce_job
        , std::string kvs_host
        , std::string kvs_port
        , utils::agent const& master
        )
      : basic_drts_worker (kvs_host, kvs_port, master)
      , _announce_job (announce_job)
    {}

    virtual void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* e)
    {
      const std::string name
        (we::type::activity_t (e->description()).transition().name());

      _jobs.insert (std::make_pair (name, job_t (*e->job_id(), e->from())));

      _announce_job (name);

      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          (new sdpa::events::SubmitJobAckEvent (_name, e->from(), *e->job_id()))
        );
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

  private:
    struct job_t
    {
      sdpa::job_id_t _id;
      std::string _owner;
      job_t (sdpa::job_id_t id, std::string owner) : _id (id), _owner (owner) {}
    };
    std::map<std::string, job_t> _jobs;

    boost::function<void (std::string)> _announce_job;
  };

  namespace client
  {
    struct client_t : boost::noncopyable
    {
      client_t (orchestrator const& orch)
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

      sdpa::status::code wait_for_state_polling
        (const sdpa::job_id_t& id, const sdpa::status::code& exp_status)
      {
        static const boost::posix_time::milliseconds sleep_duration (1000);
        sdpa::status::code curr_status (query_job_status (id));
        while(curr_status!=exp_status)
        {
          boost::this_thread::sleep (sleep_duration);
          curr_status = query_job_status (id);
        }
        return curr_status;
      }

      sdpa::discovery_info_t discover (const sdpa::job_id_t& id)
      {
        return _.discoverJobStates (fhg::util::random_string(), id);
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
    };

    struct submitted_job : boost::noncopyable
    {
      submitted_job (we::type::activity_t workflow, orchestrator const& orch)
        : _client (orch)
        , _job_id (_client.submit_job (workflow.to_string()))
      {}

      ~submitted_job()
      {
        _client.wait_for_terminal_state (_job_id);
      }

      sdpa::discovery_info_t discover()
      {
        return _client.discover (_job_id);
      }

    private:
      client_t _client;
      sdpa::job_id_t _job_id;
    };

    sdpa::job_id_t submit_job
      (sdpa::client::Client& c, std::string workflow)
    {
      return c.submitJob (workflow);
    }

    sdpa::status::code query_job_status
      (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      return c.queryJob (id);
    }

    sdpa::status::code wait_for_terminal_state_polling ( sdpa::client::Client& c
                                                , const sdpa::job_id_t& id
                                                )
    {
      sdpa::client::job_info_t UNUSED_job_info;
      return c.wait_for_terminal_state_polling (id, UNUSED_job_info);
    }

    sdpa::client::result_t retrieve_job_results
      (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      return c.retrieveResults (id);
    }

    void delete_job (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      return c.deleteJob (id);
    }

    void cancel_job (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      return c.cancelJob (id);
    }

    sdpa::status::code wait_for_terminal_state
      (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      sdpa::client::job_info_t UNUSED_job_info;
      return c.wait_for_terminal_state (id, UNUSED_job_info);
    }

    sdpa::status::code wait_for_state_polling
      ( sdpa::client::Client& c
        , const sdpa::job_id_t& id
        , const sdpa::status::code& exp_status )
    {
      static const boost::posix_time::milliseconds sleep_duration (1000);
      sdpa::status::code curr_status (c.queryJob (id));
      while(curr_status!=exp_status)
      {
        boost::this_thread::sleep (sleep_duration);
        curr_status = c.queryJob (id);
      }
      return curr_status;
    }

    namespace
    {
      sdpa::status::code wait_for_termination_impl
        (sdpa::job_id_t job_id_user, client_t& c)
      {
        const sdpa::status::code state
          (c.wait_for_terminal_state_polling (job_id_user));
        c.retrieve_job_results (job_id_user);
        c.delete_job (job_id_user);
        return state;
      }

      sdpa::status::code wait_for_termination_as_subscriber_impl
        (sdpa::job_id_t job_id_user, client_t& c)
      {
        const sdpa::status::code state
          (c.wait_for_terminal_state (job_id_user));
        c.retrieve_job_results (job_id_user);
        c.delete_job (job_id_user);
        return state;
      }
    }

    sdpa::status::code submit_job_and_wait_for_termination
      (std::string workflow, const orchestrator& orch)
    {
      client_t c (orch);

      return wait_for_termination_impl (c.submit_job (workflow), c);
    }

    sdpa::status::code submit_job_and_cancel_and_wait_for_termination
      (std::string workflow, const orchestrator& orch)
    {
      client_t c (orch);

      sdpa::job_id_t job_id_user (c.submit_job (workflow));
      c.cancel_job (job_id_user);
      return wait_for_termination_impl (job_id_user, c);
    }

    sdpa::status::code submit_job_and_wait_for_termination_as_subscriber
      (std::string workflow, const orchestrator& orch)
    {
      client_t c (orch);

      return wait_for_termination_as_subscriber_impl (c.submit_job (workflow), c);
    }

    sdpa::status::code submit_job_and_wait_for_termination_as_subscriber_with_two_different_clients
      (std::string workflow, const orchestrator& orch)
    {
      sdpa::job_id_t job_id_user;
      {
        client_t c (orch);
        job_id_user = c.submit_job (workflow);
      }

      {
        client_t c (orch);
        return wait_for_termination_as_subscriber_impl (job_id_user, c);
      }
    }
  }
}

#endif
