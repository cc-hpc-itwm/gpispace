// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

// #include <xml/parse/type/function.hpp>

#include <xml/parse/util/id_type.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>
#include <stdexcept> // for visitor only

#include <boost/variant.hpp> // for visitor only

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> expressions_type;

      struct expression_type
      {
      private:
        expressions_type _expressions;

        id::expression _id;
        id::function _parent;

      public:
        expression_type ( const id::expression& id
                        , const id::function& parent
                        );
        expression_type ( const expressions_type & exps
                        , const id::expression& id
                        , const id::function& parent
                        );

        const id::expression& id() const;
        const id::function& parent() const;

        bool is_same (const expression_type& other) const;

        void set (const std::string& exps);

        std::string expression (const std::string& sep = " ") const;

        const expressions_type& expressions (void) const;
        expressions_type& expressions (void);
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const expression_type & e
                  );
      }

      //! \todo Only expose void join (function_type::type);
      //! \note This is currently not possible due to inclusion loops
      //! in function.hpp
      // void join (const expression_type& e, function_type::type& fun)
      namespace visitor
      {
        class join : public boost::static_visitor<void>
        {
        private:
          const expression_type & e;

        public:
          join (const expression_type & _e) : e(_e) {}

          void operator () (expression_type & x) const
          {
            x.expressions().insert ( x.expressions().end()
                                   , e.expressions().begin()
                                   , e.expressions().end()
                                   );
          }

          template<typename T>
          void operator () (T &) const
          {
            throw std::runtime_error ("BUMMER: join for non expression!");
          }
        };
      }
    }
  }
}

#endif
