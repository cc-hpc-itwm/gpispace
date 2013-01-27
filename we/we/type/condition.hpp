// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONDITION_HPP
#define _WE_TYPE_CONDITION_HPP

#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/id.hpp>
#include <we/type/token.hpp>
#include <we/type/signature.hpp>

#include <we/util/cross.hpp>

#include <boost/function.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>

#include <sys/time.h>
#include <fhglog/macros.hpp>
#include <iomanip>

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

  static inline std::string no_trans (const petri_net::place_id_type &)
  {
    throw exception::no_translator_given();
  }

  class type
  {
  public:
    typedef expr::parse::parser parser_t;
    typedef expr::eval::context context_t;

  private:
    std::string expression_;
    parser_t parser;
    mutable context_t context;

    typedef boost::function<std::string (const petri_net::place_id_type &)> translate_t;
    translate_t translate;

    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, const unsigned int) const
    {
      ar & BOOST_SERIALIZATION_NVP(expression_);
    }
    template <typename Archive>
    void load(Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(expression_);
      parser = expr::parse::parser(expression_);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    friend std::ostream & operator<<(std::ostream &, const type &);

    typedef boost::unordered_map< petri_net::place_id_type
                                , std::vector<token::type>
                                > tokens_by_place_id_t;

  public:
    type ( const std::string & _expression
         , const translate_t & _translate = &no_trans
         )
      : expression_ (_expression)
        //! \todo do not initialize parser immediately, think of some other way
        // (pnetput should not parse the whole net just to put some tokens)
      , parser (_expression)
      , context ()
      , translate (_translate)
    {}

    // should correspond!
    type ( const std::string & _expression
         , const parser_t & _parser
         , const translate_t & _translate = &no_trans
         )
      : expression_ (_expression)
      , parser (_parser)
      , context ()
      , translate (_translate)
    {}

    bool operator () (cross::cross<tokens_by_place_id_t>& choices) const
    {
      if (expression_ == "true")
        {
          return true;
        }

      for (; choices.has_more(); ++choices)
        {
          if (choices.eval (parser, translate))
          {
            return true;
          }
        }

      return false;
    }

    const std::string & expression() const
    {
      return expression_;
    }

    bool is_const_true () const
    {
      try
        {
          context_t empty_context;

          return parser.eval_all_bool (empty_context);
        }
      catch (const expr::exception::eval::type_error &)
        {
          return false;
        }
      catch (const value::container::exception::missing_binding &)
        {
          return false;
        }
    }
  };

  inline std::ostream & operator << (std::ostream & os, const type & c)
  {
    return os << c.expression_;
  }
}

#endif
