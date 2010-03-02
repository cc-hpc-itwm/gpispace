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
	typedef petri_net::pid_t pid_t;
	typedef petri_net::tid_t tid_t;
	typedef petri_net::eid_t eid_t;
  };

  template <typename _Net, typename _NetTraits, typename _Data>
  void parse (_Net & net, const _Data &)
  {
	typename _NetTraits::pid_t pid_in = net.add_place(detail::place_t("in"));
	typename _NetTraits::pid_t pid_out = net.add_place(detail::place_t("out"));
	typename _NetTraits::tid_t tid_start (
	  net.add_transition(
		detail::transition_t("start", detail::transition_t::INTERNAL_SIMPLE)));
	net.add_edge(detail::edge_t("in"), petri_net::connection_t (petri_net::PT, tid_start, pid_in));
	net.add_edge(detail::edge_t("out"), petri_net::connection_t (petri_net::TP, tid_start, pid_out));
  }
}}

#endif
