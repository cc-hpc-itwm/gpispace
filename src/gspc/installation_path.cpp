#include <gspc/installation_path.hpp>

#include <gspc/installation_sentinel.hpp>

namespace gspc
{
  installation_path::installation_path()
    : installation_path (installation_prefix())
  {}
  installation_path::installation_path (boost::filesystem::path const& gspc_home)
    : boost::filesystem::path (gspc_home)
  {}

  //! \todo configure
  boost::filesystem::path installation_path::include() const
  {
    return *this / "include";
  }
  boost::filesystem::path installation_path::lib() const
  {
    return *this / "lib";
  }
  boost::filesystem::path installation_path::boost_root() const
  {
    return *this / "external" / "boost";
  }

  namespace
  {
    boost::filesystem::path libexec_gspc (boost::filesystem::path const& root)
    {
      return root / "libexec" / "gspc";
    }
  }

  boost::filesystem::path installation_path::agent() const
  {
    return libexec_gspc (*this) / "agent";
  }
  boost::filesystem::path installation_path::drts_kernel() const
  {
    return libexec_gspc (*this) / "drts-kernel";
  }
  boost::filesystem::path installation_path::vmem() const
  {
    return libexec_gspc (*this) / "gpi-space";
  }
  boost::filesystem::path installation_path::logging_demultiplexer() const
  {
    return libexec_gspc (*this) / "gspc-logging-demultiplexer.exe";
  }
}
