#include <rif/started_process_promise.hpp>

#include <util-generic/serialization/exception.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace fhg
{
  namespace rif
  {
    void started_process_promise::set_result
      (std::vector<std::string> messages)
    {
      send (true, messages);
    }
    void started_process_promise::set_exception
      (std::exception_ptr exception)
    {
      send ( false
           , fhg::util::serialization::exception::serialize
               ( exception
               , fhg::util::serialization::exception::serialization_functions()
               , fhg::util::serialization::exception::aggregated_serialization_functions()
               )
           );
    }

    //! \todo This is horrible. Better hard code a specific file
    //! descriptor and ensure that one is used?
    started_process_promise::started_process_promise
        (int& argc, char**& argv)
      : _startup_pipe_fd
          ( argc >= 2
          ? std::stoi (argv[1])
          : throw std::runtime_error
              ("command line requires at least 'exe pipefd'")
          )
    {
      ++argv;
      --argc;
    }

    template<typename T>
      void started_process_promise::send (bool result, T const& data)
    {
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
        stream (_startup_pipe_fd, boost::iostreams::close_handle);

      {
        boost::archive::text_oarchive archive (stream);
        archive & result;
        archive & data;
      }

      stream << end_sentinel_value();
    }
  }
}
