#include <map/interface.hpp>
#include <map/test/transform_file/type.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>

void map_produce ( map::user_data_type const& user_data
                 , map::memory_buffer_type buffer
                 , unsigned long id
                 , fhg::log::Logger& logger
                 )
{
  transform_file::parameter const parameter
  {transform_file::from_bytearray (user_data)};
  std::string const input {parameter.input().string()};
  unsigned long const size {parameter.size()};

  LLOG (INFO, logger, "read " << input << "[" << id << "]");

  std::ifstream stream (input, std::ifstream::binary);

  if (!stream)
  {
    throw std::runtime_error
      ((boost::format ("Could not open '%1%' for reading") % input).str());
  }

  if (!stream.seekg (id * buffer.second, stream.beg))
  {
    throw std::runtime_error
      (( boost::format ("Could not seek to position '%2%' of '%1%'")
       % input
       % (id * buffer.second)
       ).str()
      );
  };

  if (!stream.read ( static_cast<char*> (buffer.first)
                   , std::min (buffer.second, size - id * buffer.second)
                   )
     )
  {
    throw std::runtime_error
      ((boost::format ("Could not read from '%1%'") % input).str());
  };
}

void map_process
  ( map::user_data_type const&
  , map::const_memory_buffer_type input
  , map::memory_buffer_type output
  , fhg::log::Logger& logger
  )
{
  LLOG (INFO, logger, "transform");

  if (input.second != output.second)
  {
    throw std::logic_error
      ( ( boost::format ("input/output buffer sizes differ: %1% != %2%")
        % input.second
        % output.second
        ).str()
      );
  }

  char const* const mem_input (static_cast<char const* const> (input.first));
  char* const mem_output (static_cast<char*> (output.first));

  std::transform ( mem_input, mem_input + input.second
                 , mem_output
                 , [](char c) { return std::tolower (c); }
                 );
}

void map_consume ( map::user_data_type const& user_data
                 , map::const_memory_buffer_type buffer
                 , unsigned long id
                 , fhg::log::Logger& logger
                 )
{
  transform_file::parameter const parameter
  {transform_file::from_bytearray (user_data)};
  std::string const output {parameter.output().string()};
  unsigned long const size {parameter.size()};

  LLOG (INFO, logger, "write " << output << "[" << id << "]");

  std::fstream stream
    (output, std::fstream::binary | std::fstream::out | std::fstream::in);

  if (!stream)
  {
    throw std::runtime_error
      ((boost::format ("Could not open '%1%' for writing") % output).str());
  }

  if (!stream.seekp (id * buffer.second, stream.beg))
  {
    throw std::runtime_error
      (( boost::format ("Could not seek to position '%2%' of '%1%'")
       % output
       % (id * buffer.second)
       ).str()
      );
  };

  if (!stream.write ( static_cast<char const*> (buffer.first)
                    , std::min (buffer.second, size - id * buffer.second)
                    )
     )
  {
    throw std::runtime_error
      ((boost::format ("Could not write to '%1%'") % output).str());
  };
}
