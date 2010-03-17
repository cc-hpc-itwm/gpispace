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
    typedef boost::function<bool ( const token_type &
                                 , const pid_t &
                                 , const eid_t &
                                 )
                           > in_cond_t;

    typedef boost::function<bool ( const pid_t &
                                 , const eid_t &
                                 )
                           > out_cond_t;

    typedef std::pair<token_type,eid_t> token_via_edge_t;
    typedef std::vector<token_via_edge_t> vec_token_via_edge_t;
    typedef boost::unordered_map<pid_t,vec_token_via_edge_t> pid_in_map_t;

    typedef cross::cross<pid_in_map_t> choices_t;

    // set the cross product either to the end or to some valid choice
    typedef boost::function<bool (choices_t &)> choice_cond_t;
  };

  namespace In
  {
    template<typename token_type>
    class Generic
    {
    public:
      typedef typename Traits<token_type>::in_cond_t Function;

    private:
      const Function f;

    public:
      explicit Generic (const Function & _f) : f (_f) {}

      bool operator () ( const token_type & token
                       , const pid_t & pid
                       , const eid_t & eid
                       )
      {
        return f (token, pid, eid);
      }
    };

    template<typename token_type>
    class Default
    {
    public:
      Default () {}

      bool operator () (const token_type &, const pid_t &, const eid_t &)
      {
        return true;
      }
    };
  }

  namespace Out
  {
    template<typename token_type>
    class Generic
    {
    public:
      typedef typename Traits<token_type>::out_cond_t Function;

    private:
      const Function f;

    public:
      explicit Generic (const Function & _f) : f (_f) {}

      bool operator () (const pid_t & pid, const eid_t & eid)
      {
        return f (pid, eid);
      }
    };

    template<typename token_type>
    class Default
    {
    public:
      Default () {}

      bool operator () (const pid_t &, const eid_t &)
      {
        return true;
      }
    };
  }

  namespace Choice
  {
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
  }
}}

#endif
