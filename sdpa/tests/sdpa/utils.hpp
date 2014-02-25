// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include "tests_config.hpp"

#include <sdpa/client.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

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
    return fhg::util::random_string_without (". ");
#endif
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
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( name, url
          , kvs_host, kvs_port
          , assemble_master_info_list (masters)
          , gui_url
          )
    {}
    agent ( const std::string& name
          , const std::string& url
          , std::string kvs_host, std::string kvs_port
          , const orchestrator& orchestrator
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( name, url
          , kvs_host, kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (orchestrator.name()))
          , gui_url
          )
    {}
    agent ( const std::string& name
          , const std::string& url
          , std::string kvs_host, std::string kvs_port
          , const agent& master
          , const boost::optional<std::string>& gui_url = boost::none
          )
      : _ ( name, url
          , kvs_host, kvs_port
          , sdpa::master_info_list_t (1, sdpa::MasterInfo (master.name()))
          , gui_url
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
