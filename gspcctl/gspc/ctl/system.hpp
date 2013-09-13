#ifndef GSPC_CTL_SYSTEM_HPP
#define GSPC_CTL_SYSTEM_HPP

#include <string>

namespace gspc
{
  namespace ctl
  {
    /**
       get the base directory of the installation
     */
    std::string root_path ();

    /**
       get the 'etc' directory, containing configuration files
     */
    std::string etc_path ();

    /**
       get the 'bin' directory, containing executables
     */
    std::string bin_path ();

    /**
       get the 'lib' directory, containing runtime libraries
     */
    std::string lib_path ();

    /**
       get the 'inc' directory, containing runtime headers
     */
    std::string inc_path ();

    /**
       get the 'exec' directory, containing 'gspc' tools
     */
    std::string exec_path ();

    /**
       get the 'plugin' directory, containing gspc plugins
     */
    std::string plugin_path ();

    /**
       get the user's home directory
     */
    std::string user_home ();

    /**
       get the user's config file, i.e. ~/.gspc.rc
     */
    std::string user_config_file ();

    /**
       get the site config file, usually <root-path>/etc/gspc.rc
     */
    std::string site_config_file ();

    /**
       get the system config file, usually /etc/gspc.rc
     */
    std::string system_config_file ();
  }
}

#endif
