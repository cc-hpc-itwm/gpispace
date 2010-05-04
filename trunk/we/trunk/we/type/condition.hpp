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
    std::string expression_;
    expr::parse::parser<signature::field_name_t> parser;
    mutable expr::eval::context<signature::field_name_t> context;

    typedef boost::function<std::string (const petri_net::pid_t &)> translate_t;
    translate_t translate;

    typedef Function::Condition::Traits<token::type> traits;

    // WORK TODO split up into save/load, since we need to initialize the
    // parser
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, const unsigned int) const
    {
      ar & BOOST_SERIALIZATION_NVP(expression_);
    }
    template <typename Archive>
    void load(Archive & ar, const unsigned int) const
    {
      ar & BOOST_SERIALIZATION_NVP(expression_);
      parser = expr::parse::parser<signature::field_name_t>(expression_);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    friend std::ostream & operator<<(std::ostream &, const type &);
  public:
    type ( const std::string & _expression
         , const translate_t & _translate = &no_trans
         )
      : expression_ (_expression)
      , parser (_expression)
      , context ()
      , translate (_translate)
    {}

    bool operator () (traits::choices_t & choices) const
    {
      if (expression_ == "true")
        return true;

      for (; choices.has_more(); ++choices)
        {
          for ( traits::choice_it_t choice (*choices)
              ; choice.has_more()
              ; ++choice
              )
            {
              const petri_net::pid_t pid ((*choice).first);
              const token::type token ((*choice).second.first);

              context.bind (translate (pid), token.value);
            }

          if (parser.eval_all_bool (context))
            return true;
        }

      return false;
    }

    const std::string & expression() const
    {
      return expression_;
    }
  };

  inline std::ostream & operator << (std::ostream & os, const type & c)
  {
    return os << c.expression_;
  }
}

#endif
