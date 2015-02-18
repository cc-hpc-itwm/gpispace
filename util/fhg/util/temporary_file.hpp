// mirko.rahn@itwm.fraunhofer.de

#pragma once

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

      operator boost::filesystem::path() const
      {
        return _path;
      }

    private:
      boost::filesystem::path _path;
    };
  }
}
