// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <boost/utility.hpp>

#include <ios>

namespace fhg
{
  namespace util
  {
    class save_stream_flags : boost::noncopyable
    {
    public:
      save_stream_flags (std::ios_base& stream)
        : _stream (stream)
        , _flags (_stream.flags())
      {}

      ~save_stream_flags()
      {
        _stream.flags (_flags);
      }

    private:
      std::ios_base& _stream;
      std::ios_base::fmtflags _flags;
    };
  }
}