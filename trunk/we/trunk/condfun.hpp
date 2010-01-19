// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONDFUN_HPP
#define _CONDFUN_HPP

#include <trans.hpp>

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

    typedef boost::function<bool (const token_input_t &)> in_condfun_t;
    typedef boost::function<bool (const place_via_edge_t &)> out_condfun_t;
  };

  template<typename Token, typename Param>
  class Gen
  {
  private:
    typedef boost::function<bool (const Param &)> Function;

    const Function f;
  public:
    Gen (const Function & _f) : f (_f) {}

    bool operator () (const Param & param) { return f (param); }
  };

  template<typename T>
  bool fconst (const bool & b, const T &) { return b; }

  namespace In
  {
    template<typename Token>
    class Generic : public Gen<Token,typename Traits<Token>::token_input_t>
    {
    public:
      Generic (const typename Traits<Token>::in_condfun_t & f)
        : Gen<Token, typename Traits<Token>::token_input_t> (f)
      {}
    };

    template<typename Token>
    class Default : public Generic<Token>
    {
    private:
      typedef typename Traits<Token>::token_input_t param_t;

    public:
      Default () : Generic<Token> (boost::bind(&fconst<param_t>, true, _1)) {}
    };
  }

  namespace Out
  {
    template<typename Token>
    class Generic : public Gen<Token,typename Traits<Token>::place_via_edge_t>
    {
    public:
      Generic (const typename Traits<Token>::out_condfun_t & f)
        : Gen<Token, typename Traits<Token>::place_via_edge_t> (f)
      {}
    };

    template<typename Token>
    class Default : public Generic<Token>
    {
    private:
      typedef typename Traits<Token>::place_via_edge_t param_t;

    public:
      Default () : Generic<Token> (boost::bind(&fconst<param_t>, true, _1)) {}
    };
  }
}}

#endif // _TRANSFUN_HPP
