
#include <we/type/net.hpp>

#include <ostream>

namespace petri_net
{
  std::ostream& operator<< (std::ostream& s, const net& n)
  {
    for (net::place_const_it p (n.places()); p.has_more(); ++p)
      {
        s << "[" << n.get_place (*p) << ":";

        typedef boost::unordered_map<token::type, size_t> token_cnt_t;
        token_cnt_t token;
        for (net::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        {
          token[*tp]++;
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

      for (net::transition_const_it t (n.transitions()); t.has_more(); ++t)
      {
        s << "/";
        s << n.get_transition (*t);
        s << "/";
      }

      return s;
    }
} // namespace petri_net
