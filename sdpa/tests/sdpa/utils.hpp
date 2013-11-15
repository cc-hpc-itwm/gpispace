// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_TEST_UTILS_HPP
#define SDPA_TEST_UTILS_HPP

#include <CreateDrtsWorker.hpp>

#include <sdpa/client/ClientApi.hpp>

#include <fhg/plugin/core/kernel.hpp>

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

    std::ostringstream os;
    char c;
    while (f.get (c)) os << c;
    f.close();

    return os.str();
  }

  struct drts_worker
  {
    drts_worker ( std::string name
                , std::string master
                , std::string capabilities
                , std::string modules_path
                , std::string kvs_host
                , std::string kvs_port
                )
      : _kernel ( createDRTSWorker
                  (name, master, capabilities, modules_path, kvs_host, kvs_port)
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

      sdpa::client::job_info_t info;
      for (int i (0); i < NMAXTRIALS; ++i)
      {
        try
        {
          return c->queryJob (id, info);
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

      for ( bool terminated
            (sdpa::status::is_terminal (query_job_status (c, id)))
          ; !terminated
          ;
          )
      {
        boost::this_thread::sleep (sleep_duration);
        terminated = sdpa::status::is_terminal (query_job_status (c, id));
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

    void submit_job_and_wait_for_termination ( std::string workflow
                                             , std::string client_name
                                             , std::string orchestrator_name
                                             )
    {
      std::vector<std::string> command_line;
      command_line.push_back ("--orchestrator=" + orchestrator_name);

      try
      {
        sdpa::client::config_t config (sdpa::client::ClientApi::config());
        config.parse_command_line (command_line);

        sdpa::client::ClientApi::ptr_t ptrCli
          ( sdpa::client::ClientApi::create ( config
                                            , client_name
                                            , client_name + ".apps.client.out"
                                            )
          );
        ptrCli->configure_network( config );
        shutdown_on_exit _ (ptrCli);

        const sdpa::job_id_t job_id_user (submit_job (ptrCli, workflow));
        wait_for_job_termination (ptrCli, job_id_user, boost::posix_time::seconds (1));
        retrieve_job_results (ptrCli, job_id_user);
        delete_job (ptrCli, job_id_user);
      }
      catch (const retried_too_often& ex)
      {
        BOOST_FAIL (ex.what());
      }
    }
  }
}

#endif
