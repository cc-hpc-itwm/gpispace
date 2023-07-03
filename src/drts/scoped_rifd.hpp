// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <drts/scoped_rifd.fwd.hpp>

#include <drts/drts.fwd.hpp>
#include <drts/rifd_entry_points.hpp>

#include <drts/pimpl.hpp>

#include <boost/filesystem/path.hpp>
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

    GSPC_DLLEXPORT ::boost::program_options::options_description scoped_rifd
      (int = rifd::nodefile | rifd::rif_strategy | rifd::rif_port);
  }

  namespace rifd
  {
    struct GSPC_DLLEXPORT strategy
    {
      strategy (::boost::program_options::variables_map const&);

      strategy (strategy const&) = delete;
      strategy& operator= (strategy const&) = delete;
      strategy (strategy&&) = default;
      strategy& operator= (strategy&&) = default;

    private:
      friend class ::gspc::rifds;

      PIMPL (strategy);
    };

    struct GSPC_DLLEXPORT hostnames
    {
      hostnames (::boost::program_options::variables_map const&);
      hostnames (std::vector<std::string> const&);

      hostnames (hostnames const&) = delete;
      hostnames& operator= (hostnames const&) = delete;
      hostnames (hostnames&&) = default;
      hostnames& operator= (hostnames&&) = default;

    private:
      friend class ::gspc::rifds;

      PIMPL (hostnames);
    };
    struct GSPC_DLLEXPORT hostname
    {
      hostname (std::string const&);

      hostname (hostname const&) = delete;
      hostname& operator= (hostname const&) = delete;
      hostname (hostname&&) = default;
      hostname& operator= (hostname&&) = default;

    private:
      friend class ::gspc::scoped_rifd;

      PIMPL (hostname);
    };
    struct GSPC_DLLEXPORT port
    {
      port (::boost::program_options::variables_map const&);

      port (port const&) = delete;
      port& operator= (port const&) = delete;
      port (port&&) = default;
      port& operator= (port&&) = default;

    private:
      friend class ::gspc::rifds;

      PIMPL (port);
    };
  }

  class GSPC_DLLEXPORT rifds
  {
  public:
    rifds ( rifd::strategy const&
          , rifd::port const&
          , installation const&
          );

    rifds (rifds const&) = delete;
    rifds (rifds&&) = delete;
    rifds& operator= (rifds const&) = delete;
    rifds& operator= (rifds&&) = delete;

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
              , ::boost::filesystem::path const& command
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

  class GSPC_DLLEXPORT scoped_rifd : public rifds
  {
  public:
    //! \note The drts client constructor/destructor is not thread-safe
    // with the scoped_rifd's constructor/destructor. However, such cases, when
    // they are called concurrently, are extremly unlikely to happen in practice.
    scoped_rifd ( rifd::strategy const&
                , rifd::hostname const&
                , rifd::port const&
                , installation const&
                , std::ostream& = std::cout
                );
    ~scoped_rifd(); //! \todo report the failed entry points
    scoped_rifd (scoped_rifd const&) = delete;
    scoped_rifd (scoped_rifd&&) = delete;
    scoped_rifd& operator= (scoped_rifd const&) = delete;
    scoped_rifd& operator= (scoped_rifd&&) = delete;
    rifd_entry_point entry_point() const;
  };

  class GSPC_DLLEXPORT scoped_rifds : public rifds
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
    scoped_rifds (scoped_rifds const&) = delete;
    scoped_rifds (scoped_rifds&&) = delete;
    scoped_rifds& operator= (scoped_rifds const&) = delete;
    scoped_rifds& operator= (scoped_rifds&&) = delete;
  };
}
