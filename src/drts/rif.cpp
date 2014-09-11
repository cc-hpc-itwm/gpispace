#include <drts/rif.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>

#include <sstream>
#include <future>

#include <cstdio>
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

    std::string replace_rif_root ( std::string s
                                 , boost::filesystem::path const& root
                                 )
    {
      return s.find (RIF_ROOT) == 0
        ? s.replace (0, RIF_ROOT.size(), root.string())
        : s
        ;
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
      command << " >/dev/null 2>/dev/null </dev/null & echo $!; disown -a'";
      command << " | ssh -q -p " << rif.port << " " << rif.host << " /bin/sh -s";

      char buf[16];
      scoped_popen pid_file (command.str().c_str(), "r");
      fhg::syscall::fread (buf, sizeof (buf), sizeof (char), pid_file._);
      errno = 0;
      pid_t pid = std::strtoul (buf, nullptr, 0);
      if (0 == pid && errno != 0)
      {
        throw std::runtime_error
          ( "could not get remote pid from buffer: \""
          + std::string (buf, buf+sizeof(buf))
          + "\": " + strerror (errno)
          );
      }
      return pid;
    }
  }

  rif_t::rif_t (boost::filesystem::path const& root)
    : _root (root)
  {}

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
    std::list<std::future<void>> background_tasks;
    for (const endpoint_t& rif : rifs)
    {
      background_tasks.push_back
        (std::move (std::async (std::launch::async, [this, &rif, &key, &raw_command, &raw_environment] {
            {
              std::lock_guard<std::mutex> const _ (_processes_mutex);
              if (_processes[rif].count (key) > 0)
              {
                throw std::runtime_error ("key '" + key + "' is already in use on rif " + rif.host);
              }
            }

            // this should be handled by the remote rif
            std::vector<std::string> command;
            std::map<std::string, std::string> environment (raw_environment);
            for (std::string const& arg : raw_command)
            {
              command.push_back (replace_rif_root (arg, _root));
            }
            for (std::pair<std::string, std::string> const& kv : raw_environment)
            {
              environment[kv.first] = replace_rif_root (kv.second, _root);
            }

            std::unique_ptr<child_t> child
              (fhg::util::make_unique<child_t> (rif, rexec (rif, command, environment)));

            std::lock_guard<std::mutex> const _ (_processes_mutex);
            _processes[rif][key].push_back (std::move (child));
            })
          ));
    }
    for (std::future<void> & f : background_tasks)
    {
      f.wait();
    }
  }

  void rif_t::stop ( const std::list<rif_t::endpoint_t>& rifs
                   , const std::string& key
                   )
  {
    std::list<std::future<void>> background_tasks;
    for (const endpoint_t& rif : rifs)
    {
      background_tasks.push_back
        (std::move (std::async (std::launch::async, [this, &rif, &key] {
              if (! _processes[rif].erase (key))
              {
                throw std::invalid_argument ("no such key: " + key);
              }
            })
          ));
    }
    for (std::future<void> & f : background_tasks)
    {
      f.wait();
    }
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
    std::list<std::future<void>> background_tasks;
    for (const endpoint_t& rif : rifs)
    {
      background_tasks.push_back
        (std::move (std::async (std::launch::async, [this, &rif, &data, &path] {
              std::ostringstream command;
              command << "ssh -q -p " << rif.port << " " << rif.host
                      << " '/bin/cat > " << replace_rif_root (path.string(), _root) << "'";
              scoped_popen f (command.str().c_str(), "w");
              if (1 != fwrite (data.data(), data.size(), 1, f._))
              {
                throw std::runtime_error
                  ("could not write to: " + path.string() + " on " + rif.host);
              }
            })
          ));
    }
    for (std::future<void> & f : background_tasks)
    {
      f.wait();
    }
  }
}
