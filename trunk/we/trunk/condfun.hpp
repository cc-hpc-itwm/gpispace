// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONDFUN_HPP
#define _CONDFUN_HPP

#include <transfun.hpp>

#include <boost/function.hpp>

namespace Function { namespace Condition
{
  template<typename Token>
  struct Traits
  {
  public:
    typedef Function::Transition::Traits<Token> tf_traits;
    typedef typename tf_traits::token_input_t token_input_t;
    typedef typename tf_traits::place_via_edge_t place_via_edge_t;

    typedef boost::function<bool (const token_input_t &)> precondfun_t;
    typedef boost::function<bool (const place_via_edge_t &)> postcondfun_t;
  };

  template<typename Token, typename Param>
  class Const
  {
  private:
    bool b;
  public:
    Const (const bool _b) : b (_b) {} 
    bool operator () (const Param &) { return b; }
  };

  namespace Pre
  {
    template<typename Token>
    class Default : public Const<Token,typename Traits<Token>::token_input_t>
    {
    public:
      Default () 
        : Const<Token,typename Traits<Token>::token_input_t> (true)
      {}
    };
  }

  namespace Post
  {
    template<typename Token>
    class Default : public Const<Token,typename Traits<Token>::place_via_edge_t>
    {
    public:
      Default () 
        : Const<Token,typename Traits<Token>::place_via_edge_t> (true)
      {}
    };
  }
}}

#endif // _TRANSFUN_HPP
