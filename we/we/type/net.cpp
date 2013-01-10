
#include <we/type/net.hpp>

#include <ostream>

namespace petri_net
{
  namespace
  {
    const net::tokens_type& no_tokens()
    {
      static const net::tokens_type x;

      return x;
    }
  }

  const net::tokens_type& net::get_token (const place_id_type& pid) const
  {
    token_place_rel_t::const_iterator pos (_token_place_rel.find (pid));

    return (pos != _token_place_rel.end()) ? pos->second : no_tokens();
  }

  std::ostream& operator<< (std::ostream& s, const net& n)
  {
    for (net::place_const_it p (n.places()); p.has_more(); ++p)
      {
        s << "[" << n.get_place (*p) << ":";

        typedef boost::unordered_map<token::type, size_t> token_cnt_t;
        token_cnt_t token;
        BOOST_FOREACH (const token::type& t, n.get_token (*p))
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
