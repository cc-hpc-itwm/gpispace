#include "eval.hpp"

#include <sys/types.h>
#include <sys/wait.h>

#include <gspc/rif.hpp>

namespace gspc
{
  namespace ctl
  {
    namespace fs = boost::filesystem;

    int eval ( fs::path const & cmd
             , char *av[]
             , size_t ac
             , std::string & out
             , std::string & err
             , const std::string & inp
             )
    {
      gspc::rif::argv_t argv;
      gspc::rif::env_t env;
      int rc;

      argv.push_back (cmd.string ());
      for (size_t i = 0 ; i < ac ; ++i)
      {
        argv.push_back (av [i]);
      }

      gspc::rif::process_t proc (0, argv.front (), argv);

      rc = proc.fork_and_exec ();
      if (rc != 0)
      {
        return rc;
      }

      if (not inp.empty ())
      {
        proc.write (inp.c_str (), inp.size ());
      }

      while (proc.try_waitpid () != 0)
      {
        char buf [4096];
        rc = proc.read (buf, sizeof(buf));
        if (rc > 0)
          out += std::string (buf, rc);

        rc = proc.readerr (buf, sizeof(buf));
        if (rc > 0)
          err += std::string (buf, rc);

        usleep (1000);
      }

      rc = proc.waitpid ();
      rc = *proc.status ();

      if (WIFSIGNALED (rc))
        return 128 + WTERMSIG (rc);
      else
        return WEXITSTATUS (rc);
    }
  }
}
