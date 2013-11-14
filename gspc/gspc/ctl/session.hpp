#ifndef GSPC_CTL_SESSION_HPP
#define GSPC_CTL_SESSION_HPP

#include <list>
#include <gspc/ctl/session_info.hpp>
#include <boost/filesystem.hpp>

namespace gspc
{
  namespace kvs
  {
    class service_t;
  }

  namespace ctl
  {
    typedef std::list<session_info_t> session_info_list_t;

    class session_t
    {
    public:
      enum flags_t
        {
          SESSION_ACTIVE = 0x001
        , SESSION_ALL    = 0x002
        };

      session_t ();

      static int info (boost::filesystem::path const &socket, session_info_t &);
      static int list (boost::filesystem::path const &dir, session_info_list_t &);
      static int list (boost::filesystem::path const &dir, session_info_list_t &, int flags);

      int list (session_info_list_t &) const;
      int list (session_info_list_t &, int flags) const;

      int set_session_dir (boost::filesystem::path const &p);
      int get_session_dir (boost::filesystem::path &p) const;

      int set_session_name (std::string const &name);
      int get_session_name (std::string &name) const;

      int set_bind_url (std::string const &url);
      int get_bind_url (std::string & url) const;

      int run (session_info_t &) const;
      int daemonize_then_run (session_info_t &) const;
    private:
      boost::filesystem::path m_dir;
      std::string             m_name;
      std::string             m_url;
    };

    int start_session (std::string const &puburl);
  }
}

#endif
