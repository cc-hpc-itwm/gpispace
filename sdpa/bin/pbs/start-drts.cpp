// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/start_drts_kernel.hpp>

#include <fhg/util/boost/optional.hpp>
#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/hostname.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <stdexcept>
#include <thread>

namespace
{
  template<typename T>
    T const& get ( boost::program_options::variables_map const& vm
                 , const char* option
                 )
  {
    return vm.at (option).as<T>();
  }

  template<typename T>
    boost::optional<T const&> get_or_none
      ( boost::program_options::variables_map const& vm
      , const char* option
      )
  {
    return vm.count (option)
      ? boost::optional<T const&> (vm.at (option).as<T>())
      : boost::optional<T const&>();
  }
}

int main (int argc, char** argv)
{
  try
  {
    boost::program_options::options_description options;
    options.add_options()
      ("help,h", "print this help")
      ( "verbose,v"
      , boost::program_options::value<bool>()
        ->implicit_value (true)->default_value (false)
      , "verbose"
      )
      ( "identity,i"
      , boost::program_options::value
          <fhg::util::boost::program_options::positive_integral<unsigned long>>()
      , "identity"
      )
      ( "master,m"
      , boost::program_options::value<std::string>()->required()
      , "master to connect to"
      )
      ( "name-prefix,n"
      , boost::program_options::value<std::string>()->required()
      , "name to prefix before hostname and identity"
      )
      ( "gui,g"
      , boost::program_options::value<std::string>()->required()
      , "ip:port, gui address"
      )
      ( "log,l"
      , boost::program_options::value<std::string>()
      , "ip:port, log address"
      )
      ( "gpi-socket,S"
      , boost::program_options::value
          <fhg::util::boost::program_options::existing_path>()
      , "gpi socket path"
      )
      ( "lib_path,L"
      , boost::program_options::value<std::vector<boost::filesystem::path>>()
        ->multitoken()->default_value (std::vector<boost::filesystem::path>(), "{}")
      , "library paths"
      )
      ( "count,c"
      , boost::program_options::value<unsigned long>()
      , "start this many processes (0==#cores)"
      )
      ( "capability,C"
      , boost::program_options::value<std::vector<std::string>>()
        ->multitoken()->default_value (std::vector<std::string>(), "{}")
      , "start with these capabilities"
      )
      ( "state,s"
      , boost::program_options::value
          <fhg::util::boost::program_options::existing_directory>()->required()
      , "state directory"
      )
      ( "shared-memory-size,M"
      , boost::program_options::value<unsigned long>()
      , "shared memory size, if not specified, GPI is disabled"
      )
      ( "numa-socket,N"
      , boost::program_options::value<unsigned long>()
      , "numa socket"
      )
      ( "gspc-home,H"
      , boost::program_options::value
          <fhg::util::boost::program_options::existing_directory>()->required()
      , "sdpa-home"
      )
      ;

    boost::program_options::variables_map vm;
    boost::program_options::store
      ( boost::program_options::command_line_parser (argc, argv)
        .options (options).run()
      , vm
      );

    if (vm.count ("help"))
    {
      std::cout << "usage: " << argv[0] << " [options]\n" << options << "\n";
      return 0;
    }

    boost::program_options::notify (vm);

    bool const verbose (get<bool> (vm, "verbose"));
    std::string const gui (get<std::string> (vm, "gui"));
    boost::optional<std::string> const log_url
      (get_or_none<std::string> (vm, "log"));
    std::string const master (get<std::string > (vm, "master"));
    boost::optional<unsigned long> const identity
      ( get_or_none
          <fhg::util::boost::program_options::positive_integral<unsigned long>>
            (vm, "identity")
      );
    boost::optional<boost::filesystem::path> const gpi_socket
      ( get_or_none<fhg::util::boost::program_options::existing_path>
          (vm, "gpi-socket")
      );
    std::vector<std::string> const capabilities
      (get<std::vector<std::string>> (vm, "capability"));
    std::vector<boost::filesystem::path> const lib_path
      (get<std::vector<boost::filesystem::path>> (vm, "lib_path"));
    boost::filesystem::path const state_dir
      (get<fhg::util::boost::program_options::existing_directory> (vm, "state"));
    std::string const name_prefix (get<std::string> (vm, "name-prefix"));
    boost::optional<unsigned long> const shm_size
      (get_or_none<unsigned long> (vm, "shared-memory-size"));
    boost::optional<unsigned long> const numa_socket
      (get_or_none<unsigned long> (vm, "numa-socket"));
    boost::filesystem::path const sdpa_home
      ( get<fhg::util::boost::program_options::existing_directory>
          (vm, "gspc-home")
      );
    boost::optional<unsigned long> count
      (get_or_none<unsigned long> (vm, "count"));

    boost::optional<std::pair<boost::filesystem::path, unsigned long>> const gpi
      ( shm_size
      ? std::make_pair ( fhg::util::boost::get_or_throw<std::invalid_argument>
                           (gpi_socket, "GPI socket is not set")
                       , shm_size.get()
                       )
      : boost::optional<std::pair<boost::filesystem::path, unsigned long>>()
      );

    // quick hack to easily start multiple instances
    if (identity == 0)
    {
      unsigned long actual_count
        ( fhg::util::boost::get_or_throw<std::invalid_argument>
            (count, "multiple-instance startup requires count") != 0
        ? count.get()
        : std::thread::hardware_concurrency()
        );

      std::cerr << "I: starting worker: " << name_prefix
                << " x" << actual_count << " on host " << fhg::util::hostname()
                << " with parent " << master << "\n";

      int fail_count (0);
      //! \todo do in parallel
      for (unsigned long i (1); i <= actual_count; ++i)
      {
        try
        {
          fhg::drts::worker::start ( verbose
                                   , gui
                                   , log_url
                                   , master
                                   , i
                                   , gpi
                                   , capabilities
                                   , lib_path
                                   , state_dir
                                   , name_prefix
                                   , numa_socket
                                   , sdpa_home
                                   );
        }
        catch (std::runtime_error const& ex)
        {
          std::cerr << "E: " << ex.what() << std::endl;
          ++fail_count;
        }
      }
      if (fail_count)
      {
        throw std::runtime_error
          (std::to_string (fail_count) + " startups failed");
      }
    }
    else
    {
      std::cerr << "I: starting worker: " << name_prefix
                << " on host " << fhg::util::hostname()
                << " with parent " << master << "\n";

      fhg::drts::worker::start
        ( verbose
        , gui
        , log_url
        , master
        , fhg::util::boost::get_or_throw<std::invalid_argument>
            (identity, "single-instance startup requires indentity")
        , gpi
        , capabilities
        , lib_path
        , state_dir
        , name_prefix
        , numa_socket
        , sdpa_home
        );
    }
  }
  catch (std::runtime_error const& ex)
  {
    std::cerr << "E: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
