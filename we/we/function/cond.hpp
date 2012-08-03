// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _FUNCTION_COND_HPP
#define _FUNCTION_COND_HPP

#include <we/type/id.hpp>
#include <we/util/cross.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>
#include <we/type/id.hpp>

#include <boost/unordered_map.hpp>

#include <boost/function.hpp>

#include <string>

#include <list>

#include <deque>
#include <boost/serialization/deque.hpp>

#include <fhg/util/show.hpp>

namespace Function { namespace Condition
{
  typedef petri_net::pid_t pid_t;
  typedef petri_net::eid_t eid_t;

  template<typename token_type>
  struct Traits
  {
  public:
    typedef std::pair<token_type,eid_t> token_via_edge_t;
    // typedef std::vector<token_via_edge_t> vec_token_via_edge_t;
    //    typedef std::list<token_via_edge_t> vec_token_via_edge_t;
    typedef std::deque<token_via_edge_t> vec_token_via_edge_t;
    typedef boost::unordered_map<pid_t,vec_token_via_edge_t> pid_in_map_t;

    typedef cross::cross<pid_in_map_t> choices_t;
    typedef cross::iterator<pid_in_map_t> choice_it_t;

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

  template<typename token_type>
  class Expression
  {
  private:
    typedef literal::type val_t;
    std::string expression;
    expr::parse::parser parser;
    mutable expr::eval::context context;
  public:
    explicit Expression (const std::string & _expression)
      : expression (_expression)
      , parser (expression)
      , context ()
    {}

    Expression & operator= (const Expression &rhs)
    {
      if (this != &rhs)
      {
        expression = rhs.expression;
        parser = expr::parse::parser (expression);
        // no  need to  copy  context,  it's only  there  to avoid  construction
        // overhead
      }
      return *this;
    }

    bool operator () (typename Traits<token_type>::choices_t & choices) const
    {
      if (expression == "true")
        return true;

      for (; choices.has_more(); ++choices)
        {
          for ( typename Traits<token_type>::choice_it_t choice (*choices)
              ; choice.has_more()
              ; ++choice
              )
            context.bind ( "_" + fhg::util::show ((*choice).first)
                         , val_t ((*choice).second.first)
                         );

          if (parser.eval_all_bool (context))
            return true;
        }

      return false;
    }
  };
}}

#endif
