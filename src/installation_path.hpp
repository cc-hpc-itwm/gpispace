#pragma once

#include <util-generic/executable_path.hpp>

#include <boost/filesystem/path.hpp>

namespace gspc
{
  class installation_path : public boost::filesystem::path
  {
  public:
    installation_path()
      : boost::filesystem::path
        (fhg::util::executable_path().parent_path().parent_path())
    {}

    //! \todo configure
    boost::filesystem::path include() const
    {
      return *this / "include";
    }
    boost::filesystem::path lib() const
    {
      return *this / "lib";
    }
    boost::filesystem::path boost_root() const
    {
      return *this / "external" / "boost";
    }
  };
}
