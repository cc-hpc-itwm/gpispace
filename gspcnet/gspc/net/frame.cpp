#include "frame.hpp"

#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

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

    std::string const &
    frame::get_header (std::string const & key) const
    {
      static const std::string dflt ("");

      header_type::const_iterator it = m_header.find (key);

      if (it != m_header.end ()) return it->second;
      else                       return dflt;
    }

    bool frame::has_header (std::string const & key) const
    {
      return m_header.find (key) != m_header.end ();
    }

    frame & frame::set_body (std::string const &body)
    {
      m_body.assign (body.begin (), body.end ());
      /*
      set_header ( "content-length"
                 , boost::lexical_cast<std::string>(m_body.size ())
                 );
      */

      return *this;
    }

    std::string frame::get_body_as_string () const
    {
      return std::string (m_body.begin (), m_body.end ());
    }

    std::string frame::to_string () const
    {
      std::ostringstream os;

      os << get_command () << std::endl;
      BOOST_FOREACH ( frame::header_type::value_type const & kvp
                    , get_header ()
                    )
      {
        os << kvp.first << ":" << kvp.second << std::endl;
      }
      os << std::endl;
      os << get_body_as_string ();
      os << '\0';

      return os.str ();
    }

    namespace make
    {
      frame error_frame ( int ec
                        , const char *message
                        )
      {
        frame f;
        f.set_command ("ERROR");
        f.set_header ( "error-code"
                     , boost::lexical_cast<std::string>(ec)
                     );
        f.set_header ("error-message"
                     , message
                     );
        return f;
      }
    }
  }
}
