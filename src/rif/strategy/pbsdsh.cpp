#include <rif/strategy/pbsdsh.hpp>

#include <util-generic/blocked.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace pbsdsh
      {
        namespace
        {
          namespace option
          {
            namespace validator = fhg::util::boost::program_options;

            constexpr char const* const block_size {"pbsdshs-at-once"};
            constexpr char const* const block_size_description
              {"how many PBSDSH connections to establish at once"};
            using block_size_type = std::size_t;
            using block_size_validator
              = validator::positive_integral<std::size_t>;
            constexpr block_size_type const block_size_default = 64;
          }

          //! \todo use fhg::util::boost::program_options::generic
#define EXTRACT_PARAMETERS(parameters_)                                 \
          boost::program_options::options_description options;          \
          options.add_options()                                         \
            ( option::block_size                                        \
            , boost::program_options::value<option::block_size_validator>() \
              ->default_value (option::block_size_default)              \
            , option::block_size_description                            \
            )                                                           \
            ;                                                           \
                                                                        \
          boost::program_options::variables_map vm;                     \
          boost::program_options::store                                 \
            ( boost::program_options::command_line_parser (parameters)  \
              .options (options).run()                                  \
            , vm                                                        \
            );                                                          \
                                                                        \
          boost::program_options::notify (vm);                          \
                                                                        \
          boost::optional<option::block_size_type> const block_size     \
            ( vm.count (option::block_size)                             \
            ? boost::optional<option::block_size_type>                  \
                (vm.at (option::block_size).as<option::block_size_validator>()) \
            : boost::none                                               \
            )
        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_hostnames
            , block_size
            , [] (std::string const& hostname) { return hostname; }
            , [&] (std::string const& hostname)
              {
                //! \todo use torque's tm_spawn API directly to avoid system()
                util::system_with_blocked_SIGCHLD
                  (( boost::format
                     ( "pbsdsh -h %5% %1% %2% --register-host %3% --register-port %4%"
                     " --register-key %5%"
                     )
                   % binary
                   % (port ? "--port " + std::to_string (*port) : "")
                   % register_host
                   % register_port
                   % hostname
                   ).str()
                  );
              }
            ).second;
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
            ( std::unordered_map<std::string, fhg::rif::entry_point> const& all_entry_points
            , std::vector<std::string> const& parameters
            )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_entry_points
            , block_size
            , [] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                return entry_point.first;
              }
            , [&] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                //! \todo use torque's tm_spawn API directly to avoid system()
                util::system_with_blocked_SIGCHLD
                  (( boost::format ("pbsdsh -h %1% /bin/kill -TERM %2%")
                   % entry_point.second.hostname
                   % entry_point.second.pid
                   ).str()
                  );
              }
            );
        }
      }
    }
  }
}
