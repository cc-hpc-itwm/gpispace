#include <drts/rif.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>

#include <sstream>

#include <cstdio>
#include <cstdlib>

namespace gspc
{
  namespace
  {
    struct scoped_FILE
    {
      explicit scoped_FILE (FILE *file)
        : _ (file)
      {}

      ~scoped_FILE()
      {
        if (_)
        {
          fhg::syscall::pclose (_);
        }
      }

      FILE *_;
    };

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
      scoped_FILE pid_file (fhg::syscall::popen (command.str().c_str(), "r"));
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
                   , const std::vector<std::string>& command
                   , const std::map<std::string, std::string>& environment
                   )
  {
    for (const endpoint_t& rif: rifs)
    {
      if (_processes[rif].find (key) != _processes[rif].end())
      {
        throw std::runtime_error ("key '" + key + "' is already in use on rif " + rif.host);
      }

      _processes[rif][key].push_back
        (fhg::util::make_unique<child_t> (rif, rexec (rif, command, environment)));
    }
  }

  void rif_t::stop ( const std::list<rif_t::endpoint_t>& rifs
                   , const std::string& key
                   )
  {
    for (const endpoint_t& rif: rifs)
    {
      _processes[rif].at (key).clear ();
      _processes[rif].erase (key);

      if (_processes[rif].empty())
      {
        _processes.erase (rif);
      }
    }
  }

  boost::filesystem::path
  rif_t::make_relative_to_rif_root (boost::filesystem::path const& p)
  {
    return "<rif-root>" / p;
  }
}
