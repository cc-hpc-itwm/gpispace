// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/node.hpp>

#include <we/expr/token/prop.hpp>

#include <we/expr/exception.hpp>

#include <we2/type/value/show.hpp>

#include <fhg/util/show.hpp>

#include <boost/foreach.hpp>

namespace expr
{
  namespace parse
  {
    namespace node
    {
      unary_t::unary_t (const token::type & _token, const type & _child)
        : token (_token), child (_child)
      {}

      binary_t::binary_t ( const token::type & _token
                         , const type & _l
                         , const type & _r
                         )
          : token (_token), l (_l), r (_r)
      {}

      ternary_t::ternary_t ( const token::type & _token
                           , const type & _child0
                           , const type & _child1
                           , const type & _child2
                           )
        : token (_token), child0 (_child0), child1 (_child1), child2 (_child2)
      {}

      namespace {
        std::string show_key_vec (const std::list<std::string>& key_vec)
        {
          std::string s;

          BOOST_FOREACH (const std::string& k, key_vec)
          {
            if (!s.empty())
            {
              s += ".";
            }
            s += k;
          }

          return s;
        }
      }

      namespace
      {
        class visitor_show : public boost::static_visitor<void>
        {
        private:
          std::ostream & s;

        public:
          visitor_show (std::ostream & _s) : s (_s) {}

          void operator () (const pnet::type::value::value_type & v) const
          {
            s << pnet::type::value::show (v);
          }

          void operator () (const std::list<std::string>& key) const
          {
            s << "${" << show_key_vec (key) << "}";
          }

          void operator () (const unary_t & u) const
          {
            s << u.token << "(" << u.child << ")";
          }

          void operator () (const binary_t & b) const
          {
            if (token::is_prefix (b.token))
              {
                s << b.token << "(" << b.l << ", " <<  b.r << ")";
              }
            else
              {
                s << "(" << b.l << b.token << b.r << ")";
              }
          }

          void operator () (const ternary_t & t) const
          {
            switch (t.token)
              {
              case token::_map_assign:
                s << token::_map_assign
                  << "(" << t.child0
                  << "," << t.child1
                  << "," << t.child2
                  << ")"
                  ;
                break;
              default:
                throw exception::strange ("unknown ternary token");
              }
          }
        };
      }

      std::ostream& operator << (std::ostream & s, const type & node)
      {
        boost::apply_visitor (visitor_show (s), node);

        return s;
      }

      namespace
      {
        class visitor_get_value
          : public boost::static_visitor<const pnet::type::value::value_type &>
        {
        public:
          const pnet::type::value::value_type & operator () (const pnet::type::value::value_type & v) const
          {
            return v;
          }
          template<typename T>
          const pnet::type::value::value_type & operator () (const T &) const
          {
            throw exception::eval::type_error ("get: node is not an value");
          }
        };
      }

      const pnet::type::value::value_type& get (const type & node)
      {
        return boost::apply_visitor (visitor_get_value(), node);
      }

      namespace
      {
        class visitor_is_value : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const pnet::type::value::value_type &) const { return true; }
          bool operator () (const std::list<std::string>&) const { return false; }
          bool operator () (const unary_t &) const { return false; }
          bool operator () (const binary_t &) const { return false; }
          bool operator () (const ternary_t &) const { return false; }
        };
      }

      bool is_value (const type& node)
      {
        return boost::apply_visitor (visitor_is_value(), node);
      }

      namespace
      {
        class visitor_is_ref : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const pnet::type::value::value_type &) const { return false; }
          bool operator () (const std::list<std::string>&) const { return true; }
          bool operator () (const unary_t &) const { return false; }
          bool operator () (const binary_t &) const { return false; }
          bool operator () (const ternary_t &) const { return false; }
        };
      }

      bool is_ref (const type& node)
      {
        return boost::apply_visitor (visitor_is_ref(), node);
      }

      namespace
      {
        class visitor_rename : public boost::static_visitor<void>
        {
        public:
          visitor_rename (const std::string& _from, const std::string& _to)
            : from (_from), to (_to)
          {}

          void operator () (pnet::type::value::value_type &) const
          {
            return;
          }

          void operator () (std::list<std::string>& v) const
          {
            if (v.size() > 0 && *v.begin() == from)
              {
                *v.begin() = to;
              }
          }

          void operator () (unary_t & u) const
          {
            boost::apply_visitor (*this, u.child);
          }

          void operator () (binary_t & b) const
          {
            boost::apply_visitor (*this, b.l);
            boost::apply_visitor (*this, b.r);
          }

          void operator () (ternary_t & t) const
          {
            boost::apply_visitor (*this, t.child0);
            boost::apply_visitor (*this, t.child1);
            boost::apply_visitor (*this, t.child2);
          }

        private:
          const std::string from;
          const std::string to;
        };
      }

      void rename (type& t, const std::string& from, const std::string& to)
      {
        boost::apply_visitor (visitor_rename (from, to), t);
      }
    }
  }
}
