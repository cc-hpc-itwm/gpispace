#ifndef GSPC_NET_HEARTBEAT_INFO_HPP
#define GSPC_NET_HEARTBEAT_INFO_HPP

#include <string>
#include <iosfwd>
#include <gspc/net/frame_fwd.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace gspc
{
  namespace net
  {
    class heartbeat_info_t
    {
    public:
      heartbeat_info_t ();

      explicit
      heartbeat_info_t (frame const &f);

      explicit
      heartbeat_info_t (std::string const &heartbeat_header);

      boost::optional<boost::posix_time::time_duration> recv_duration () const;
      boost::optional<boost::posix_time::time_duration> send_duration () const;

      heartbeat_info_t & recv_duration (boost::optional<boost::posix_time::time_duration> const &);
      heartbeat_info_t & send_duration (boost::optional<boost::posix_time::time_duration> const &);

      heartbeat_info_t opposite () const;

      std::string str () const;
      void apply (frame & f) const;

      void set_from_string (std::string const &);
      void set_from_stream (std::istream &);
    private:
      boost::optional<boost::posix_time::time_duration> m_recv_duration;
      boost::optional<boost::posix_time::time_duration> m_send_duration;
    };

    std::ostream & operator << (std::ostream &, heartbeat_info_t const &);
    std::istream & operator >> (std::istream &, heartbeat_info_t &);
  }
}

#endif
