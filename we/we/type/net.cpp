
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
} // namespace petri_net
