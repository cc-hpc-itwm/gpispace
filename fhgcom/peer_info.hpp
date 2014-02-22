#ifndef FHG_COM_PEER_INFO_HPP
#define FHG_COM_PEER_INFO_HPP 1

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

    class peer_info_t
    {
    public:
      peer_info_t ()
        : name_()
        , host_()
        , port_()
        , to_string_cache_()
      {}

      peer_info_t ( const std::string & n
                  , const host_t & host
                  , const port_t & port
                  )
        : name_(n)
        , host_(host)
        , port_(port)
        , to_string_cache_()
      {
        update_to_string_cache();
      }

      std::string const & name () const
      {
        return name_;
      }
      std::string const & name (const std::string & def) const
      {
        if (name_.empty()) return def;
        else return name();
      }

      std::string const & host () const
      {
        return host_;
      }
      std::string const & host (const std::string & def) const
      {
        if (host_.empty()) return def;
        else return host();
      }

      std::string const & port () const
      {
        return port_;
      }
      std::string const & port (const std::string & def) const
      {
        if (port_.empty()) return def;
        else return port();
      }

      std::string const & to_string () const
      {
        if (to_string_cache_.empty())
        {
          update_to_string_cache();
        }
        return to_string_cache_;
      }

      static peer_info_t from_string (const std::string & s)
      {
        peer_info_t pi;
        pi.parse (s);
        return pi;
      }

      void parse (const std::string & s)
      {
        std::string::size_type b_pos, e_pos;

        b_pos = 0;
        e_pos = s.find ('@', b_pos);
        if (e_pos != std::string::npos)
        {
          name_ = s.substr(b_pos, e_pos - b_pos);
          b_pos = e_pos + 1;
        }
        else
        {
          name_ = "";
        }

        e_pos = s.find ('[', b_pos);
        if (e_pos != std::string::npos)
        {
          b_pos = e_pos + 1;
          e_pos = s.find (']', b_pos);
          if (e_pos == std::string::npos)
            throw std::runtime_error ("peer_info: could not parse: " + s);
          host_ = s.substr (b_pos, e_pos - b_pos);
          b_pos = e_pos;
        }
        else
        {
          e_pos = s.find (':', b_pos);
          if (e_pos != std::string::npos)
          {
            host_ = s.substr (b_pos, e_pos - b_pos);
            b_pos = e_pos;
          }
          else
          {
            host_ = s.substr (b_pos);
            b_pos = std::string::npos;
          }
        }

        b_pos = s.find (':', b_pos);
        if (b_pos != std::string::npos)
        {
          port_ = s.substr (b_pos+1);
        }
        else
        {
          throw std::runtime_error ("peer_info: parse error: port is missing!");
        }
        update_to_string_cache();
      }
    private:
      void update_to_string_cache() const
      {
        if (! name_.empty())
        {
          to_string_cache_ = name_ + "@";
        }

        if (host_.find (":") != std::string::npos)
        {
          to_string_cache_ += "[" + host_ + "]";
        }
        else
        {
          to_string_cache_ += host_;
        }

        if (! port_.empty())
        {
          to_string_cache_ += ":" + port_;
        }
      }

      std::string name_;
      std::string host_;
      std::string port_;
      mutable std::string to_string_cache_;
    };

    inline
    std::ostream & operator << (std::ostream & os, peer_info_t const & p)
    {
      return os << p.to_string ();
    }

    inline
    std::istream & operator >> (std::istream & is, peer_info_t & p)
    {
      std::string s;
      is >> s;
      p.parse (s);
      return is;
    }
  }
}

#endif
