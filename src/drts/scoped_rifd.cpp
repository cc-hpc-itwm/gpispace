// bernd.loerwald@itwm.fraunhofer.de

#include <drts/scoped_rifd.hpp>

#include <drts/drts.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/option.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <util-generic/read_lines.hpp>

#include <rif/strategy/meta.hpp>

namespace gspc
{
  namespace rifd
  {
    PIMPL_IMPLEMENTATION
      (strategy, std::pair<std::string, std::vector<std::string>>)
    PIMPL_IMPLEMENTATION (hostnames, std::vector<std::string>)
    PIMPL_IMPLEMENTATION (hostname, std::string)
    PIMPL_IMPLEMENTATION (port, boost::optional<unsigned short>)

    strategy::strategy (boost::program_options::variables_map const& vm)
      : _ ( new implementation
              (  std::make_pair ( require_rif_strategy (vm)
                                , require_rif_strategy_parameters (vm)
                                )
              )
          )
    {}

    hostnames::hostnames (boost::program_options::variables_map const& vm)
      : _ (new implementation (fhg::util::read_lines (require_nodefile (vm))))
    {}
    hostnames::hostnames (std::vector<std::string> const& hostnames)
      : _ (new implementation (hostnames))
    {}
    hostname::hostname (std::string const& host)
      : _ (new implementation (host))
    {}

    port::port (boost::program_options::variables_map const& vm)
      : _ (new implementation (get_rif_port (vm)))
    {}
  }

  struct scoped_rifds::implementation
  {
    implementation ( std::string const& strategy
                   , std::vector<std::string> const& parameters
                   , std::vector<std::string> const& hostnames
                   , boost::optional<unsigned short> const& rifd_port
                   , boost::filesystem::path const& gspc_home
                   )
      : _strategy (strategy)
      , _parameters (parameters)
      , _entry_points ( fhg::rif::strategy::bootstrap ( _strategy
                                                      , hostnames
                                                      , rifd_port
                                                      , gspc_home
                                                      , _parameters
                                                      )
                      )
    {}
    ~implementation()
    {
      //! \todo somehow report the failed teardown and how to get rid
      //! of left-overs (see #482 Clean shutdown binary)
      std::vector<fhg::rif::entry_point> failed_entry_points;
      fhg::rif::strategy::teardown
        ( _strategy
        , _entry_points
        , failed_entry_points
        , _parameters
        );
    }

    std::string _strategy;
    std::vector<std::string> _parameters;
    std::vector<fhg::rif::entry_point> _entry_points;
  };

  scoped_rifds::scoped_rifds ( rifd::strategy const& strategy
                             , rifd::hostnames const& hostnames
                             , rifd::port const& port
                             , installation const& installation
                             )
    : _ (new implementation ( strategy._->_.first
                            , strategy._->_.second
                            , hostnames._->_
                            , port._->_
                            , installation.gspc_home()
                            )
        )
  {}

  PIMPL_DTOR (scoped_rifds)

  rifd_entry_points scoped_rifds::entry_points() const
  {
    return {new rifd_entry_points::implementation (_->_entry_points)};
  }

  struct scoped_rifd::implementation : public scoped_rifds::implementation
  {
    implementation ( std::string const& strategy
                   , std::vector<std::string> const& parameters
                   , std::string const& host
                   , boost::optional<unsigned short> const& rifd_port
                   , boost::filesystem::path const& gspc_home
                   )
      : scoped_rifds::implementation (strategy, parameters, {host}, rifd_port, gspc_home)
    {}
  };

  scoped_rifd::scoped_rifd ( rifd::strategy const& strategy
                           , rifd::hostname const& hostname
                           , rifd::port const& port
                           , installation const& installation
                           )
    : _ (new implementation ( strategy._->_.first
                            , strategy._->_.second
                            , hostname._->_
                            , port._->_
                            , installation.gspc_home()
                            )
        )
  {}

  PIMPL_DTOR (scoped_rifd)

  rifd_entry_point scoped_rifd::entry_point() const
  {
    return {new rifd_entry_point::implementation (_->_entry_points.front())};
  }
}
