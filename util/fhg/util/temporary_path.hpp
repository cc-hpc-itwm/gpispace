// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_TEMPORARY_PATH_HPP
#define FHG_UTIL_TEMPORARY_PATH_HPP

#include <boost/filesystem.hpp>

namespace fhg
{
  namespace util
  {
    class temporary_path
    {
    public:
      temporary_path()
        : _path (boost::filesystem::unique_path())
      {
        boost::filesystem::create_directories (_path);
      }
      ~temporary_path()
      {
        boost::filesystem::remove_all (_path);
      }
      operator boost::filesystem::path() const
      {
        return _path;
      }
      operator std::string() const
      {
        return _path.string();
      }

    private:
      boost::filesystem::path _path;
    };
  }
}

#endif
