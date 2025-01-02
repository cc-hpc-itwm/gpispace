// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>
#include <iml/gaspi/NetdevID.hpp>
#include <iml/rif/EntryPoints.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <chrono>
#include <iostream>
#include <memory>

namespace iml
{
  //! A set of distributed IML servers that will run as long as the
  //! \c RuntimeSystem object lives. IML servers manage and contain
  //! the global segments and allocations and can be interacted with
  //! using a \c Client via a local socket.
  class IML_DLLEXPORT RuntimeSystem
  {
  public:
    //! Start IML servers on the given \a entry_points, listening on
    //! \a socket for a local \c Client, using \a port to communicate
    //! between servers, giving the startup \a timeout to start and
    //! advising GPI to use \a netdev_id. While starting, output is
    //! written to \a output.
    //! \note Use \c Rifs to easily construct \a entry_points.
    //! \note \a socket and \a port are the same on every node and
    //! shall not exist/be used yet.
    //! \note While \a netdev_id is gaspi specific, it is required
    //! during bootstrapping and is independent of a specific segment
    //! or whether a gaspi segment is created.
    RuntimeSystem ( rif::EntryPoints const& entry_points
                  , ::boost::filesystem::path socket
                  , unsigned short port
                  , std::chrono::seconds timeout
                  , gaspi::NetdevID netdev_id = {}
                  , std::ostream& output = std::cout
                  );

    //! Start IML servers on the given \a entry_points using the
    //! information given in \a vm, writing messages to \a output.
    RuntimeSystem ( rif::EntryPoints const& entry_points
                  , ::boost::program_options::variables_map const& vm
                  , std::ostream& output = std::cout
                  );

    //! Command line options for use in Boost.ProgramOptions, which
    //! can be used to produce the input for the \c vm overload of the
    //! constructor.
    //! \see set_socket(), set_port(), set_startup_timeout(),
    //! set_netdev_id()
    static ::boost::program_options::options_description options();

    //! Overwrite the socket to be used to connect to the server.
    static void set_socket
      (::boost::program_options::variables_map&, ::boost::filesystem::path);
    //! Overwrite the port used for communication between servers.
    static void set_port
      (::boost::program_options::variables_map&, unsigned short);
    //! Overwrite the timeout used when starting servers.
    static void set_startup_timeout
      (::boost::program_options::variables_map&, std::chrono::seconds);
    //! Overwrite the netdev ID to be passed to GPI.
    static void set_netdev_id
      (::boost::program_options::variables_map&, iml::gaspi::NetdevID);

    RuntimeSystem() = delete;
    RuntimeSystem (RuntimeSystem const&) = delete;
    RuntimeSystem& operator= (RuntimeSystem const&) = delete;
    RuntimeSystem (RuntimeSystem&&) = delete;
    RuntimeSystem& operator= (RuntimeSystem&&) = delete;
    ~RuntimeSystem();

  private:
    struct ProcessesStorage;
    std::unique_ptr<ProcessesStorage> _processes_storage;
  };
}
