// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/RuntimeSystem.hpp>
#include <iml/gaspi/NetdevID.hpp>
#include <iml/rif/EntryPoint.hpp>
#include <iml/rif/EntryPoints.hpp>
#include <iml/rif/protocol.hpp>

#include <boost/filesystem/path.hpp>

#include <chrono>
#include <mutex>
#include <optional>
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
    struct RuntimeSystem::ProcessesStorage
    {
      std::mutex _guard;
      std::unordered_map < rif::EntryPoint
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      ProcessesStorage (std::ostream& info_output)
        : _info_output (info_output)
      {}

      ~ProcessesStorage();
      ProcessesStorage (ProcessesStorage const&) = delete;
      ProcessesStorage (ProcessesStorage&&) = delete;
      ProcessesStorage& operator= (ProcessesStorage const&) = delete;
      ProcessesStorage& operator= (ProcessesStorage&&) = delete;

      void store (rif::EntryPoint const&, std::string const& name, pid_t);
      std::optional<pid_t> pidof
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
