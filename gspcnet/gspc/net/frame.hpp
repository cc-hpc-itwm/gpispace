#ifndef GSPC_NET_FRAME_HPP
#define GSPC_NET_FRAME_HPP

#include <map>
#include <vector>
#include <string>
#include <iosfwd>

#include <boost/optional.hpp>

namespace gspc
{
  namespace net
  {
    class frame
    {
    public:
      typedef std::string                        key_type;
      typedef std::string                        value_type;
      typedef std::map<std::string, value_type>  header_type;
      typedef std::vector<char>                  body_type;
      typedef boost::optional<value_type>        header_value;

      frame () {}

      explicit
      frame (std::string const & command)
        : m_command (command)
      {}

      frame (std::string const & command, const char *data, const size_t len);

      std::string const & get_command () const { return m_command; }
      frame & set_command (std::string const &cmd);

      /**
         Sets a header key to the given value.
       */
      frame & set_header (key_type const &key, value_type const &val);

      /**
         Delete the given header entry.
       */
      frame & del_header (key_type const &key);

      /**
         Get the whole header
       */
      header_type const & get_header () const;

      /**
         Get a header entry. Returns  a boost::optional depending on whether the
         header entry was found or not.
       */
      header_value get_header (key_type const &key) const;

      /**
         Check if a header entry exists or not.
       */
      bool has_header (std::string const &key) const;

      frame & set_body (std::string const & body);
      frame & set_body (const char *buf, std::size_t len);
      frame & set_body (body_type const & body);
      frame & add_body (std::string const & body);
      frame & add_body (const char *buf, std::size_t len);

      body_type const & get_body () const { return m_body; }
      std::string get_body_as_string () const;

      std::string to_string () const;
      std::string to_hex () const;
    private:
      std::string       m_command;
      header_type       m_header;
      body_type         m_body;
    };
  }
}

#endif
