#ifndef GSPC_NET_FRAME_HPP
#define GSPC_NET_FRAME_HPP

#include <list>
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
      typedef std::list<std::pair<std::string, value_type> >  header_type;
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

      /**
         Sets a header key to the given value.
       */
      frame & set_header (key_type const &key, value_type const &val);

      /**
         Sets a header key to the given value.
       */
      frame & set_header (key_type const &key, header_value const &opt_val);

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
         Get a header entry or the given default.
       */
      value_type get_header (key_type const &key, value_type const &def) const;

      /**
         Check if a header entry exists or not.
       */
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
