// bernd.loerwald@itwm.fraunhofer.de

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
    boost::program_options::options_description scoped_rifd();
  }

  namespace rifd
  {
    struct strategy
    {
      strategy (boost::program_options::variables_map const&);

    private:
      friend class ::gspc::scoped_rifd;

      PIMPL (strategy);
    };

    struct hostnames
    {
      hostnames (boost::program_options::variables_map const&);
      hostnames (std::vector<std::string> const&);

    private:
      friend class ::gspc::scoped_rifd;

      PIMPL (hostnames);
    };
    struct port
    {
      port (boost::program_options::variables_map const&);

    private:
      friend class ::gspc::scoped_rifd;

      PIMPL (port);
    };
  }

  class scoped_rifd : boost::noncopyable
  {
  public:
    scoped_rifd ( rifd::strategy const&
                , rifd::hostnames const&
                , rifd::port const&
                , installation const&
                );

    rifd_entry_points entry_points() const;

    PIMPL (scoped_rifd);
  };
}
