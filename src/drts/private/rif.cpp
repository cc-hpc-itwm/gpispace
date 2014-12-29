#include <drts/private/rif.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

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

    std::string const RIF_ROOT {"<rif-root>"};

    std::string replace_rif_root ( std::string const& s
                                 , boost::filesystem::path const& root
                                 )
    {
      return boost::algorithm::replace_all_copy (s, RIF_ROOT, root.string());
    }

    pid_t rexec ( const rif_t::endpoint_t rif
                , const std::vector<std::string>& cmd
                , const std::map<std::string, std::string>& environment
                )
    {
      std::ostringstream command;
      command << "echo '/usr/bin/env";
      for (auto kv : environment)
      {
        command << " " << kv.first << "=\"" << kv.second << "\"";
      }
      for (auto arg : cmd)
      {
        command << " " << arg;
      }
      command << " | ssh -q " << rif.host << " /usr/bin/env bash -s";

      std::string buf;
      const scoped_popen pid_file (command.str().c_str(), "r");
      while (!feof (pid_file._))
      {
        char c;
        if (1 != ::fread (&c, sizeof (char), 1, pid_file._))
        {
          const int ec (::ferror (pid_file._));
          if (ec != 0)
          {
            throw std::runtime_error
              ("could not fread() remote pid: " + std::to_string (ec));
          }
        }
        else
        {
          buf += c;
        }
      }

      errno = 0;
      pid_t pid = std::strtoul (buf.c_str(), nullptr, 0);
      if (0 == pid && errno != 0)
      {
        throw std::runtime_error
          ("could not get remote pid from buffer: \"" + buf + "\"" + strerror (errno));
      }
      return pid;
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
      for (std::future<void> & f : background_tasks)
      {
        f.wait();
      }
    }
  }

  rif_t::rif_t (boost::filesystem::path const& root)
    : _root (root)
  {
    // FIXME: we explicitly don't remove this directory in the dtor
    // since we currently cannot ensure that it does not already
    // exist/is cleaned up correctly, since we don't fully control
    // remote rifs
    boost::filesystem::create_directories (root);
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
    rexec (_rif, {"kill", "-TERM", std::to_string (_pid)}, {});
  }

  void rif_t::exec ( const std::list<rif_t::endpoint_t>& rifs
                   , const std::string& key
                   , const std::vector<std::string>& raw_command
                   , const std::map<std::string, std::string>& raw_environment
                   )
  {
    run_asynchronously_and_wait
      ( rifs
      , [this, &key, &raw_command, &raw_environment] (rif_t::endpoint_t const& rif)
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
          std::vector<std::string> command;
          std::map<std::string, std::string> environment (raw_environment);
          for (std::string const& arg : raw_command)
          {
            command.push_back (replace_rif_root (arg, _root));
          }
          command.emplace_back
            (">/dev/null 2>/dev/null </dev/null & echo $!; disown $!'");
          for (std::pair<std::string, std::string> const& kv : raw_environment)
          {
            environment[kv.first] = replace_rif_root (kv.second, _root);
          }

          std::unique_ptr<child_t> child
            (fhg::util::make_unique<child_t> (rif, rexec (rif, command, environment)));

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

  boost::filesystem::path
  rif_t::make_relative_to_rif_root (boost::filesystem::path const& p)
  {
    return RIF_ROOT / p;
  }

  void rif_t::store ( const std::list<endpoint_t>& rifs
                    , const std::string& data
                    , const boost::filesystem::path& path
                    )
  {
    run_asynchronously_and_wait
      ( rifs
      , [this, &data, &path] (rif_t::endpoint_t const& rif)
        {
          const std::string real_path (replace_rif_root (path.string(), _root));
          std::ostringstream command;
          command << "ssh -q " << rif.host
                  << " 'mkdir -p $(dirname " << real_path << "); "
                  << " /bin/cat > " << real_path << "'";
          scoped_popen f (command.str().c_str(), "w");
          if (1 != fwrite (data.data(), data.size(), 1, f._))
          {
            throw std::runtime_error
              ("could not write to: " + path.string() + " on " + rif.host);
          }
        }
      );
  }
}
