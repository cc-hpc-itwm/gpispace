#include <gspc/installation_path.hpp>

#include <fhg/revision.hpp>
#include <gspc/installation_sentinel.hpp>

#include <util-generic/read_file.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

namespace gspc
{
  installation_path::installation_path()
    : installation_path (installation_prefix())
  {}
  installation_path::installation_path (boost::filesystem::path const& gspc_home)
    : boost::filesystem::path (gspc_home)
  {
    auto const path_revision (*this / "revision");

    if (!boost::filesystem::exists (path_revision))
    {
      throw std::invalid_argument
        (( boost::format ("GSPC revision mismatch: File '%1%' does not exist.")
         % path_revision
         ).str());
    }

    auto const revision (fhg::util::read_file (path_revision));

    if (revision != fhg::project_revision())
    {
      throw std::invalid_argument
        (( boost::format ( "GSPC revision mismatch: Expected '%1%'"
                         ", installation in '%2%' has version '%3%'"
                         )
         % fhg::project_revision()
         % *this
         % revision
         ).str()
        );
    }
  }

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
