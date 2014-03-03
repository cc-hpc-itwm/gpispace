#include <gpi-space/pc/url.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/parse/until.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace gpi
{
  namespace pc
  {
    namespace
    {
      bool is_end_of_value (fhg::util::parse::position const& pos)
      {
        return ! isalnum (*pos);
      }

      std::string require_value (fhg::util::parse::position& pos)
      {
        if (pos.end() || is_end_of_value (pos))
        {
          throw fhg::util::parse::error::expected ("value [a-zA-Z_0-9]+", pos);
        }

        return fhg::util::parse::until (pos, &is_end_of_value);
      }

      template<unsigned int BITS>
      unsigned int require_uint (fhg::util::parse::position& pos)
      {
        unsigned int const i (fhg::util::read_uint (pos));

        if ((i >> BITS) > 0)
        {
          throw fhg::util::parse::error::expected
            ((boost::format ("number in [0,%1%)") % (1u << BITS)).str(), pos);
        }

        return i & ((1u << BITS) - 1);
      }

      std::string require_port (fhg::util::parse::position& pos)
      {
        if (pos.end() || ! (isdigit (*pos) || *pos == '*'))
        {
          throw fhg::util::parse::error::expected ("port {UINT16|*}", pos);
        }

        return *pos == '*' ? (++pos, "*")
          : boost::lexical_cast<std::string> (require_uint<16> (pos));
      }

      std::string require_ip (fhg::util::parse::position& pos)
      {
        unsigned int const a (require_uint<8> (pos));
        fhg::util::parse::require::require (pos, '.');
        unsigned int const b (require_uint<8> (pos));
        fhg::util::parse::require::require (pos, '.');
        unsigned int const c (require_uint<8> (pos));
        fhg::util::parse::require::require (pos, '.');
        unsigned int const d (require_uint<8> (pos));

        return (boost::format ("%1%.%2%.%3%.%4%") % a % b % c % d).str();
      }

      bool is_end_of_hostname (fhg::util::parse::position const& pos)
      {
        return ! (isalnum (*pos) || *pos == '.' || *pos == '-' || *pos == '_');
      }

      bool is_end_of_path (fhg::util::parse::position const &pos)
      {
        return *pos == '?';
      }

      std::string require_host (fhg::util::parse::position& pos)
      {
        if (pos.end() || ! (isalnum (*pos) || *pos == '*'))
        {
          throw fhg::util::parse::error::expected
            ("host {identifier_with_dot|ip}", pos);
        }

        return *pos == '*' ? (++pos, "*")
          : isdigit (*pos) ? require_ip (pos)
          : fhg::util::parse::until (pos, &is_end_of_hostname);
      }
    }

    url_t::url_t (std::string const& input)
    {
      fhg::util::parse::position_string pos (input);

      m_type = fhg::util::parse::require::identifier (pos);

      if (!pos.end())
      {
        fhg::util::parse::require::require (pos, "://");

        if (!pos.end())
        {
          if (*pos != '/' && *pos != '?')
          {
            m_path = require_host (pos);

            if (!pos.end() && *pos == ':')
            {
              m_path += *pos;
              ++pos;
              m_path += require_port (pos);
            }
          }

          while (!pos.end() && *pos == '/' && *pos != '?')
          {
            m_path += *pos;
            ++pos;
            m_path += fhg::util::parse::until (pos, &is_end_of_path);
          }

          char sep ('?');

          while (!pos.end())
          {
            fhg::util::parse::require::require (pos, sep); sep = '&';

            std::string const key (fhg::util::parse::require::identifier (pos));
            fhg::util::parse::require::require (pos, '=');
            if (!m_args.insert ( std::make_pair (key, require_value (pos))
                               ).second
               )
            {
              throw fhg::util::parse::error::generic
                ((boost::format ("duplicate key '%1%'") % key).str(), pos);
            }
          }
        }
      }
    }

    boost::optional<std::string const&>
    url_t::get (std::string const& key) const
    {
      arg_map_t::const_iterator const value (m_args.find (key));

      if (value != m_args.end ())
      {
        return value->second;
      }

      return boost::none;
    }
  }
}
