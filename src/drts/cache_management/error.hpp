#pragma once

#include <boost/format.hpp>

#include <string>

namespace drts
{
  namespace cache
  {
    namespace error
    {
      class generic : public std::runtime_error
      {
      public:
        generic (const std::string & msg)
          : std::runtime_error ("ERROR: " + msg)
        { }

        generic (const boost::format& bf)
          : generic (bf.str())
        { }
      };


      class buffer_larger_than_cache_size : public generic
      {
      public:
        buffer_larger_than_cache_size ( unsigned long, unsigned long);
        virtual ~buffer_larger_than_cache_size() throw() = default;
      };

      class insufficient_space_available_in_cache : public generic
      {
      public:
        insufficient_space_available_in_cache ( unsigned long, unsigned long, unsigned long, unsigned long);
        virtual ~insufficient_space_available_in_cache() throw() = default;
      };

      class buffer_not_in_cache : public generic
      {
      public:
        buffer_not_in_cache(std::string dataid);
        virtual ~buffer_not_in_cache() throw() = default;
      };
    }
  }
}
