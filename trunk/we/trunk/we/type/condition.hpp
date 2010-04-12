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
#include <boost/serialization/nvp.hpp>

#include <string>

namespace condition
{
  namespace exception
  {
    class no_translator_given : std::runtime_error
    {
    public:
      no_translator_given () : std::runtime_error ("no translator given") {};
    };
  }

  static inline std::string no_trans (const petri_net::pid_t &)
  {
    throw exception::no_translator_given();
  }

  class type
  {
  private:
    std::string expression;
    expr::parse::parser<signature::field_name_t> parser;
    expr::eval::context<signature::field_name_t> context;

    typedef boost::function<std::string (const petri_net::pid_t &)> translate_t;
    translate_t translate;

    typedef Function::Condition::Traits<token::type> traits;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(expression);
    }

  public:
    type (const std::string & _expression, const translate_t & _translate)
      : expression (_expression)
      , parser (expression)
      , context ()
      , translate (_translate)
    {}

    bool operator () (traits::choices_t & choices)
    {
      for (; choices.has_more(); ++choices)
        {
          for ( traits::choice_it_t choice (*choices)
              ; choice.has_more()
              ; ++choice
              )
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
