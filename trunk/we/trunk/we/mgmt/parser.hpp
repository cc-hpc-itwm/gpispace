/*
 * =====================================================================================
 *
 *       Filename:  parser.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/25/2010 05:05:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_PARSER_HPP
#define WE_MGMT_PARSER_HPP 1

#include <we/mgmt/bits/types.hpp>

namespace we { namespace mgmt {
  template <typename Net>
  struct NetTraits
  {
	typedef typename petri_net::pid_t pid_t;
	typedef typename petri_net::tid_t tid_t;
	typedef typename petri_net::eid_t eid_t;

	typedef typename detail::transition_t transition_t;
	typedef typename detail::place_t place_t;
	typedef typename detail::token_t token_t;
	typedef typename detail::edge_t edge_t;
  };

  template <typename _Net, typename _Traits, typename _Data>
  void parse (_Net & net, const _Data &)
  {
	typedef typename _Traits::pid_t pid_t;
	typedef typename _Traits::tid_t tid_t;
	typedef typename _Traits::transition_t transition_t;
	typedef typename _Traits::place_t place_t;
	typedef typename _Traits::edge_t edge_t;
	typedef typename _Traits::token_t token_t;

	pid_t pid_in = net.add_place(place_t("in"));
	pid_t pid_out = net.add_place(place_t("out"));
	tid_t tid_start ( net.add_transition( transition_t("start", transition_t::INTERNAL_SIMPLE)));
	net.add_edge (edge_t("in"), petri_net::connection_t (petri_net::PT, tid_start, pid_in));
	net.add_edge (edge_t("out"), petri_net::connection_t (petri_net::TP, tid_start, pid_out));

	net.put_token(pid_in, token_t("token-data"));
  }

  template <typename Stream, typename Net>
  inline Stream & operator << (Stream & s, const Net & n)
  {
	for (typename Net::place_const_it p (n.places()); p.has_more(); ++p)
	  {
		s << "[" << n.get_place (*p) << ":";

		for (typename Net::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
		  s << " " << *tp;

		s << "]";
	  }

	return s;
  }


}}

#endif
