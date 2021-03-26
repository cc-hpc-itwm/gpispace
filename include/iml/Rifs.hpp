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

#include <iml/detail/dllexport.hpp>
#include <iml/rif/EntryPoints.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional/optional.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace iml
{
  //! A scoped wrapper around \c rif::bootstrap() and \c
  //! rif::teardown() for convenience in most use cases.
  class IML_DLLEXPORT Rifs
  {
  public:
    //! Start RIF daemons using the information given in \a vm,
    //! writing messages to \a output.
    //! \see options()
    // \todo Alias for vm and option_description etc?
    Rifs ( boost::program_options::variables_map const& vm
         , std::ostream& output = std::cout
         );

    //! Start RIF daemons on every host given in \a hostnames using
    //! the strategy named \a strategy with the accompanying \a
    //! strategy_parameters. When given, the daemons will use \a port
    //! to listen on or an automatically determined one otherwise. The
    //! strategy may print messages to \a output.
    Rifs ( std::vector<std::string> const& hostnames
         , std::string const& strategy
         , std::vector<std::string> const& strategy_parameters = {}
         , boost::optional<unsigned short> const& port = boost::none
         , std::ostream& = std::cout
         );

    //! Retrieve the entry points started by this instance.
    operator rif::EntryPoints() const;

    //! Command line options for use in Boost.ProgramOptions, which
    //! can be used to produce the input for the \c vm overload of the
    //! constructor.
    //! \see set_nodefile(), set_port(), set_strategy(),
    //! set_strategy_parameters()
    static boost::program_options::options_description options();

    //! Overwrite the nodefile path that's read to determine the hosts
    //! to use in starting RIF daemons.
    // \todo Drop the setters? We have a ctor overload that takes them
    // all and the options() always add all of them. The only use case
    // for the setters is partially ignoring user input, which feels
    // like a bad UX and hopefully nobody does. Removing highlights the
    // "we take care of it" vs "you take care of it" model of the
    // ctors. The only current user is our testing helper to set the
    // nodefile from the environment (instead of configuring it like the
    // strategy).
    static void set_nodefile
      (boost::program_options::variables_map&, boost::filesystem::path);
    //! Overwrite the port to be used by the started RIF daemons.
    static void set_port
      (boost::program_options::variables_map&, unsigned short);
    //! Overwrite the strategy used to start RIF daemons.
    static void set_strategy
      (boost::program_options::variables_map&, std::string);
    //! Overwrite the strategy parameters used to start RIF daemons.
    static void set_strategy_parameters
      (boost::program_options::variables_map&, std::vector<std::string>);

    Rifs() = delete;
    Rifs (Rifs const&) = delete;
    Rifs (Rifs&&) = delete;
    Rifs& operator= (Rifs const&) = delete;
    Rifs& operator= (Rifs&&) = delete;
    // \todo Report the failures by giving users a pre-dtor teardown()?
    ~Rifs();

  private:
    std::string _strategy;
    std::vector<std::string> _strategy_parameters;
    boost::optional<unsigned short> _port;
    rif::EntryPoints _entry_points;
  };
}
