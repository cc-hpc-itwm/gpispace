#ifndef FHG_COM_PEER_HPP
#define FHG_COM_PEER_HPP 1

#include <string>

namespace fhg
{
  namespace com
  {
    struct host_t
    {
      explicit
      host_t (std::string const & h)
        : value (h)
      {}

      operator std::string () const
      {
        return value;
      }
    private:
      std::string value;
    };

    struct port_t
    {
      explicit
      port_t (std::string const & p)
        : value (p)
      {}

      operator std::string () const
      {
        return value;
      }
    private:
      std::string value;
    };

    /*!
      This class abstracts from an endpoint
     */
    class peer_t
    {
    public:
      peer_t ()
        : name_()
        , host_()
        , port_()
      {}

      peer_t ( std::string const & name
             , host_t const & host
             , port_t const & port
             )
        : name_(name)
        , host_(host)
        , port_(port)
      {}

      std::string const & name () const { return name_; }
      std::string const & host () const { return host_; }
      std::string const & port () const { return port_; }

      void start () {}
      void stop () {}

      void send (const char * data, const std::size_t n, int timeout);
    private:
      std::string name_;
      std::string host_;
      std::string port_;
    };

    inline
    std::ostream & operator << (std::ostream & os, peer_t const & p)
    {
      return os << p.name() << "@"
                << "[" << p.host() << "]:" << p.port()
                << std::endl;
    }

    inline
    std::istream & operator >> (std::istream & is, peer_t & p)
    {
      std::string s;
      is >> s;
      std::string name = s.substr(0, s.find("@["));
      std::string host (s.begin()+s.find("@[")+2, s.begin()+s.find("]:"));
      std::string port = s.substr(s.find("]:")+2);
      p = peer_t (name, host_t(host), port_t(port));
      return is;
    }
  }
}

#endif
