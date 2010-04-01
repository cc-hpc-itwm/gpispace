// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _FUNCTION_COND_HPP
#define _FUNCTION_COND_HPP

#include <we/type/id.hpp>
#include <we/util/cross.hpp>

#include <boost/unordered_map.hpp>

#include <boost/function.hpp>

namespace Function { namespace Condition
{
  typedef petri_net::pid_t pid_t;
  typedef petri_net::eid_t eid_t;

  template<typename token_type>
  struct Traits
  {
  public:
    typedef std::pair<token_type,eid_t> token_via_edge_t;
    typedef std::vector<token_via_edge_t> vec_token_via_edge_t;
    typedef boost::unordered_map<pid_t,vec_token_via_edge_t> pid_in_map_t;

    typedef cross::cross<pid_in_map_t> choices_t;
    typedef cross::star_iterator<pid_in_map_t> choice_star_it_t;

    // set the cross product either to the end or to some valid choice
    typedef boost::function<bool (choices_t &)> choice_cond_t;
  };

  template<typename token_type>
  class Generic
  {
  public:
    typedef typename Traits<token_type>::choice_cond_t Function;

  private:
    const Function f;

  public:
    explicit Generic (const Function & _f) : f (_f) {}

    bool operator () (typename Traits<token_type>::choices_t & choices)
    {
      return f(choices);
    }
  };

  template<typename token_type>
  class Default
  {
  public:
    Default () {}

    bool operator () (typename Traits<token_type>::choices_t &)
    {
      return true;
    }
  };
}}

#endif
