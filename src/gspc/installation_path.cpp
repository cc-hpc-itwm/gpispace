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
  boost::filesystem::path installation_path::libexec_gspc() const
  {
    return *this / "libexec" / "gspc";
  }
  boost::filesystem::path installation_path::agent() const
  {
    return libexec_gspc() / "agent";
  }
  boost::filesystem::path installation_path::drts_kernel() const
  {
    return libexec_gspc() / "drts-kernel";
  }
  boost::filesystem::path installation_path::vmem() const
  {
    return libexec_gspc() / "gpi-space";
  }
  boost::filesystem::path installation_path::logging_demultiplexer() const
  {
    return libexec_gspc() / "gspc-logging-demultiplexer.exe";
  }
}
