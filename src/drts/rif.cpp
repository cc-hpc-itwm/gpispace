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

    static std::string const RIF_ROOT {"<rif-root>"};

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
    for (const endpoint_t& rif: rifs)
    {
      if (_processes[rif].find (key) != _processes[rif].end())
      {
        throw std::runtime_error ("key '" + key + "' is already in use on rif " + rif.host);
      }

      // this should be handled by the remote rif
      std::vector<std::string> command (raw_command);
      std::map<std::string, std::string> environment (raw_environment);
      for (auto& arg : command)
      {
        arg = replace_rif_root (arg, _root);
      }
      for (auto& kv : environment)
      {
        kv.second = replace_rif_root (kv.second, _root);
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
    return RIF_ROOT / p;
  }

  void rif_t::store ( const std::list<endpoint_t>& rifs
                    , const std::vector<char>& data
                    , const std::string& path
                    )
  {
    std::for_each (rifs.begin(), rifs.end(), [this, data, path] (endpoint_t const& rif) -> void {
        std::ostringstream command;
        command << "ssh -q -p " << rif.port << " " << rif.host
                << " '/bin/cat > " << replace_rif_root (path, _root) << "'";
        scoped_popen f (command.str().c_str(), "w");
        if (1 != fwrite (data.data(), data.size(), 1, f._))
        {
          throw std::runtime_error
            ("could not write to: " + path + " on " + rif.host);
        }
      }
    );
  }
}
