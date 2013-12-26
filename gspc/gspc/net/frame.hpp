#ifndef GSPC_NET_FRAME_HPP
#define GSPC_NET_FRAME_HPP

#include <list>
#include <vector>
#include <string>
#include <iosfwd>

#include <boost/optional.hpp>

#include <map>

namespace gspc
{
  namespace net
  {
    class frame
    {
    public:
      typedef std::string                        key_type;
      typedef std::string                        value_type;
      typedef std::map<key_type, value_type>     header_type;
      typedef std::string                        body_type;
      typedef boost::optional<value_type>        header_value;

      frame () {}

      explicit
      frame (std::string const & command)
        : m_command (command)
      {}

      frame (std::string const & command, const char *data, const size_t len);

      std::string const & get_command () const { return m_command; }
      frame & set_command (std::string const &cmd);

      frame & set_header (header_type const &);
      frame & set_header (key_type const &key, value_type const &val);
      frame & set_header (key_type const &key, header_value const &opt_val);

      frame & del_header (key_type const &key);

      header_type const & get_header () const;

      header_value get_header (key_type const &key) const;
      value_type get_header (key_type const &key, value_type const &def) const;

      bool has_header (std::string const &key) const;

      frame & set_body (body_type const & body);
      frame & add_body (body_type const & body);
      frame & add_body (const char *buf, std::size_t len);

      body_type const & get_body () const { return m_body; }

      std::string const to_string () const;
    private:
      frame & update_content_length ();

      std::string       m_command;
      header_type       m_header;
      body_type         m_body;
    };
  }
}

#endif
