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

    void frame::invalidate_cache ()
    {
      m_to_string_cache = boost::none;
      m_to_hex_cache = boost::none;
    }

    frame & frame::set_command (std::string const & cmd)
    {
      invalidate_cache ();

      m_command = cmd;
      return *this;
    }

    frame & frame::set_header (frame::header_type const &hdr)
    {
      invalidate_cache ();

      m_header = hdr;
      return *this;
    }

    frame & frame::set_header ( std::string const & key
                              , std::string const & val
                              )
    {
      invalidate_cache ();

      bool updated = false;
      header_type::iterator it = m_header.begin ();
      const header_type::iterator end = m_header.end ();
      while (it != end)
      {
        if (it->first == key)
        {
          it->second = val;
          updated = true;
        }

        ++it;
      }

      if (it == end && not updated)
      {
        m_header.push_back (header_type::value_type (key, val));
      }

      return *this;
    }

    frame & frame::set_header ( frame::key_type const & key
                              , frame::header_value const & val
                              )
    {
      if (val)
      {
        return set_header (key, *val);
      }
      else
      {
        return del_header (key);
      }
    }

    frame & frame::del_header (std::string const & key)
    {
      invalidate_cache ();

      header_type::iterator it = m_header.begin ();
      const header_type::iterator end = m_header.end ();
      while (it != end)
      {
        if (it->first == key)
        {
          it = m_header.erase (it);
        }
        else
        {
          ++it;
        }
      }

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
      frame::header_value val;

      header_type::const_reverse_iterator it = m_header.rbegin ();
      const header_type::const_reverse_iterator end = m_header.rend ();
      while (it != end)
      {
        if (it->first == key)
        {
          return it->second;
        }
        ++it;
      }

      return boost::none;
    }

    bool frame::has_header (std::string const & key) const
    {
      return get_header (key) != boost::none;
    }

    frame & frame::set_body (frame::body_type const & body)
    {
      invalidate_cache ();

      m_body = body;

      return update_content_length ();
    }

    frame & frame::add_body (std::string const &body)
    {
      invalidate_cache ();

      m_body.insert ( m_body.end ()
                    , body.begin ()
                    , body.end ()
                    );
      return update_content_length ();
    }

    frame & frame::add_body (const char *bytes, size_t len)
    {
      invalidate_cache ();

      m_body.insert ( m_body.end ()
                    , bytes
                    , bytes + len
                    );
      return update_content_length ();
    }

    static const char EOL = '\n';
    static const char NUL = '\0';

    std::string const & frame::to_string () const
    {
      if (not m_to_string_cache)
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
          os << get_body ();
          os << NUL;
        }

        m_to_string_cache = os.str ();
      }

      return *m_to_string_cache;
    }

    std::string const & frame::to_hex () const
    {
      if (not m_to_hex_cache)
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

        m_to_hex_cache = os.str ();
      }

      return *m_to_hex_cache;
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
