// Copyright (C) 2020-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/RuntimeSystem.hpp>
#include <gspc/iml/gaspi/NetdevID.hpp>
#include <gspc/iml/rif/EntryPoints.hpp>
#include <gspc/rif/entry_point.hpp>

#include <chrono>
#include <filesystem>
#include <mutex>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>

namespace gspc::iml
{
    enum class component_type
    {
      vmem,
    };

    // \todo Move to RuntimeSystem.cpp.
    struct RuntimeSystem::ProcessesStorage
    {
      std::mutex _guard;
      std::unordered_map
        < gspc::rif::entry_point
        , std::unordered_map<std::string /*name*/, pid_t>
        > _;

      ProcessesStorage (std::ostream& info_output)
        : _info_output (info_output)
      {}

      ~ProcessesStorage();
      ProcessesStorage (ProcessesStorage const&) = delete;
      ProcessesStorage (ProcessesStorage&&) = delete;
      ProcessesStorage& operator= (ProcessesStorage const&)
        = delete;
      ProcessesStorage& operator= (ProcessesStorage&&)
        = delete;

      void store
        ( gspc::rif::entry_point const&
        , std::string const& name
        , pid_t
        );
      std::optional<pid_t> pidof
        ( gspc::rif::entry_point const&
        , std::string const& name
        );

      void startup
        ( std::filesystem::path gpi_socket
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
