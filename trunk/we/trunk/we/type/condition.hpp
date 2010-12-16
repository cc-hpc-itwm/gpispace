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

#include <sys/time.h>
#include <fhglog/macros.hpp>
#include <iomanip>

namespace statistics
{
  static inline double current_time()
  {
    struct timeval tv;

    gettimeofday (&tv, NULL);

    return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
  }

  typedef boost::unordered_map<std::string, double> time_map_t;
  typedef boost::unordered_map<std::string, unsigned long> cnt_map_t;

  static inline time_map_t & get_time_map ()
  {
    static time_map_t time_map;

    return time_map;
  }

  static inline cnt_map_t & get_call_map ()
  {
    static cnt_map_t map;

    return map;
  }

  static inline cnt_map_t & get_eval_map ()
  {
    static cnt_map_t map;

    return map;
  }

  static inline cnt_map_t & get_false_map ()
  {
    static cnt_map_t map;

    return map;
  }

  static inline cnt_map_t & get_true_map ()
  {
    static cnt_map_t map;

    return map;
  }

  static inline cnt_map_t & get_max_eval_map ()
  {
    static cnt_map_t map;

    return map;
  }

  static inline cnt_map_t & get_max_size_map ()
  {
    static cnt_map_t map;

    return map;
  }

  static inline unsigned long value (const cnt_map_t & m, const std::string & key)
  {
    cnt_map_t::const_iterator pos (m.find (key));

    return (pos == m.end()) ? 0 : pos->second;
  }

  static inline void reset_maps ()
  {
    get_time_map().clear();
    get_call_map().clear();
    get_eval_map().clear();
    get_false_map().clear();
    get_true_map().clear();
    get_max_eval_map().clear();
    get_max_size_map().clear();
  }

  static inline void dump_maps ()
  {
    for ( time_map_t::const_iterator pos (get_time_map().begin())
        ; pos != get_time_map().end()
	; ++pos
	)
      {
	LOG( INFO
	   , "stat_map "
	   << std::setw(12) << std::setprecision(5) << pos->second
	   << " call " << std::setw(10) << value (get_call_map(), pos->first)
	   << " eval " << std::setw(10) << value (get_eval_map(), pos->first)
	   << " max_eval " << std::setw(10) << value (get_max_eval_map(), pos->first)
	   << " max_size " << std::setw(10) << value (get_max_size_map(), pos->first)
	   << " true " << std::setw(10) << value (get_true_map(), pos->first)
	   << " false " << std::setw(10) << value (get_false_map(), pos->first)
	   << " " << pos->first
	   );
      }
  }
}

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
  public:
    typedef expr::parse::parser parser_t;
    typedef expr::eval::context context_t;

  private:
    std::string expression_;
    parser_t parser;
    mutable context_t context;

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
      parser = expr::parse::parser(expression_);
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

    bool operator () (traits::choices_t & choices) const
    {
      unsigned long eval (0);

      statistics::get_call_map()[expression_] += 1;

      statistics::get_max_size_map()[expression_]
	= std::max ( statistics::get_max_size_map()[expression_]
		   , choices.size()
		   );
	;
      statistics::get_time_map()[expression_] -= statistics::current_time();

      if (expression_ == "true")
	{
	  statistics::get_time_map()[expression_] += statistics::current_time();
	  statistics::get_true_map()[expression_] += 1;

	  return true;
	}

      for (; choices.has_more(); ++choices)
        {
          for ( traits::choice_it_t choice (*choices)
              ; choice.has_more()
              ; ++choice
              )
            {
              const petri_net::pid_t & pid (choice.key());
              const token::type & token (choice.val().first);

              context.bind (translate (pid), token.value);
            }

	  statistics::get_eval_map()[expression_] += 1;
	  eval += 1;

          if (parser.eval_all_bool (context))
	    {
	      statistics::get_time_map()[expression_] += statistics::current_time();

	      statistics::get_true_map()[expression_] += 1;
	      statistics::get_max_eval_map()[expression_]
		= std::max ( statistics::get_max_eval_map()[expression_]
			   , eval
			   );

	      return true;
	    }
        }

      statistics::get_time_map()[expression_] += statistics::current_time();

      statistics::get_false_map()[expression_] += 1;
      statistics::get_max_eval_map()[expression_]
	= std::max ( statistics::get_max_eval_map()[expression_]
		   , eval
		   );

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
