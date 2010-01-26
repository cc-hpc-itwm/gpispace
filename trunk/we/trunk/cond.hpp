// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _COND_HPP
#define _COND_HPP

#include <netfwd.hpp>

#include <boost/function.hpp>

namespace Function { namespace Condition
{
  typedef petri_net::pid_t pid_t;
  typedef petri_net::eid_t eid_t;

  template<typename Token>
  struct Traits
  {
  public:
    typedef boost::function<bool ( const Token &
                                 , const pid_t &
                                 , const eid_t &
                                 )
                           > in_cond_t;

    typedef boost::function<bool ( const pid_t &
                                 , const eid_t &
                                 )
                           > out_cond_t;
  };

  namespace In
  {
    template<typename Token>
    class Generic
    {
    private:
      typedef typename Traits<Token>::in_cond_t Function;

      const Function f;

    public:
      explicit Generic (const Function & _f) : f (_f) {}

      bool operator () ( const Token & token
                       , const pid_t & pid
                       , const eid_t & eid
                       )
      {
        return f (token, pid, eid);
      }
    };

    template<typename Token>
    class Default
    {
    public:
      Default () {}

      bool operator () (const Token &, const pid_t &, const eid_t &)
      {
        return true;
      }
    };
  }

  namespace Out
  {
    template<typename Token>
    class Generic
    {
    private:
      typedef typename Traits<Token>::out_cond_t Function;

      const Function f;

    public:
      explicit Generic (const Function & _f) : f (_f) {}

      bool operator () (const pid_t & pid, const eid_t & eid)
      {
        return f (pid, eid);
      }
    };

    template<typename Token>
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
}}

#endif // _COND_HPP
