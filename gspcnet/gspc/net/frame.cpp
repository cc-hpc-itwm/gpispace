#include "frame.hpp"

#include <sstream>
#include <iomanip>

#include <boost/none.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <gspc/net/frame_util.hpp>

namespace gspc
{
  namespace net
  {
    frame & frame::set_command (std::string const & cmd)
    {
      m_command = cmd;
      return *this;
    }

    frame & frame::set_header ( std::string const & key
                              , std::string const & val
                              )
    {
      m_header [key] = val;
      return *this;
    }

    frame & frame::del_header ( std::string const & key
                              )
    {
      m_header.erase (key);
      return *this;
    }

    frame::header_type const &
    frame::get_header () const
    {
      return m_header;
    }

    boost::optional<frame::value_type>
    frame::get_header (std::string const & key) const
    {
      header_type::const_iterator it = m_header.find (key);

      if (it != m_header.end ()) return it->second;
      else                       return boost::none;
    }

    bool frame::has_header (std::string const & key) const
    {
      return m_header.find (key) != m_header.end ();
    }

    frame & frame::set_body (std::string const &body)
    {
      m_body.assign (body.begin (), body.end ());

      return *this;
    }

    frame & frame::add_body (std::string const &body)
    {
      m_body.insert ( m_body.end ()
                    , body.begin ()
                    , body.end ()
                    );
      return *this;
    }

    frame & frame::add_body (const char *bytes, size_t len)
    {
      m_body.insert ( m_body.end ()
                    , bytes
                    , bytes + len
                    );
      return *this;
    }

    frame & frame::close ()
    {
      if (m_body.size ())
      {
        set_header ( "content-length"
                   , boost::lexical_cast<value_type>(m_body.size ())
                   );
      }
      return *this;
    }

    std::string frame::get_body_as_string () const
    {
      return std::string (m_body.begin (), m_body.end ());
    }

    static const char EOL = '\n';
    static const char NUL = '\0';

    std::string frame::to_string () const
    {
      std::ostringstream os;

      if (is_heartbeat (*this))
      {
        os << EOL;
      }
      else
      {
        os << get_command () << EOL;
        BOOST_FOREACH ( frame::header_type::value_type const & kvp
                      , get_header ()
                      )
        {
          os << kvp.first << ":" << kvp.second << EOL;
        }
        os << EOL;
        os << get_body_as_string ();
        os << NUL;
      }

      return os.str ();
    }

    std::string frame::to_hex () const
    {
      std::string frame_as_string = to_string ();
      std::ostringstream os;

      os << std::hex;
      BOOST_FOREACH (const char c, frame_as_string)
      {
        os << "\\x"
           << std::setfill ('0')
           << std::setw (2)
           << (int)(c & 0xff)
          ;
      }
      return os.str ();
    }
  }
}
