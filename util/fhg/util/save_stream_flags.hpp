// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_SAVE_STREAM_FLAGS_HPP
#define FHG_UTIL_SAVE_STREAM_FLADGS_HPP

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

#endif
