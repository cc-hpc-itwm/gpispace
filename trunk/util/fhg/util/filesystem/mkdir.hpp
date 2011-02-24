// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_FILESYSTEM_MKDIR
#define FHG_UTIL_FILESYSTEM_MKDIR 1

#include <boost/filesystem.hpp>

namespace fhg
{
  namespace util
  {
    inline bool mkdir (const boost::filesystem::path & path)
    {
      return
        (  boost::filesystem::exists (path)
        || boost::filesystem::create_directory (path)
        )
        && boost::filesystem::is_directory (path)
        ;
    }

    inline bool mkdirs (const boost::filesystem::path & path)
    {
      bool res (true);

      boost::filesystem::path stack;

      if (path.is_relative())
        {
          stack = boost::filesystem::path (".");
        }

      for ( boost::filesystem::path::iterator pos (path.begin())
          ; res && pos != path.end()
          ; ++pos
          )
        {
          stack = stack / *pos;

          res &= mkdir (stack);
        }

      return res;
    }
  }
}

#endif
