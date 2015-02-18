// bernd.loerwald@itwm.fraunhofer.de

#include <rif/startup_messages_pipe.hpp>

#include <stdexcept>
#include <string>

namespace fhg
{
  namespace rif
  {
    namespace
    {
      int get_fd (boost::program_options::variables_map const& vm)
      {
        if (!vm.count (startup_messages_pipe::option_name()))
        {
          throw std::logic_error ( std::string ("OPTION MISSING: ")
                                 + startup_messages_pipe::option_name()
                                 );
        }
        return vm.at (startup_messages_pipe::option_name()).as<int>();
      }
    }

    startup_messages_pipe::startup_messages_pipe
        (boost::program_options::variables_map const& vm)
      : _stream (get_fd (vm), boost::iostreams::close_handle)
    {}
    startup_messages_pipe::~startup_messages_pipe()
    {
      *this << end_sentinel_value();
    }

    boost::program_options::options_description
      startup_messages_pipe::program_options()
    {
      boost::program_options::options_description options ("Startup");
      options.add_options()
        ( option_name()
        , boost::program_options::value<int>()->required()
        , "pipe filedescriptor to use for communication during startup (ports used, ...)"
        );
      return options;
    }
  }
}
