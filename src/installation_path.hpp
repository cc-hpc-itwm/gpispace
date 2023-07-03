// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/executable_path.hpp>

#include <boost/filesystem/path.hpp>

namespace gspc
{
  class installation_path : public ::boost::filesystem::path
  {
  public:
    installation_path()
      : ::boost::filesystem::path
        (fhg::util::executable_path().parent_path().parent_path())
    {}
    installation_path (::boost::filesystem::path const& gspc_home)
      : ::boost::filesystem::path (gspc_home)
    {}

    //! \todo configure
    ::boost::filesystem::path include() const
    {
      return *this / "include";
    }
    ::boost::filesystem::path lib() const
    {
      return *this / "lib";
    }
    ::boost::filesystem::path boost_root() const
    {
      return *this / "external" / "boost";
    }
    ::boost::filesystem::path libexec_gspc() const
    {
      return *this / "libexec" / "gspc";
    }
    ::boost::filesystem::path agent() const
    {
      return libexec_gspc() / "agent";
    }
    ::boost::filesystem::path drts_kernel() const
    {
      return libexec_gspc() / "drts-kernel";
    }
    ::boost::filesystem::path logging_demultiplexer() const
    {
      return libexec_gspc() / "gspc-logging-demultiplexer.exe";
    }
  };
}
