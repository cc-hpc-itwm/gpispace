#pragma once

#include <util-generic/executable_path.hpp>

#include <boost/filesystem/path.hpp>

namespace iml_client
{
  class installation_path : public boost::filesystem::path
  {
  public:
    installation_path()
      : boost::filesystem::path
        (fhg::util::executable_path().parent_path().parent_path())
    {}
    installation_path (boost::filesystem::path const& iml_home)
      : boost::filesystem::path (iml_home)
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
    boost::filesystem::path libexec() const
    {
      return *this / "libexec" / "iml";
    }
    boost::filesystem::path vmem() const
    {
      return libexec() / "iml-gpi-server";
    }
  };
}
