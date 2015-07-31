#pragma once

#include <drts/scoped_rifd.fwd.hpp>

#include <drts/drts.fwd.hpp>
#include <drts/rifd_entry_points.hpp>

#include <drts/pimpl.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <string>
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
    rifd_entry_points entry_points (rifd::hostnames const&) const;
    rifd_entry_points entry_points() const;

    rifd_entry_points bootstrap (rifd::hostnames const&);
    std::vector<std::string> teardown (rifd::hostnames const&);
    std::vector<std::string> teardown();

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
                );
    ~scoped_rifd(); //! \todo report the failed entry points
    rifd_entry_point entry_point() const;
  };

  class scoped_rifds : public rifds
  {
  public:
    scoped_rifds ( rifd::strategy const& strategy
                 , rifd::hostnames const& hostnames
                 , rifd::port const& port
                 , installation const& installation
                 )
      : rifds (strategy, port, installation)
    {
      bootstrap (hostnames);
    }
    ~scoped_rifds()
    {
      teardown(); //! \todo report the failed entry points
    }
  };
}
