// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_TEMPORARY_PATH_HPP
#define FHG_UTIL_TEMPORARY_PATH_HPP

#include <boost/filesystem.hpp>

namespace fhg
{
  namespace util
  {
    class temporary_path : boost::noncopyable
    {
    public:
      temporary_path (boost::filesystem::path const& path)
        : _path (path)
      {
        boost::filesystem::create_directories (_path);
      }
      temporary_path()
        : temporary_path (boost::filesystem::unique_path())
      {}
      ~temporary_path()
      {
        boost::filesystem::remove_all (_path);
      }
      operator boost::filesystem::path() const
      {
        return _path;
      }

    private:
      boost::filesystem::path _path;
    };
  }
}

#endif
