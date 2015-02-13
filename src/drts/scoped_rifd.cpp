// bernd.loerwald@itwm.fraunhofer.de

#include <drts/scoped_rifd.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <fhg/util/read_lines.hpp>

#include <rif/strategy/meta.hpp>

namespace gspc
{
  namespace rifd
  {
#define IMPLEMENTATION(_name, _type...)                                 \
    struct _name::implementation                                        \
    {                                                                   \
      implementation (_type const& _name)                               \
        : _ (_name)                                                     \
      {}                                                                \
                                                                        \
      _type const _;                                                    \
    };                                                                  \
                                                                        \
    _name::~_name()                                                     \
    {                                                                   \
      delete _;                                                         \
      _ = nullptr;                                                      \
    }

    IMPLEMENTATION (strategy, std::string)
    IMPLEMENTATION (hostnames, std::vector<std::string>)
    IMPLEMENTATION (port, boost::optional<unsigned short>)

#undef IMPLEMENTATION

    strategy::strategy (boost::program_options::variables_map const& vm)
      : _ (new implementation (require_rif_strategy (vm)))
    {}

    hostnames::hostnames (boost::program_options::variables_map const& vm)
      : _ (new implementation (fhg::util::read_lines (require_nodefile (vm))))
    {}
    hostnames::hostnames (std::vector<std::string> const& hostnames)
      : _ (new implementation (hostnames))
    {}

    port::port (boost::program_options::variables_map const& vm)
      : _ (new implementation (get_rif_port (vm)))
    {}
  }

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

  scoped_rifd::scoped_rifd ( rifd::strategy const& strategy
                           , rifd::hostnames const& hostnames
                           , rifd::port const& port
                           , installation const& installation
                           )
    : _ (new implementation ( strategy._->_
                            , hostnames._->_
                            , port._->_
                            , installation.gspc_home()
                            )
        )
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
