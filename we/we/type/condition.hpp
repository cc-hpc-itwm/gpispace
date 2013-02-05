// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONDITION_HPP
#define _WE_TYPE_CONDITION_HPP

#include <we/expr/parse/parser.hpp>

#include <we/type/id.hpp>
#include <we/type/value.hpp>
#include <we/type/signature.hpp>

#include <we/type/value/container/exception.hpp>

#include <boost/function.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>

#include <sys/time.h>
#include <fhglog/macros.hpp>
#include <iomanip>

namespace condition
{
  class type
  {
  private:
    std::string _expression;
    expr::parse::parser _parser;

    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive& ar, const unsigned int) const
    {
      ar & BOOST_SERIALIZATION_NVP (_expression);
    }
    template <typename Archive>
    void load(Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_expression);
      _parser = expr::parse::parser (_expression);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    friend std::ostream& operator<< (std::ostream&, const type&);

    typedef boost::unordered_map< petri_net::place_id_type
                                , std::list<value::type>
                                > tokens_by_place_id_t;

  public:
    type ( const std::string& exp)
      : _expression (exp)
        //! \todo do not initialize parser immediately, think of some other way
        // (pnetput should not parse the whole net just to put some tokens)
      , _parser (exp)
    {}

    // should correspond!
    type ( const std::string& exp
         , const expr::parse::parser& p
         )
      : _expression (exp)
      , _parser (p)
    {}

    const std::string& expression() const
    {
      return _expression;
    }

    const expr::parse::parser& parser() const
    {
      return _parser;
    }

    bool is_const_true() const
    {
      try
        {
          return _parser.eval_all_bool();
        }
      catch (const expr::exception::eval::type_error&)
        {
          return false;
        }
      catch (const value::container::exception::missing_binding&)
        {
          return false;
        }
    }
  };

  inline std::ostream& operator << (std::ostream& os, const type& c)
  {
    return os << c._expression;
  }
}

#endif
