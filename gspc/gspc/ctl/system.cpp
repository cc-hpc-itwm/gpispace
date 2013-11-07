#include "system.hpp"

#include <errno.h>  // errno
#include <stdlib.h> // getenv, abort
#include <string.h> // strerror

#include <iostream>

#include <boost/filesystem/path.hpp>

#include <fhg/util/program_info.h>
#include <fhg/util/get_home_dir.hpp>

namespace fs = boost::filesystem;

namespace gspc
{
  namespace ctl
  {
    static fs::path s_get_root_path ()
    {
      if (getenv ("GSPC_HOME") != 0)
      {
        return fs::path (getenv ("GSPC_HOME"));
      }
      else
      {
        namespace fs = boost::filesystem;

        char buf [4096];
        int rc;

        rc = fhg_get_executable_path (buf, sizeof (buf));
        if (rc < 0)
        {
          std::cerr << "could not get path of executable: " << strerror (errno)
                    << std::endl;
          std::cerr << "try to export GSPC_HOME" << std::endl;
          abort ();
        }

        return fs::path (buf).parent_path ().parent_path ();
      }
    }

    static fs::path const & s_root_path ()
    {
      static fs::path p (s_get_root_path ());
      return p;
    }

    /**
       get the base directory of the installation
     */
    std::string const & root_path ()
    {
      static std::string s (s_root_path ().string ());
      return s;
    }

    /**
       get the 'etc' directory, containing configuration files
     */
    std::string const & etc_path ()
    {
      static std::string s ((s_root_path () / "etc").string ());
      return s;
    }

    /**
       get the 'bin' directory, containing executables
     */
    std::string const & bin_path ()
    {
      static std::string s ((s_root_path () / "bin").string ());
      return s;
    }

    /**
       get the 'lib' directory, containing runtime libraries
     */
    std::string const & lib_path ()
    {
      static std::string s ((s_root_path () / "lib").string ());
      return s;
    }

    /**
       get the 'inc' directory, containing runtime headers
     */
    std::string const & inc_path ()
    {
      static std::string s ((s_root_path () / "include").string ());
      return s;
    }

    /**
       get the 'exec' directory, containing 'gspc' tools
     */
    std::string const & exec_path ()
    {
      static std::string s ((s_root_path () / "libexec" / "gspc" / "scripts").string ());
      return s;
    }

    /**
       get the 'plugin' directory, containing gspc plugins
     */
    std::string const & plugin_path ()
    {
      static std::string s ((s_root_path () / "libexec" / "fhg" / "plugins").string ());
      return s;
    }

    /**
       get the user's home directory
     */
    std::string const & user_home ()
    {
      static std::string s (fhg::util::get_home_dir ());
      return s;
    }

    /**
       get the user's config file, i.e. ~/.gspc.rc
     */
    std::string const & user_config_file ()
    {
      static std::string s ((fs::path (user_home ()) / ".gspc.rc").string ());
      return s;
    }

    /**
       get the site config file, usually <root-path>/etc/gspc.rc
     */
    std::string const & site_config_file ()
    {
      static std::string s ((fs::path (etc_path ()) / "gspc.rc").string ());
      return s;
    }

    /**
       get the system config file, usually /etc/gspc.rc
     */
    std::string const & system_config_file ()
    {
      static std::string s ("/etc/gspc.rc");
      return s;
    }
  }
}
