// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_TEMPORARY_FILE_HPP
#define FHG_UTIL_TEMPORARY_FILE_HPP

#include <boost/filesystem.hpp>

namespace fhg
{
  namespace util
  {
    class temporary_file : boost::noncopyable
    {
    public:
      temporary_file (boost::filesystem::path const& path)
        : _path (path)
      {}

      ~temporary_file()
      {
        boost::filesystem::remove (_path);
      }

    private:
      boost::filesystem::path _path;
    };
  }
}

#endif
