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
    std::string root_path ()
    {
      return s_root_path ().string ();
    }

    /**
       get the 'etc' directory, containing configuration files
     */
    std::string etc_path ()
    {
      return (s_root_path () / "etc").string ();
    }

    /**
       get the 'bin' directory, containing executables
     */
    std::string bin_path ()
    {
      return (s_root_path () / "bin").string ();
    }

    /**
       get the 'lib' directory, containing runtime libraries
     */
    std::string lib_path ()
    {
      return (s_root_path () / "lib").string ();
    }

    /**
       get the 'inc' directory, containing runtime headers
     */
    std::string inc_path ()
    {
      return (s_root_path () / "include").string ();
    }

    /**
       get the 'exec' directory, containing 'gspc' tools
     */
    std::string exec_path ()
    {
      return (s_root_path () / "libexec" / "gspc" / "scripts").string ();
    }

    /**
       get the 'plugin' directory, containing gspc plugins
     */
    std::string plugin_path ()
    {
      return (s_root_path () / "libexec" / "fhg" / "plugins").string ();
    }

    /**
       get the user's home directory
     */
    std::string user_home ()
    {
      return fhg::util::get_home_dir ();
    }

    /**
       get the user's config file, i.e. ~/.gspc.rc
     */
    std::string user_config_file ()
    {
      return (fs::path (user_home ()) / ".gspc.rc").string ();
    }

    /**
       get the site config file, usually <root-path>/etc/gspc.rc
     */
    std::string site_config_file ()
    {
      return (fs::path (etc_path ()) / "gspc.rc").string ();
    }

    /**
       get the system config file, usually /etc/gspc.rc
     */
    std::string system_config_file ()
    {
      return "/etc/gspc.rc";
    }
  }
}
