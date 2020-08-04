#pragma once

#include <boost/filesystem/path.hpp>

namespace gspc
{
  class installation_path : public boost::filesystem::path
  {
  public:
    installation_path();
    installation_path (boost::filesystem::path const& gspc_home);

    boost::filesystem::path include() const;
    boost::filesystem::path lib() const;
    boost::filesystem::path boost_root() const;
    boost::filesystem::path libexec_gspc() const;
    boost::filesystem::path agent() const;
    boost::filesystem::path drts_kernel() const;
    boost::filesystem::path vmem() const;
    boost::filesystem::path logging_demultiplexer() const;
  };
}
