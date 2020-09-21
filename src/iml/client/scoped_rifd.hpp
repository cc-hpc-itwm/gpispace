#pragma once

#include <iml/client/scoped_rifd.fwd.hpp>

#include <iml/client/iml.fwd.hpp>
#include <iml/client/rifd_entry_points.hpp>

#include <iml/client/iml.pimpl.hpp>

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

namespace iml_client
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

  namespace iml_rifd
  {
    struct strategy
    {
      strategy (boost::program_options::variables_map const&);

    private:
      friend class ::iml_client::rifds;

      PIMPL (strategy);
    };

    struct hostnames
    {
      hostnames (boost::program_options::variables_map const&);
      hostnames (std::vector<std::string> const&);

    private:
      friend class ::iml_client::rifds;

      PIMPL (hostnames);
    };
    struct hostname
    {
      hostname (std::string const&);

    private:
      friend class ::iml_client::scoped_rifd;

      PIMPL (hostname);
    };
    struct port
    {
      port (boost::program_options::variables_map const&);

    private:
      friend class ::iml_client::rifds;

      PIMPL (port);
    };
  }

  class rifds : boost::noncopyable
  {
  public:
    rifds ( iml_rifd::strategy const&
          , iml_rifd::port const&
          , installation const&
          );

    std::vector<std::string> hosts() const;
    std::pair< rifd_entry_points
             , std::pair< std::unordered_set<std::string> // known
                        , std::unordered_set<std::string> // unknown
                        >
             >
      entry_points (iml_rifd::hostnames const&) const;
    rifd_entry_points entry_points() const;

    std::pair< rifd_entry_points
             , std::unordered_map<std::string, std::exception_ptr>
             >
      bootstrap (iml_rifd::hostnames const&, std::ostream& = std::cout);

    std::pair < std::unordered_set<std::string>
              , std::unordered_map<std::string, std::exception_ptr>
              >
      teardown (iml_rifd::hostnames const&);

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

    PIMPL (rifds);

  private:
    friend class scoped_rifd;
    friend class scoped_rifds;
  };

  class scoped_rifd : public rifds
  {
  public:
    scoped_rifd ( iml_rifd::strategy const&
                , iml_rifd::hostname const&
                , iml_rifd::port const&
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
    scoped_rifds ( iml_rifd::strategy const&
                 , iml_rifd::hostnames const&
                 , iml_rifd::port const&
                 , installation const&
                 , std::ostream& = std::cout
                 );
    ~scoped_rifds(); //! \todo report the failed entry points
  };
}
