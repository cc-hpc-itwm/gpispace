// bernd.loerwald@itwm.fraunhofer.de

#include <drts/scoped_rifd.hpp>

#include <rif/strategy/meta.hpp>

namespace gspc
{
  struct scoped_rifd::implementation
  {
    implementation ( std::string const& strategy
                   , std::vector<std::string> const& hostnames
                   , boost::optional<unsigned short> const& rifd_port
                   , boost::filesystem::path const& gspc_home
                   )
      : _strategy (strategy)
      , _entry_points ( fhg::rif::strategy::bootstrap ( _strategy
                                                      , hostnames
                                                      , rifd_port
                                                      , gspc_home
                                                      )
                      )
    {}
    ~implementation()
    {
      //! \todo somehow report the failed teardown and how to get rid
      //! of left-overs (see #482 Clean shutdown binary)
      std::vector<fhg::rif::entry_point> failed_entry_points;
      fhg::rif::strategy::teardown
        (_strategy, _entry_points, failed_entry_points);
    }

    std::string _strategy;
    std::vector<fhg::rif::entry_point> _entry_points;
  };

  scoped_rifd::scoped_rifd ( std::string const& strategy
                           , std::vector<std::string> const& hostnames
                           , boost::optional<unsigned short> const& rifd_port
                           , boost::filesystem::path const& gspc_home
                           )
    : _ (new implementation (strategy, hostnames, rifd_port, gspc_home))
  {}

  scoped_rifd::~scoped_rifd()
  {
    delete _;
    _ = nullptr;
  }
}
