#include <vmem/segment/beegfs.hpp>

#include <util-generic/read_lines.hpp>

#include <beegfs/beegfs.h>

#include <boost/filesystem/path.hpp>

#include <string>

namespace fhg
{
  namespace vmem
  {
    namespace segment
    {
      namespace beegfs
      {
        namespace
        {
          boost::filesystem::path config_file (int fd)
          {
            char* raw_path (nullptr);
            if (!beegfs_getRuntimeConfigFile (fd, &raw_path))
            {
              throw requirements_not_met
                ( "unable to get config file of BeeGFS mountpoint: "
                + std::string (strerror (errno))
                );
            }
            boost::filesystem::path path (raw_path);
            free (raw_path);
            return path;
          }
          bool tuneUseGlobalFileLocks (int fd)
          {
            for ( std::string const& line
                : fhg::util::read_lines (beegfs::config_file (fd))
                )
            {
              if (line == "tuneUseGlobalFileLocks = 1")
              {
                return true;
              }
              else if (line == "tuneUseGlobalFileLocks = 0")
              {
                return false;
              }
            }
            throw requirements_not_met
              ( "BeeGFS segment requires 'tuneUseGlobalFileLocks = 1'"
                " in BeeGFS config, but it isn't set"
              );
          }
        }

        void check_requirements (int fd)
        {
          if (!beegfs_testIsBeeGFS (fd))
          {
            throw requirements_not_met
              ("BeeGFS segment requires BeeGFS mountpoint");
          }
          if (!tuneUseGlobalFileLocks (fd))
          {
            throw requirements_not_met
              ( "BeeGFS segment requires 'tuneUseGlobalFileLocks = 1'"
                " in BeeGFS config"
              );
          }
        }
      }
    }
  }
}
