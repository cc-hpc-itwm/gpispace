// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
    installation_path (boost::filesystem::path const& gspc_home)
      : boost::filesystem::path (gspc_home)
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
    boost::filesystem::path libexec_gspc() const
    {
      return *this / "libexec" / "gspc";
    }
    boost::filesystem::path agent() const
    {
      return libexec_gspc() / "agent";
    }
    boost::filesystem::path drts_kernel() const
    {
      return libexec_gspc() / "drts-kernel";
    }
    boost::filesystem::path vmem() const
    {
      return libexec_gspc() / "gpi-space";
    }
    boost::filesystem::path logging_demultiplexer() const
    {
      return libexec_gspc() / "gspc-logging-demultiplexer.exe";
    }
  };
}
