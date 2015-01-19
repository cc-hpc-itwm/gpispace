// bernd.loerwald@itwm.fraunhofer.de

#include <drts/scoped_rifd.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <fhg/util/read_lines.hpp>

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

  scoped_rifd::scoped_rifd ( boost::program_options::variables_map const& vm
                           , installation const& installation
                           )
    : scoped_rifd
        ( require_rif_strategy (vm)
        , fhg::util::read_lines (require_nodefile (vm))
        , get_rif_port (vm)
        , installation.gspc_home()
        )
  {}


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

  rifd_entry_points scoped_rifd::entry_points() const
  {
    return {new rifd_entry_points::implementation (_->_entry_points)};
  }
}
