#include "heartbeat_info.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    heartbeat_info_t::heartbeat_info_t ()
      : m_recv_duration (boost::none)
      , m_send_duration (boost::none)
    {}

    heartbeat_info_t::heartbeat_info_t (frame const &f)
      : m_recv_duration (boost::none)
      , m_send_duration (boost::none)
    {
      set_from_string (f.get_header ("heart-beat", "0,0"));
    }

    heartbeat_info_t::heartbeat_info_t (std::string const &s)
      : m_recv_duration (boost::none)
      , m_send_duration (boost::none)
    {
      set_from_string (s);
    }

    std::string heartbeat_info_t::str () const
    {
      std::ostringstream os;
      if (m_recv_duration)
      {
        os << m_recv_duration->total_seconds ();
      }
      else
      {
        os << "0";
      }

      os << ",";

      if (m_send_duration)
      {
        os << m_send_duration->total_seconds ();
      }
      else
      {
        os << "0";
      }

      return os.str ();
    }

    void heartbeat_info_t::apply (frame & f) const
    {
      f.set_header ("heart-beat", str ());
    }

    heartbeat_info_t & heartbeat_info_t::recv_duration (boost::optional<boost::posix_time::time_duration> const &d)
    {
      m_recv_duration = d;
      return *this;
    }

    heartbeat_info_t & heartbeat_info_t::send_duration (boost::optional<boost::posix_time::time_duration> const &d)
    {
      m_send_duration = d;
      return *this;
    }

    boost::optional<boost::posix_time::time_duration>
    heartbeat_info_t::recv_duration () const
    {
      return m_recv_duration;
    }

    boost::optional<boost::posix_time::time_duration>
    heartbeat_info_t::send_duration () const
    {
      return m_send_duration;
    }

    heartbeat_info_t heartbeat_info_t::opposite () const
    {
      heartbeat_info_t o;
      o.send_duration (recv_duration ());
      o.recv_duration (send_duration ());
      return o;
    }

    void heartbeat_info_t::set_from_string (std::string const &heart_beat)
    {
      std::istringstream sstr (heart_beat);
      set_from_stream (sstr);

      int c = sstr.get ();
      if (c != -1)
      {
        throw std::invalid_argument
          ("unexpected trailing character(s) at the end of heart-beat" + heart_beat);
      }
    }

    void heartbeat_info_t::set_from_stream (std::istream &is)
    {
      int c;
      int i;

      is >> i;
      if (is.fail () || i < 0)
      {
        throw std::invalid_argument
          ("expected uint as 'recv' interval");
      }
      if (0 == i)
      {
        m_recv_duration = boost::none;
      }
      else
      {
        m_recv_duration = boost::posix_time::seconds (i);
      }

      c = is.get ();
      if (c != ',')
      {
        throw std::invalid_argument
          ("heart-beat in invalid format, expected ','");
      }

      is >> i;
      if (is.fail () || i < 0)
      {
        throw std::invalid_argument
          ("expected uint as 'send' interval");
      }
      if (0 == i)
      {
        m_send_duration = boost::none;
      }
      else
      {
        m_send_duration = boost::posix_time::seconds (i);
      }
    }

    std::ostream & operator << (std::ostream &os, heartbeat_info_t const &hb)
    {
      os << hb.str ();
      return os;
    }

    std::istream & operator >> (std::istream &is, heartbeat_info_t &hb)
    {
      hb.set_from_stream (is);
      return is;
    }
  }
}
