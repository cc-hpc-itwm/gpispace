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
    frame::frame ( std::string const & command
                 , const char *data
                 , const size_t len
                 )
      : m_command (command)
      , m_header ()
      , m_body (data, data + len)
    {
      update_content_length ();
    }

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

    frame::header_value
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

      return update_content_length ();
    }

    frame & frame::add_body (std::string const &body)
    {
      m_body.insert ( m_body.end ()
                    , body.begin ()
                    , body.end ()
                    );
      return update_content_length ();
    }

    frame & frame::add_body (const char *bytes, size_t len)
    {
      m_body.insert ( m_body.end ()
                    , bytes
                    , bytes + len
                    );
      return update_content_length ();
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

    frame & frame::update_content_length ()
    {
      if (not m_body.empty ())
      {
        return set_header ( "content-length"
                          , boost::lexical_cast<value_type>(m_body.size ())
                          );
      }
      else
      {
        return del_header ("content-length");
      }
    }
  }
}
