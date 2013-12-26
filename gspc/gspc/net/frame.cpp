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

    frame & frame::set_header (frame::header_type const &hdr)
    {
      m_header = hdr;
      return *this;
    }

    frame & frame::set_header ( std::string const & key
                              , std::string const & val
                              )
    {
      std::pair<header_type::iterator, bool> const
        iresult (m_header.insert (std::make_pair (key, val)));

      if (not iresult.second)
      {
        iresult.first->second = val;
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
      header_type::const_iterator const pos (m_header.find (key));

      if (pos == m_header.end())
      {
        return boost::none;
      }

      return pos->second;
    }

    frame::value_type frame::get_header ( key_type const &key
                                        , value_type const &def
                                        ) const
    {
      frame::header_value val = get_header (key);
      return val ? *val : def;
    }

    bool frame::has_header (std::string const & key) const
    {
      return get_header (key) != boost::none;
    }

    frame & frame::set_body (frame::body_type const & body)
    {
      m_body = body;

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

    static const char EOL = '\n';
    static const char NUL = '\0';

    std::string const frame::to_string () const
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

        return os.str();
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
