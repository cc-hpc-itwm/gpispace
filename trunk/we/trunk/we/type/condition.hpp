// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONDITION_HPP
#define _WE_TYPE_CONDITION_HPP

#include <we/function/cond.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/id.hpp>
#include <we/type/token.hpp>
#include <we/type/signature.hpp>

#include <boost/function.hpp>

#include <string>

namespace condition
{
  template<typename NET>
  class type
  {
  private:
    const std::string expression;
    const expr::parse::parser<signature::field_name_t> parser;
    expr::eval::context<signature::field_name_t> context;

    typedef boost::function<std::string (const petri_net::pid_t &)> translate_t;
    const translate_t translate;

    typedef Function::Condition::Traits<token::type> traits;

  public:
    type (const std::string & _expression, const NET & net)
      : expression (_expression)
      , parser (expression)
      , context ()
      , translate (boost::bind (&place::name<NET>, boost::ref(net), _1))
    {}

    bool operator () (traits::choices_t & choices)
    {
      for (; choices.has_more(); ++choices)
        {
          traits::choice_star_it_t choice (*choices);

          for ( ; choice.has_more(); ++choice)
            {
              const petri_net::pid_t pid ((*choice).first);
              const token::type token ((*choice).second.first);

              token.bind (translate (pid), context);
            }

          if (parser.eval_all_bool (context))
            return true;
        }

      return false;
    }
  };
}

#endif
