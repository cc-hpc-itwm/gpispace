/*
 * =====================================================================================
 *
 *       Filename:  net_output_operator.hpp
 *
 *    Description:  net output operator
 *
 *        Version:  1.0
 *        Created:  04/14/2010 12:08:47 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_UTIL_NET_OUTPUT_OPERATOR_HPP
#define WE_UTIL_NET_OUTPUT_OPERATOR_HPP 1

#include <ostream>

#include <we/util/show.hpp>
#include <we/net.hpp>

namespace we { namespace util {
  template <typename Place, typename Trans, typename Edge, typename Token>
  std::ostream & operator << (std::ostream & s, const petri_net::net<Place, Trans, Edge, Token> & n)
  {
    typedef petri_net::net<Place, Trans, Edge, Token> pnet_t;

    for (typename pnet_t::place_const_it p = (n.places()); p.has_more(); ++p)
    {
      s << "[" << n.get_place (*p) << ":";

      typedef boost::unordered_map<Token, size_t> token_cnt_t;
      token_cnt_t token;
      for (typename pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
      {
        token[*tp]++;
      }

      for (typename token_cnt_t::const_iterator t (token.begin()); t != token.end(); ++t)
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

    return s;
  }
}}

#endif
