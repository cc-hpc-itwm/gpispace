// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace fhg
{
  namespace rif
  {
    struct startup_messages_pipe
    {
      //! \note requires program_options() to be added to options_description
      startup_messages_pipe (boost::program_options::variables_map const& vm);
      ~startup_messages_pipe();

      static boost::program_options::options_description program_options();
      static constexpr const char* option_name() { return "startup-messages-pipe"; }
      static constexpr const char* end_sentinel_value() { return "DONE"; }

    private:
      template<typename T> friend
        startup_messages_pipe& operator<< (startup_messages_pipe&, T const&);
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink> _stream;
    };

    //! \note Shall not be called with val == end_sentinel_value
    template<typename T>
      startup_messages_pipe& operator<< (startup_messages_pipe& pipe, T const& val)
    {
      pipe._stream << val << '\n';
      return pipe;
    }
  }
}
