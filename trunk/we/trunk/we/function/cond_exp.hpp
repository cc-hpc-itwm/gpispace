// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _FUNCTION_COND_EXP_HPP
#define _FUNCTION_COND_EXP_HPP

#include <we/function/cond.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/expr/variant/variant.hpp>

#include <we/type/token.hpp>
#include <we/util/show.hpp>

#include <string>

namespace Function { namespace Condition
{
  template<typename token_type>
  class Expression
  {
  private:
    typedef expr::variant::type val_t;
    const std::string expression;
    const expr::parse::parser<pid_t> parser;
    expr::eval::context<pid_t> context;
  public:
    explicit Expression (const std::string & _expression)
      : expression (_expression)
      , parser (expression)
      , context ()
    {}

    bool operator () (typename Traits<token_type>::choices_t & choices)
    {
      for (; choices.has_more(); ++choices)
        {
          typename Traits<token_type>::choice_star_it_t choice (*choices);

          for ( ; choice.has_more(); ++choice)
            context.bind (choice->first, val_t (choice->second.first));

          if (parser.eval_all_bool (context))
            return true;
        }

      return false;
    }
  };

  template<>
  class Expression<we::token::type>
  {
  private:
    const std::string expression;
    const expr::parse::parser<std::string> parser;
    expr::eval::context<std::string> context;
  public:
    explicit Expression (const std::string & _expression)
      : expression (_expression)
      , parser (expression)
      , context ()
    {}

    bool operator () (Traits<we::token::type>::choices_t & choices)
    {
      for (; choices.has_more(); ++choices)
        {
          Traits<we::token::type>::choice_star_it_t choice (*choices);

          for ( ; choice.has_more(); ++choice)
            {
              const pid_t pid (choice->first);
              const we::token::type token (choice->second.first);
              
              for ( we::token::type::const_iterator field (token.begin())
                  ; field != token.end()
                  ; ++field
                  )
                context.bind ( util::show (pid) + "." + we::token::name (*field)
                             , we::token::value (*field)
                             );
            }

          if (parser.eval_all_bool (context))
            return true;
        }

      return false;
    }
  };
}}

#endif
