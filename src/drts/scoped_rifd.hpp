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
      friend class ::gspc::scoped_rifds;
      friend class ::gspc::scoped_rifd;

      PIMPL (strategy);
    };

    struct hostnames
    {
      hostnames (boost::program_options::variables_map const&);
      hostnames (std::vector<std::string> const&);

    private:
      friend class ::gspc::scoped_rifds;

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
      friend class ::gspc::scoped_rifds;
      friend class ::gspc::scoped_rifd;

      PIMPL (port);
    };
  }

  class scoped_rifd : boost::noncopyable
  {
  public:
    scoped_rifd ( rifd::strategy const&
                , rifd::hostname const&
                , rifd::port const&
                , installation const&
                );

    rifd_entry_point entry_point() const;

    PIMPL (scoped_rifd);
  };
  class scoped_rifds : boost::noncopyable
  {
  public:
    scoped_rifds ( rifd::strategy const&
                 , rifd::hostnames const&
                 , rifd::port const&
                 , installation const&
                 );

    rifd_entry_points entry_points() const;

    PIMPL (scoped_rifds);

  private:
    friend class scoped_rifd;
  };
}
