#ifndef GSPC_NET_HEARTBEAT_INFO_HPP
#define GSPC_NET_HEARTBEAT_INFO_HPP

#include <string>
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

      std::size_t recv_interval () const;
      std::size_t send_interval () const;

      std::string str () const;
      void apply (frame & f) const;
    private:
      void set_from_string (std::string const &);

      std::size_t m_recv_interval;
      std::size_t m_send_interval;
    };
  }
}

#endif
