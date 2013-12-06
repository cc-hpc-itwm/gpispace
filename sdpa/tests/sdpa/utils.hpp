// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include "tests_config.hpp"

#include <sdpa/client.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/plugins/drts.hpp>

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
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
      : _ (sdpa::daemon::Orchestrator::create (name, url))
      , _name (name)
    {}
    ~orchestrator()
    {
      _->shutdown();
    }
    sdpa::daemon::Orchestrator::ptr_t _;
    std::string _name; std::string name() const { return _name; }
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
          , const agents_t& masters
          , const unsigned int rank = 0
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( sdpa::daemon::Agent::create
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
      : _ ( sdpa::daemon::Agent::create
            ( name, url
            , sdpa::master_info_list_t (1, sdpa::MasterInfo (orchestrator.name()))
            , rank, gui_url
            )
          )
      , _name (name)
    {}
    agent ( const std::string& name
          , const std::string& url
          , const agent& master
          , const unsigned int rank = 0
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( sdpa::daemon::Agent::create
            ( name, url
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
                       )
    {
      boost::shared_ptr<fhg::core::kernel_t> kernel(new fhg::core::kernel_t);
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
                , const agent& master
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

    void add_capability(std::string const &cap)
    {
      drts::DRTS *drts_plugin = _kernel->lookup_plugin_as<drts::DRTS>("drts");
      BOOST_REQUIRE(drts_plugin);
      drts_plugin->add_virtual_capability(cap);
    }

    boost::shared_ptr<fhg::core::kernel_t> _kernel;
    boost::thread _thread;
  };

  namespace client
  {
    sdpa::job_id_t submit_job
      (sdpa::client::Client& c, std::string workflow)
    {
      LOG (DEBUG, "Submitting the following test workflow: \n" << workflow);

      return c.submitJob (workflow);
    }

    sdpa::status::code query_job_status
      (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Query status for job " << id);

      return c.queryJob (id);
    }

    sdpa::status::code wait_for_job_termination ( sdpa::client::Client& c
                                                , const sdpa::job_id_t& id
                                                )
    {
      LOG (DEBUG, "Waiting for termination of job " << id);

      sdpa::client::job_info_t job_info;
      return c.wait_for_terminal_state_polling (id, job_info);
    }

    sdpa::client::result_t retrieve_job_results
      (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Retrieving results of job " << id);

      return c.retrieveResults (id);
    }

    void delete_job (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Delete job " << id);

      return c.deleteJob (id);
    }

    void cancel_job (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Cancel job " << id);

      return c.cancelJob (id);
    }

    sdpa::status::code wait_for_terminal_state
      (sdpa::client::Client& c, const sdpa::job_id_t& id)
    {
      LOG (DEBUG, "Subscribe to job " << id);

      sdpa::client::job_info_t job_info;
      return c.wait_for_terminal_state (id, job_info);
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
        (sdpa::job_id_t job_id_user, sdpa::client::Client& c)
      {
        const sdpa::status::code state
          (wait_for_job_termination (c, job_id_user));
        retrieve_job_results (c, job_id_user);
        delete_job (c, job_id_user);
        return state;
      }

      sdpa::status::code wait_for_termination_as_subscriber_impl
        (sdpa::job_id_t job_id_user, sdpa::client::Client& c)
      {
        const sdpa::status::code state
          (wait_for_terminal_state (c, job_id_user));
        retrieve_job_results (c, job_id_user);
        delete_job (c, job_id_user);
        return state;
      }
    }

    sdpa::status::code submit_job_and_wait_for_termination
      (std::string workflow, const orchestrator& orch)
    {
      sdpa::client::Client c (orch.name());

      return wait_for_termination_impl (submit_job (c, workflow), c);
    }

    sdpa::status::code submit_job_and_cancel_and_wait_for_termination
      (std::string workflow, const orchestrator& orch)
    {
      sdpa::client::Client c (orch.name());

      sdpa::job_id_t job_id_user(submit_job (c, workflow));
      cancel_job (c, job_id_user);
      return wait_for_termination_impl (job_id_user, c);
    }

    sdpa::status::code submit_job_and_wait_for_termination_as_subscriber
      (std::string workflow, const orchestrator& orch)
    {
      sdpa::client::Client c (orch.name());

      return wait_for_termination_as_subscriber_impl (submit_job (c, workflow), c);
    }

    sdpa::status::code submit_job_and_wait_for_termination_as_subscriber_with_two_different_clients
      (std::string workflow, const orchestrator& orch)
    {
      sdpa::job_id_t job_id_user;
      {
        sdpa::client::Client c (orch.name());
        job_id_user = submit_job (c, workflow);
      }

      {
        sdpa::client::Client c (orch.name());
        return wait_for_termination_as_subscriber_impl (job_id_user, c);
      }
    }
  }
}

#endif
