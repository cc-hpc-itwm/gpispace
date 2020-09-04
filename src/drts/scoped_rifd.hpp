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

#include <drts/scoped_rifd.fwd.hpp>

#include <drts/drts.fwd.hpp>
#include <drts/rifd_entry_points.hpp>

#include <drts/pimpl.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gspc
{
  namespace options
  {
    enum rifd : int { nodefile = 1 << 0
                    , rif_strategy = 1 << 1
                    , rif_port = 1 << 2
                    };

    boost::program_options::options_description scoped_rifd
      (int = rifd::nodefile | rifd::rif_strategy | rifd::rif_port);
  }

  namespace rifd
  {
    struct strategy
    {
      strategy (boost::program_options::variables_map const&);

    private:
      friend class ::gspc::rifds;

      PIMPL (strategy);
    };

    struct hostnames
    {
      hostnames (boost::program_options::variables_map const&);
      hostnames (std::vector<std::string> const&);

    private:
      friend class ::gspc::rifds;

      PIMPL (hostnames);
    };
    struct hostname
    {
      hostname (std::string const&);

    private:
      friend class ::gspc::scoped_rifd;

      PIMPL (hostname);
    };
    struct port
    {
      port (boost::program_options::variables_map const&);

    private:
      friend class ::gspc::rifds;

      PIMPL (port);
    };
  }

  class rifds : boost::noncopyable
  {
  public:
    rifds ( rifd::strategy const&
          , rifd::port const&
          , installation const&
          );

    std::vector<std::string> hosts() const;
    std::pair< rifd_entry_points
             , std::pair< std::unordered_set<std::string> // known
                        , std::unordered_set<std::string> // unknown
                        >
             >
      entry_points (rifd::hostnames const&) const;
    rifd_entry_points entry_points() const;

    std::pair< rifd_entry_points
             , std::unordered_map<std::string, std::exception_ptr>
             >
      bootstrap (rifd::hostnames const&, std::ostream& = std::cout);

    std::pair < std::unordered_set<std::string>
              , std::unordered_map<std::string, std::exception_ptr>
              >
      teardown (rifd::hostnames const&);

    std::pair < std::unordered_set<std::string>
              , std::unordered_map<std::string, std::exception_ptr>
              >
      teardown (std::unordered_set<std::string> const& hostnames)
    {
      std::vector<std::string> hosts {hostnames.begin(), hostnames.end()};

      return teardown (hosts);
    }

    std::pair < std::unordered_set<std::string>
              , std::unordered_map<std::string, std::exception_ptr>
              >
      teardown();

    std::pair< std::unordered_map<std::string, std::vector<std::string>>
             , std::unordered_map<std::string, std::exception_ptr>
             >
      execute ( std::unordered_set<std::string> const& hostnames
              , boost::filesystem::path const& command
              , std::vector<std::string> const& arguments
                  = {}
              , std::unordered_map<std::string, std::string> const& environment
                  = std::unordered_map<std::string, std::string> {}
              ) const;

    PIMPL (rifds);

  private:
    friend class scoped_rifd;
    friend class scoped_rifds;
  };

  class scoped_rifd : public rifds
  {
  public:
    scoped_rifd ( rifd::strategy const&
                , rifd::hostname const&
                , rifd::port const&
                , installation const&
                , std::ostream& = std::cout
                );
    ~scoped_rifd(); //! \todo report the failed entry points
    rifd_entry_point entry_point() const;
  };

  class scoped_rifds : public rifds
  {
  public:
    using rifds::rifds;
    scoped_rifds ( rifd::strategy const&
                 , rifd::hostnames const&
                 , rifd::port const&
                 , installation const&
                 , std::ostream& = std::cout
                 );
    ~scoped_rifds(); //! \todo report the failed entry points
  };
}
