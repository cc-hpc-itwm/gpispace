// condition functions, mirko.rahn@itwm.fraunhofer.de

#ifndef _FUNCTION_COND_EXP_HPP
#define _FUNCTION_COND_EXP_HPP

#include <we/function/cond.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <string>

namespace Function { namespace Condition
{
  namespace In
  {
    typedef double val_t;
    typedef std::pair<pid_t, val_t> bind_t;
    typedef std::vector<bind_t> default_t;
    typedef expr::parse::parser<pid_t,val_t> parser_t;
    typedef expr::eval::context<pid_t,val_t> context_t;

    template<typename token_type>
    class Expression
    {
    private:
      const std::string expression;
      const parser_t parser;
      context_t context;
    public:
      explicit Expression ( const std::string & _expression
                          , const default_t & dflt = default_t()
                          )
        : expression (_expression) 
        , parser (expression)
        , context ()
      {
        for ( default_t::const_iterator it (dflt.begin())
            ; it != dflt.end()
            ; ++it
            )
          context.bind (it->first, it->second);
      }

      bool operator () ( const token_type & token
                       , const pid_t & pid
                       , const eid_t &
                       )
      {
        context.bind (pid, double(token));

        return parser.eval_all_bool (context);
      }
    };
  }
}}

#endif
