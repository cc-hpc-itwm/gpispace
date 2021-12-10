// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/RuntimeSystem.hpp>
#include <iml/gaspi/NetdevID.hpp>
#include <iml/rif/EntryPoint.hpp>
#include <iml/rif/EntryPoints.hpp>
#include <iml/rif/protocol.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>

namespace iml
{
    enum class component_type
    {
      vmem,
    };

    // \todo Move to RuntimeSystem.cpp.
    struct RuntimeSystem::ProcessesStorage : ::boost::noncopyable
    {
      std::mutex _guard;
      std::unordered_map < rif::EntryPoint
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      ProcessesStorage (std::ostream& info_output)
        : _info_output (info_output)
      {}

      ~ProcessesStorage();

      void store (rif::EntryPoint const&, std::string const& name, pid_t);
      ::boost::optional<pid_t> pidof
        (rif::EntryPoint const&, std::string const& name);

      void startup
        ( ::boost::filesystem::path gpi_socket
        , std::chrono::seconds vmem_startup_timeout
        , unsigned short vmem_port
        , gaspi::NetdevID vmem_netdev_id
        , rif::EntryPoints const&
        , std::ostream& info_output
        );

    private:
      std::ostream& _info_output;
    };
}
