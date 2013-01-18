
#include <we/type/net.hpp>

#include <ostream>

namespace petri_net
{
  namespace
  {
    const std::vector<token::type>& no_tokens()
    {
      static const std::vector<token::type> x;

      return x;
    }
  }

  const std::vector<token::type>&
  net::get_token (const place_id_type& pid) const
  {
    token_by_place_id_t::const_iterator pos (_token_by_place_id.find (pid));

    return (pos != _token_by_place_id.end()) ? pos->second : no_tokens();
  }

  std::ostream& operator<< (std::ostream& s, const net& n)
  {
    typedef std::pair<petri_net::place_id_type, place::type> ip_type;

    BOOST_FOREACH (const ip_type& ip, n.places())
      {
        s << "[" << ip.second << ":";

        typedef boost::unordered_map<token::type, size_t> token_cnt_t;
        token_cnt_t token;
        BOOST_FOREACH (const token::type& t, n.get_token (ip.first))
          {
            token[t]++;
          }

        for (token_cnt_t::const_iterator t (token.begin()); t != token.end(); ++t)
        {
          if (t->second > 1)
          {
            s << " " << t->second << "x " << t->first;
          }
          else
          {
            s << " " << t->first;
          }
        }
        s << "]";
      }

    BOOST_FOREACH ( const we::type::transition_t& t
                  , n.transitions() | boost::adaptors::map_values
                  )
    {
      s << "/" << t << "/";
    }

      return s;
    }
} // namespace petri_net
