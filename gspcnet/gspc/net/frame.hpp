#ifndef GSPC_NET_FRAME_HPP
#define GSPC_NET_FRAME_HPP

#include <map>
#include <string>

namespace gspc
{
  namespace net
  {
    class frame
    {
    public:
      typedef std::string string_type;
      typedef std::map<string_type, string_type> header_type;

      frame () {}

      explicit
      frame (string_type const & command)
        : m_command (command)
      {}

      string_type const & command () const { return m_command; }
      frame & set_command (string_type const &cmd);

      frame & set_header (string_type const &key, string_type const &val);
      frame & del_header (string_type const &key);
      string_type const & get_header (string_type const &key) const;
      bool has_header (string_type const &key) const;
    private:
      string_type m_command;
      header_type m_header;
    };
  }
}

#endif
