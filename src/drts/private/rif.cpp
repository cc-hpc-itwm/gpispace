#include <drts/private/rif.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>
#include <future>

#include <cstdlib>

namespace gspc
{
  namespace
  {
    struct scoped_popen
    {
      explicit scoped_popen (const char *command, const char *how)
        : _ (fhg::syscall::popen (command, how))
      {}

      ~scoped_popen()
      {
        if (_)
        {
          fhg::syscall::pclose (_);
        }
      }

      FILE *_;
    };

    pid_t rexec (const rif_t::endpoint_t rif, std::string const& command)
    {
      const scoped_popen pid_file
        (("ssh -q " + rif.host + " " + command).c_str(), "r");

      struct free_on_scope_exit
      {
        ~free_on_scope_exit()
        {
          free (_);
          _ = nullptr;
        }
        char* _ {nullptr};
      } line;
      std::size_t length (0);

      std::vector<std::string> lines;
      ssize_t read;
      while ((read = getline (&line._, &length, pid_file._)) != -1)
      {
        lines.emplace_back (line._, read - 1);
      }

      return boost::lexical_cast<pid_t> (lines.at (0));
    }

    void rexec_no_pid_returned
      (const rif_t::endpoint_t rif, std::string const& command)
    {
      fhg::util::system_with_blocked_SIGCHLD
        (("ssh -q " + rif.host + " " + command).c_str());
    }

    void run_asynchronously_and_wait ( const std::list<rif_t::endpoint_t>& rifs
                                     , std::function<void (rif_t::endpoint_t const&)> fun
                                     )
    {
      std::list<std::future<void>> background_tasks;
      for (const rif_t::endpoint_t& rif : rifs)
      {
        background_tasks.emplace_back
          (std::async ( std::launch::async
                      , [&rif, &fun]
                        {
                          fun (rif);
                        }
                      )
          );
      }
      std::vector<std::string> accumulated_whats;
      for (std::future<void>& future : background_tasks)
      {
        try
        {
          future.get();
        }
        catch (std::exception const& ex)
        {
          accumulated_whats.emplace_back (ex.what());
        }
      }
      if (!accumulated_whats.empty())
      {
        throw std::runtime_error (fhg::util::join (accumulated_whats, ", "));
      }
    }
  }

  rif_t::endpoint_t::endpoint_t (const std::string& host, unsigned short port)
    : host (host)
    , port (port)
  {}

  bool rif_t::endpoint_t::operator< (const rif_t::endpoint_t& other) const
  {
    return host < other.host;
  }
  bool rif_t::endpoint_t::operator== (const rif_t::endpoint_t& other) const
  {
    return host == other.host;
  }

  rif_t::child_t::child_t (const rif_t::endpoint_t& rif, const pid_t pid)
    : _rif (rif)
    , _pid (pid)
  {}

  rif_t::child_t::~child_t ()
  {
    rexec_no_pid_returned
      (_rif, "/usr/bin/env kill -TERM " + std::to_string (_pid));
  }

  void rif_t::exec ( const std::list<rif_t::endpoint_t>& rifs
                   , const std::string& key
                   , std::string const& startup_messages_pipe_option
                   , std::string const& end_sentinel_value
                   , boost::filesystem::path const& command
                   , const std::vector<std::string>& raw_arguments
                   , const std::map<std::string, std::string>& raw_environment
                   , boost::filesystem::path const& gspc_home
                   )
  {
    run_asynchronously_and_wait
      ( rifs
      , [ this, &key, &startup_messages_pipe_option, &end_sentinel_value
        , &command, &raw_arguments, &raw_environment, &gspc_home
        ] (rif_t::endpoint_t const& rif)
        {
          {
            std::lock_guard<std::mutex> const _ (_processes_mutex);
            if (_processes[rif].count (key) > 0)
            {
              throw std::runtime_error
                ("key '" + key + "' is already in use on rif " + rif.host);
            }
          }

          // this should be handled by the remote rif
          std::string environment_string;
          for (std::pair<std::string, std::string> const& var : raw_environment)
          {
            environment_string += " --environment " + var.first + "='"
              + var.second + "'";
          }
          std::string arguments_string;
          for (std::string const& arg : raw_arguments)
          {
            arguments_string += " --arguments " + arg;
          }

          std::unique_ptr<child_t> child
            ( fhg::util::make_unique<child_t>
                ( rif
                , rexec ( rif
                        , ( "'" + (gspc_home / "bin" / "start-and-fork").string() + "'"
                          + " --end-sentinel-value " + end_sentinel_value
                          + " --startup-messages-pipe-option " + startup_messages_pipe_option
                          + " --command '" + command.string() + "'"
                          + arguments_string
                          + environment_string
                          )
                        )
                )
            );

          std::lock_guard<std::mutex> const _ (_processes_mutex);
          _processes[rif][key].push_back (std::move (child));
        }
      );
  }

  void rif_t::stop ( const std::list<rif_t::endpoint_t>& rifs
                   , const std::string& key
                   )
  {
    run_asynchronously_and_wait
      ( rifs
      , [this, &key] (rif_t::endpoint_t const& rif)
        {
          if (! _processes[rif].erase (key))
          {
            throw std::invalid_argument ("no such key: " + key);
          }
        }
      );
    for (const endpoint_t& rif : rifs)
    {
      if (_processes[rif].empty())
      {
        _processes.erase (rif);
      }
    }
  }
}
