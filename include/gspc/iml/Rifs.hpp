// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>
#include <gspc/iml/rif/EntryPoints.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace gspc::iml
{
  //! A scoped wrapper around \c gspc::rif::strategy bootstrap
  //! and teardown for convenience in most use cases.
  class GSPC_EXPORT Rifs
  {
  public:
    //! Start RIF daemons using the information given in
    //! \a vm, writing messages to \a output.
    //! \see options()
    Rifs ( ::boost::program_options::variables_map const& vm
         , std::ostream& output = std::cout
         );

    //! Start RIF daemons on every host given in
    //! \a hostnames using the strategy named \a strategy
    //! with the accompanying \a strategy_parameters. When
    //! given, the daemons will use \a port to listen on or
    //! an automatically determined one otherwise. The
    //! strategy may print messages to \a output.
    Rifs ( std::vector<std::string> const& hostnames
         , std::string const& strategy
         , std::vector<std::string> const&
             strategy_parameters = {}
         , std::optional<unsigned short> const&
             port = std::nullopt
         , std::ostream& = std::cout
         );

    //! Wrap already-bootstrapped entry points without
    //! owning their lifecycle. The destructor will still
    //! attempt to tear down the daemons.
    Rifs ( rif::EntryPoints entry_points
         , std::string strategy
         , std::vector<std::string> strategy_parameters
         );

    //! Retrieve the entry points started by this
    //! instance.
    operator rif::EntryPoints() const;

    //! Command line options for use in
    //! Boost.ProgramOptions, which can be used to produce
    //! the input for the \c vm overload of the
    //! constructor.
    static ::boost::program_options::options_description
      options();

    static void set_nodefile
      ( ::boost::program_options::variables_map&
      , std::filesystem::path
      );
    static void set_port
      ( ::boost::program_options::variables_map&
      , unsigned short
      );
    static void set_strategy
      ( ::boost::program_options::variables_map&
      , std::string
      );
    static void set_strategy_parameters
      ( ::boost::program_options::variables_map&
      , std::vector<std::string>
      );

    Rifs() = delete;
    Rifs (Rifs const&) = delete;
    Rifs (Rifs&&) = delete;
    Rifs& operator= (Rifs const&) = delete;
    Rifs& operator= (Rifs&&) = delete;
    ~Rifs();

  private:
    std::string _strategy;
    std::vector<std::string> _strategy_parameters;
    std::optional<unsigned short> _port;
    rif::EntryPoints _entry_points;
  };
}
