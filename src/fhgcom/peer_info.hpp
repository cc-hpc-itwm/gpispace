#ifndef FHG_COM_PEER_INFO_HPP
#define FHG_COM_PEER_INFO_HPP 1

namespace fhg
{
  namespace com
  {
    struct hard_string
    {
      explicit hard_string (std::string const& s)
        : _s (s)
      {}
      operator std::string () const
      {
        return _s;
      }
    private:
      std::string const _s;
    };

    struct host_t : hard_string
    {
      explicit host_t (std::string const& s)
        : hard_string (s)
      {}
    };
    struct port_t : hard_string
    {
      explicit port_t (std::string const& s)
        : hard_string (s)
      {}
    };

    class peer_info_t
    {
    public:
      peer_info_t ( const std::string & n
                  , const host_t & host
                  , const port_t & port
                  )
        : name_(n)
        , host_(host)
        , port_(port)
      {}

      peer_info_t (std::string const& s)
        : name_ ()
        , host_ ()
        , port_ ()
      {
        parse (s);
      }

      std::string const & host (const std::string & def) const
      {
        if (host_.empty()) return def;
        else return host_;
      }

      std::string const & port (const std::string & def) const
      {
        if (port_.empty()) return def;
        else return port_;
      }

      std::string const to_string () const
      {
        std::string s;

        if (! name_.empty())
        {
          s += name_ + "@";
        }

        if (host_.find (":") != std::string::npos)
        {
          s += "[" + host_ + "]";
        }
        else
        {
          s += host_;
        }

        if (! port_.empty())
        {
          s += ":" + port_;
        }

        return s;
      }

    private:
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
      }

      std::string name_;
      std::string host_;
      std::string port_;
    };
  }
}

#endif
