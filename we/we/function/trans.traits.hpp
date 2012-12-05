// mirko.rahn@itwm.fraunhofer.de

#ifndef _FUNCTION_TRANS_TRAITS_HPP
#define _FUNCTION_TRANS_TRAITS_HPP

#include <we/type/id.hpp>

#include <vector>

#include <boost/unordered_map.hpp>

#include <boost/function.hpp>

namespace Function { namespace Transition
{
  typedef petri_net::pid_t pid_t;
  typedef petri_net::eid_t eid_t;

  template<typename Token>
  struct Traits
  {
  public:
    // a transition gets a number of input tokens, taken from places,
    // connected to the transition via edges
    // so input is of type: [(Token,(Place,Edge))]
    // the same holds true for the output, but the tokens are to be produced
    typedef std::pair<pid_t, eid_t> place_via_edge_t;
    typedef std::pair<Token, place_via_edge_t> token_input_t;
    typedef std::vector<token_input_t> input_t;

    typedef boost::unordered_map<pid_t,eid_t> output_descr_t;
    typedef std::pair<Token, pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> output_t;

    typedef boost::function<output_t ( const input_t &
                                     , const output_descr_t &
                                     )
                           > fun_t;
  };
}}

#endif
