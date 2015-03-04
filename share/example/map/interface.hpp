#pragma once

#include <we/type/bytearray.hpp>

#include <fhglog/level.hpp>

#include <functional>
#include <utility>

namespace map
{
  typedef we::type::bytearray user_data_type;
  typedef unsigned long size_in_bytes_type;
  typedef std::pair<void*, size_in_bytes_type> memory_buffer_type;
  typedef std::pair<void const*, size_in_bytes_type> const_memory_buffer_type;
  typedef std::function<void ( fhg::log::Level const& severity
                             , std::string const& file
                             , std::string const& function
                             , std::size_t const& line
                             , std::string const& message
                             )> logger_type;
}

extern "C"
{
  void map_produce ( map::user_data_type const&
                   , map::memory_buffer_type
                   , unsigned long id
                   , map::logger_type
                   );

  void map_process ( map::user_data_type const&
                   , map::const_memory_buffer_type
                   , map::memory_buffer_type
                   , map::logger_type
                   );

  void map_consume ( map::user_data_type const&
                   , map::const_memory_buffer_type
                   , unsigned long id
                   , map::logger_type
                   );
}

#include <boost/current_function.hpp>
#include <sstream>

#define MAP_LOG(_message)                       \
  do                                            \
  {                                             \
    std::ostringstream message;                 \
    message << _message;                        \
    logger ( fhg::log::INFO                     \
           , __FILE__                           \
           , BOOST_CURRENT_FUNCTION             \
           , __LINE__                           \
           , message.str()                      \
           );                                   \
  } while (0)
