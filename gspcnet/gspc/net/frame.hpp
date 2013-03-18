#ifndef GSPC_NET_FRAME_HPP
#define GSPC_NET_FRAME_HPP

#include <map>
#include <vector>
#include <string>
#include <iosfwd>

namespace gspc
{
  namespace net
  {
    class frame
    {
    public:
      typedef std::map<std::string, std::string> header_type;
      typedef std::vector<char>                  body_type;

      frame () {}

      explicit
      frame (std::string const & command)
        : m_command (command)
      {}

      std::string const & get_command () const { return m_command; }
      frame & set_command (std::string const &cmd);

      frame & set_header (std::string const &key, std::string const &val);
      frame & del_header (std::string const &key);
      header_type const & get_header () const;
      std::string const & get_header (std::string const &key) const;
      bool has_header (std::string const &key) const;

      frame & set_body (std::string const & body);
      frame & set_body (const char *buf, std::size_t len);
      frame & set_body (body_type const & body);

      body_type const & get_body () const { return m_body; }
      body_type & get_body () { return m_body; }
      std::string get_body_as_string () const;

      std::string to_string () const;
    private:
      std::string       m_command;
      header_type       m_header;
      body_type         m_body;
    };

    namespace make
    {
      frame error_frame (int ec, const char *message);
    }
  }
}

#endif
