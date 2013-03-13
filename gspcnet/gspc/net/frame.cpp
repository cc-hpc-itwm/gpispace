#include "frame.hpp"

namespace gspc
{
  namespace net
  {
    frame & frame::set_command (std::string const & cmd)
    {
      m_command = cmd;
      return *this;
    }

    frame & frame::set_header ( string_type const & key
                              , string_type const & val
                              )
    {
      m_header [key] = val;
      return *this;
    }

    frame & frame::del_header (string_type const & key)
    {
      m_header.erase (key);
      return *this;
    }

    frame::string_type const &
    frame::get_header (string_type const & key) const
    {
      static const std::string dflt ("");

      header_type::const_iterator it = m_header.find (key);

      if (it != m_header.end ()) return it->second;
      else                       return dflt;
    }

    bool frame::has_header (string_type const & key) const
    {
      return m_header.find (key) != m_header.end ();
    }
  }
}
