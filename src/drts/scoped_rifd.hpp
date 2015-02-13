// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_GSPC_SCOPED_RIFD_HPP
#define FHG_GSPC_SCOPED_RIFD_HPP

#include <drts/scoped_rifd.fwd.hpp>

#include <drts/drts.fwd.hpp>
#include <drts/rifd_entry_points.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace gspc
{
  namespace rifd
  {
#define PIMPL(_name)                      \
      private:                            \
        friend class ::gspc::scoped_rifd; \
        struct implementation;            \
        implementation* _;                \
      public:                             \
        ~_name()

    struct strategy
    {
      strategy (boost::program_options::variables_map const&);

      PIMPL (strategy);
    };

    struct hostnames
    {
      hostnames (boost::program_options::variables_map const&);
      hostnames (std::vector<std::string> const&);

      PIMPL (hostnames);
    };
    struct port
    {
      port (boost::program_options::variables_map const&);

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

    ~scoped_rifd();

    rifd_entry_points entry_points() const;

  private:
    struct implementation;
    implementation* _;
  };
}

#endif
