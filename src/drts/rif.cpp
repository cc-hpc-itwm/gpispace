#include <drts/rif.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/make_unique.hpp>

#include <cstdio>

namespace gspc
{
  namespace
  {
    pid_t rexec ( const std::string& host
                , const std::vector<std::string>& cmd
                , const std::map<std::string, std::string>& environment
                )
    {
      pid_t child = fork ();
      if (0 == child)
      {
        std::vector<std::string> command {
          "ssh", "-q", "-n", "-tt", host, "/usr/bin/env"
            };
        for (auto kv : environment)
        {
          command.emplace_back (kv.first + "=\"" + kv.second + "\"");
        }
        command.insert (command.end(), cmd.begin(), cmd.end());

        std::vector<char> argv_buffer;
        std::vector<char*> argv;

        {
          std::vector<std::size_t> argv_offsets;

          for (const std::string& param : command)
          {
            const std::size_t pos (argv_buffer.size());
            argv_buffer.resize (argv_buffer.size() + param.size() + 1);
            std::copy (param.begin(), param.end(), argv_buffer.data() + pos);
            argv_buffer[argv_buffer.size() - 1] = '\0';
            argv_offsets.push_back (pos);
          }
          for (std::size_t offset : argv_offsets)
          {
            argv.push_back (argv_buffer.data() + offset);
          }
          argv.push_back (nullptr);
        }

        fhg::syscall::execvp (argv[0], argv.data());
      }
      else
      {
        return child;
      }
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

  rif_t::child_t::child_t (const pid_t pid)
    : _pid (pid)
  {}

  rif_t::child_t::~child_t ()
  {
    fhg::syscall::kill (_pid, SIGTERM);
    fhg::syscall::waitpid (_pid, nullptr, 0);
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
        (fhg::util::make_unique<child_t> (rexec (rif.host, command, environment)));
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
}
