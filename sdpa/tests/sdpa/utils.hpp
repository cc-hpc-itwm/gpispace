// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include "tests_config.hpp"

#include <sdpa/client/ClientApi.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/memory.hpp>

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

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

  struct orchestrator : boost::noncopyable
  {
    orchestrator (const std::string& name, const std::string& url)
      : _ (sdpa::daemon::Orchestrator::create_with_start_called (name, url))
      , _name (name)
    {}
    ~orchestrator()
    {
      _->shutdown();
    }
    sdpa::daemon::Orchestrator::ptr_t _;
    std::string _name; std::string name() const { return _name; }
  };

  template<typename WFE> struct agent;

  typedef std::vector<boost::reference_wrapper<const utils::agent<we::mgmt::layer> > >
    agents_t;

  namespace
  {
    sdpa::master_info_list_t assemble_master_info_list (const agents_t& masters);
  }

  template<typename WFE> struct agent : boost::noncopyable
  {
    agent ( const std::string& name
          , const std::string& url
          , const agents_t& masters
          , const unsigned int rank = 0
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( sdpa::daemon::AgentFactory<WFE>::create_with_start_called
            (name, url, assemble_master_info_list (masters), rank, gui_url)
          )
      , _name (name)
    {}
    agent ( const std::string& name
          , const std::string& url
          , const orchestrator& orchestrator
          , const unsigned int rank = 0
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( sdpa::daemon::AgentFactory<WFE>::create_with_start_called
            (name, url
            , sdpa::master_info_list_t (1, sdpa::MasterInfo (orchestrator.name()))
            , rank, gui_url
            )
          )
      , _name (name)
    {}
    agent ( const std::string& name
          , const std::string& url
          , const agent<WFE>& master
          , const unsigned int rank = 0
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( sdpa::daemon::AgentFactory<WFE>::create_with_start_called
            (name, url
            , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
            , rank, gui_url
            )
          )
      , _name (name)
    {}
    ~agent()
    {
      _->shutdown();
    }
    sdpa::daemon::Agent::ptr_t _;
    std::string _name; std::string name() const { return _name; }
  };

  namespace
  {
    std::string assemble_string (const agents_t& masters)
    {
      std::string result;
      BOOST_FOREACH ( const utils::agent<we::mgmt::layer>& agent
                    , masters
                    )
      {
        result += agent.name() + ",";
      }
      return result.substr (0, result.size() - 1);
    }
  }

  namespace
  {
    sdpa::master_info_list_t assemble_master_info_list (const agents_t& masters)
    {
      sdpa::master_info_list_t result;
      BOOST_FOREACH ( const utils::agent<we::mgmt::layer>& agent
                    , masters
                    )
      {
        result.push_back (sdpa::MasterInfo (agent.name()));
      }
      return result;
    }

    sdpa::shared_ptr<fhg::core::kernel_t>
      createDRTSWorker ( const std::string& drtsName
                       , const std::string& masterName
                       , const std::string& cpbList
                       , const std::string& strModulesPath
                       , const std::string& kvsHost
                       , const std::string& kvsPort
                       )
    {
      sdpa::shared_ptr<fhg::core::kernel_t> kernel(new fhg::core::kernel_t);
      kernel->set_name (drtsName);

      kernel->put ("plugin.kvs.host", kvsHost);
      kernel->put ("plugin.kvs.port", kvsPort);

      //see ~/.sdpa/configs/sdpa.rc

      kernel->put ("plugin.drts.name", drtsName);
      kernel->put ("plugin.drts.master", masterName);
      kernel->put ("plugin.drts.backlog", "2");

      if(!cpbList.empty())
        kernel->put ("plugin.drts.capabilities", cpbList);

      kernel->put ("plugin.wfe.library_path", strModulesPath);

      kernel->load_plugin (TESTS_KVS_PLUGIN_PATH);
      kernel->load_plugin (TESTS_WFE_PLUGIN_PATH);
      kernel->load_plugin (TESTS_FVM_FAKE_PLUGIN_PATH);
      kernel->load_plugin (TESTS_DRTS_PLUGIN_PATH);

      return kernel;
    }
  }

  struct drts_worker : boost::noncopyable
  {
    drts_worker ( std::string name
                , const agent<we::mgmt::layer>& master
                , std::string capabilities
                , std::string modules_path
                , std::string kvs_host
                , std::string kvs_port
                )
      : _kernel ( createDRTSWorker
                  (name, master.name(), capabilities, modules_path, kvs_host, kvs_port)
                )
      , _thread (&fhg::core::kernel_t::run, _kernel)
    {
    }
    drts_worker ( std::string name
                , const agents_t& masters
                , std::string capabilities
                , std::string modules_path
                , std::string kvs_host
                , std::string kvs_port
                )
      : _kernel ( createDRTSWorker
                  (name, assemble_string (masters), capabilities, modules_path, kvs_host, kvs_port)
                )
      , _thread (&fhg::core::kernel_t::run, _kernel)
    {
    }

    ~drts_worker()
    {
      _kernel->stop();
      if (_thread.joinable())
      {
        _thread.join();
      }
      _kernel->unload_all();
    }

    sdpa::shared_ptr<fhg::core::kernel_t> _kernel;
    boost::thread _thread;
  };

  namespace client
  {
    namespace
    {
      const int NMAXTRIALS = 5;
    }

    struct retried_too_often : public std::runtime_error
    {
      retried_too_often (std::string what)
        : std::runtime_error ("RETRIED TOO OFTEN: " + what)
      {}
    };

    sdpa::job_id_t submit_job
      (sdpa::client::ClientApi::ptr_t c, std::string workflow)
    {
      LOG (DEBUG, "Submitting the following test workflow: \n" << workflow);
      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->submitJob (workflow);
        }
        catch (const sdpa::client::ClientException& ex)
        {
          LOG (DEBUG, ex.what());
        }
      }

      throw retried_too_often ("submit_job");
    }

    sdpa::status::code query_job_status
      (sdpa::client::ClientApi::ptr_t c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Query status for job " << id);

      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->queryJob (id);
        }
        catch (const sdpa::client::ClientException& ex)
        {
          LOG (DEBUG, ex.what());
        }
      }

      throw retried_too_often ("query_job_status");
    }

    template<typename Duration>
      void wait_for_job_termination ( sdpa::client::ClientApi::ptr_t c
                                    , const sdpa::job_id_t& id
                                    , Duration sleep_duration
                                    )
    {
      LOG (DEBUG, "Waiting for termination of job " << id);

      while (!sdpa::status::is_terminal (query_job_status (c, id)))
      {
        boost::this_thread::sleep (sleep_duration);
      }
    }

    sdpa::client::result_t retrieve_job_results
      (sdpa::client::ClientApi::ptr_t c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Retrieving results of job " << id);

      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->retrieveResults (id);
        }
        catch (const sdpa::client::ClientException& ex)
        {
          LOG (DEBUG, ex.what());
        }
      }

      throw retried_too_often ("retrieve_job_results");
    }

    void delete_job (sdpa::client::ClientApi::ptr_t c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Delete job " << id);

      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->deleteJob (id);
        }
        catch (const sdpa::client::ClientException& ex)
        {
          LOG (DEBUG, ex.what());
        }
      }

      throw retried_too_often ("delete_job");
    }

    void cancel_job (sdpa::client::ClientApi::ptr_t c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Cancel job " << id);

      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->cancelJob (id);
        }
        catch (const sdpa::client::ClientException& ex)
        {
          LOG (DEBUG, ex.what());
        }
      }

      throw retried_too_often ("cancel_job");
    }

    void subscribe (sdpa::client::ClientApi::ptr_t c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Subscribe to job " << id);

      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->subscribe (id);
        }
        catch (const sdpa::client::ClientException& ex)
        {
          LOG (DEBUG, ex.what());
        }
      }

      throw retried_too_often ("subscribe");
    }

    //! \note This duplicates behavior from apps/blocking_client.cpp,
    //! which should probably go into ClientApi.
    sdpa::status::code wait_for_status_change
      (sdpa::client::ClientApi::ptr_t c, const sdpa::job_id_t& id)
    {
      subscribe (c, id);

      seda::IEvent::Ptr event (c->waitForNotification());

      if ( sdpa::events::JobFinishedEvent* evt
         = dynamic_cast<sdpa::events::JobFinishedEvent*> (event.get())
         )
      {
        BOOST_REQUIRE (evt->job_id() == id);
        return sdpa::status::FINISHED;
      }
      else if ( sdpa::events::JobFailedEvent* evt
              = dynamic_cast<sdpa::events::JobFailedEvent*> (event.get())
              )
      {
        BOOST_REQUIRE (evt->job_id() == id);
        return sdpa::status::FAILED;
      }
      else if ( sdpa::events::CancelJobAckEvent* evt
              = dynamic_cast<sdpa::events::CancelJobAckEvent*> (event.get())
              )
      {
        BOOST_REQUIRE (evt->job_id() == id);
        return sdpa::status::CANCELED;
      }
      else if ( sdpa::events::ErrorEvent* evt
              = dynamic_cast<sdpa::events::ErrorEvent*>(event.get())
              )
      {
        throw std::runtime_error
          ( "got error event: reason := "
          + evt->reason()
          + " code := "
          + boost::lexical_cast<std::string> (evt->error_code())
          );
      }
      else
      {
        throw std::runtime_error
          ("invalid event after subscription to client");
      }
    }

    namespace
    {
      struct shutdown_on_exit
      {
        shutdown_on_exit (sdpa::client::ClientApi::ptr_t& c)
          : _c (c)
        {}
        ~shutdown_on_exit()
        {
          if (_c)
          {
            _c->shutdown_network();
          }
        }

        sdpa::client::ClientApi::ptr_t& _c;
      };
    }

    void create_client_and_execute
      ( std::string client_name
      , const orchestrator& orch
      , boost::function<void (sdpa::client::ClientApi::ptr_t)> function
      )
    {
      std::vector<std::string> command_line;
      command_line.push_back ("--orchestrator=" + orch.name());

      try
      {
        sdpa::client::config_t config (sdpa::client::ClientApi::config());
        config.parse_command_line (command_line);

        sdpa::client::ClientApi::ptr_t ptrCli
          ( sdpa::client::ClientApi::create_with_configured_network
            ( config
            , client_name
            , client_name + ".apps.client.out"
            )
          );
        shutdown_on_exit _ (ptrCli);

        function (ptrCli);
      }
      catch (const retried_too_often& ex)
      {
        BOOST_FAIL (ex.what());
      }
    }

    namespace
    {
      void wait_for_termination_impl
        (sdpa::job_id_t job_id_user, sdpa::client::ClientApi::ptr_t ptrCli)
      {
        wait_for_job_termination (ptrCli, job_id_user, boost::posix_time::seconds (1));
        retrieve_job_results (ptrCli, job_id_user);
        delete_job (ptrCli, job_id_user);
      }

      void wait_for_termination_as_subscriber_impl
        (sdpa::job_id_t job_id_user, sdpa::client::ClientApi::ptr_t ptrCli)
      {
        const sdpa::status::code state
          (wait_for_status_change (ptrCli, job_id_user));
        BOOST_REQUIRE (sdpa::status::is_terminal (state));
        retrieve_job_results (ptrCli, job_id_user);
        delete_job (ptrCli, job_id_user);
      }

      void submit_job_and_wait_for_termination_impl
        (std::string workflow, sdpa::client::ClientApi::ptr_t ptrCli)
      {
        const sdpa::job_id_t job_id_user (submit_job (ptrCli, workflow));
        wait_for_termination_impl (job_id_user, ptrCli);
      }

      void submit_job_and_cancel_and_wait_for_termination_impl
        (std::string workflow, sdpa::client::ClientApi::ptr_t ptrCli)
      {
        const sdpa::job_id_t job_id_user (submit_job (ptrCli, workflow));
        //! \todo There should not be a requirement for this!
        boost::this_thread::sleep (boost::posix_time::seconds (1));
        cancel_job (ptrCli, job_id_user);
        wait_for_termination_impl (job_id_user, ptrCli);
      }

      void submit_job_and_wait_for_termination_as_subscriber_impl
        (std::string workflow, sdpa::client::ClientApi::ptr_t ptrCli)
      {
        wait_for_termination_as_subscriber_impl
          (submit_job (ptrCli, workflow), ptrCli);
      }

      void submit_job_result_by_ref
        ( std::string workflow
        , sdpa::client::ClientApi::ptr_t ptrCli
        , sdpa::job_id_t* job_id
        )
      {
        *job_id = submit_job (ptrCli, workflow);
      }
    }

    void submit_job_and_wait_for_termination ( std::string workflow
                                             , const orchestrator& orch
                                             )
    {
      create_client_and_execute
        ( "sdpac"
        , orch
        , boost::bind (&submit_job_and_wait_for_termination_impl, workflow, _1)
        );
    }

    void submit_job_and_cancel_and_wait_for_termination ( std::string workflow
                                                        , const orchestrator& orch
                                                        )
    {
      create_client_and_execute
        ( "sdpac"
        , orch
        , boost::bind (&submit_job_and_cancel_and_wait_for_termination_impl, workflow, _1)
        );
    }

    void submit_job_and_wait_for_termination_as_subscriber
      ( std::string workflow
      , const orchestrator& orch
      )
    {
      create_client_and_execute
        ( "sdpac"
        , orch
        , boost::bind (&submit_job_and_wait_for_termination_as_subscriber_impl, workflow, _1)
        );
    }

    void submit_job_and_wait_for_termination_as_subscriber_with_two_different_clients
      ( std::string workflow
      , const orchestrator& orch
      )
    {
      sdpa::job_id_t job_id_user;
      create_client_and_execute ( "sdpac"
                                , orch
                                , boost::bind ( &submit_job_result_by_ref
                                              , workflow
                                              , _1
                                              , &job_id_user
                                              )
                                );

      create_client_and_execute
        ( "sdpac2"
        , orch
        , boost::bind (&wait_for_termination_as_subscriber_impl, job_id_user, _1)
        );
    }
  }
}

#endif
