#include "heartbeat_info.hpp"

#include <stdexcept>
#include <sstream>
#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    heartbeat_info_t::heartbeat_info_t ()
      : m_recv_interval (0)
      , m_send_interval (0)
    {}

    heartbeat_info_t::heartbeat_info_t (frame const &f)
      : m_recv_interval (0)
      , m_send_interval (0)
    {
      set_from_string (f.get_header ("heart-beat", "0,0"));
    }

    heartbeat_info_t::heartbeat_info_t (std::string const &s)
      : m_recv_interval (0)
      , m_send_interval (0)
    {
      set_from_string (s);
    }

    std::string heartbeat_info_t::str () const
    {
      std::ostringstream os;
      os << m_recv_interval << "," << m_send_interval;
      return os.str ();
    }

    void heartbeat_info_t::apply (frame & f) const
    {
      f.set_header ("heart-beat", str ());
    }

    std::size_t heartbeat_info_t::recv_interval () const
    {
      return m_recv_interval;
    }

    std::size_t heartbeat_info_t::send_interval () const
    {
      return m_send_interval;
    }

    boost::optional<boost::posix_time::time_duration>
    heartbeat_info_t::recv_duration () const
    {
      if (0 == recv_interval ())
      {
        return boost::none;
      }
      else
      {
        return boost::posix_time::seconds (recv_interval ());
      }
    }

    boost::optional<boost::posix_time::time_duration>
    heartbeat_info_t::send_duration () const
    {
      if (0 == send_interval ())
      {
        return boost::none;
      }
      else
      {
        return boost::posix_time::seconds (send_interval ());
      }
    }

    void heartbeat_info_t::set_from_string (std::string const &heart_beat)
    {
      int c;
      int i;

      std::istringstream sstr (heart_beat);
      sstr >> i;
      if (sstr.fail () || i < 0)
      {
        throw std::invalid_argument
          ("expected uint as 'recv' interval in: " + heart_beat);
      }
      m_recv_interval = static_cast<std::size_t>(i);

      c = sstr.get ();
      if (c != ',')
      {
        throw std::invalid_argument
          ("heart-beat in invalid format, expected ',': " + heart_beat);
      }

      sstr >> i;
      if (sstr.fail () || i < 0)
      {
        throw std::invalid_argument
          ("expected uint as 'send' interval in: " + heart_beat);
      }
      m_send_interval = static_cast<std::size_t>(i);

      c = sstr.get ();
      if (c != -1)
      {
        throw std::invalid_argument
          ("unexpected trailing character(s) at the end of heart-beat" + heart_beat);
      }
    }
  }
}
