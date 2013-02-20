// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/node.hpp>

#include <we/expr/token/prop.hpp>

#include <we/expr/exception.hpp>

#include <we/type/value/show.hpp>

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
        std::string show_key_vec (const key_vec_t & key_vec)
        {
          std::string s;

          for ( key_vec_t::const_iterator pos (key_vec.begin())
              ; pos != key_vec.end()
              ; ++pos
              )
            s += ((pos != key_vec.begin()) ? "." : "") + fhg::util::show (*pos);

          return s;
        }
      }

      namespace visitor
      {
        class show : public boost::static_visitor<void>
        {
        private:
          std::ostream & s;

        public:
          show (std::ostream & _s) : s (_s) {}

          void operator () (const value::type & v) const
          {
            s << v;
          }

          void operator () (const key_vec_t & key) const
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
        boost::apply_visitor (visitor::show (s), node);

        return s;
      }

      namespace visitor
      {
        class get_value : public boost::static_visitor<const value::type &>
        {
        public:
          const value::type & operator () (const value::type & v) const
          {
            return v;
          }
          template<typename T>
          const value::type & operator () (const T &) const
          {
            throw exception::eval::type_error ("get: node is not an value");
          }
        };
      }

      const value::type& get (const type & node)
      {
        return boost::apply_visitor (visitor::get_value(), node);
      }

      namespace visitor
      {
        class is_value : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const value::type &) const { return true; }
          bool operator () (const key_vec_t &) const { return false; }
          bool operator () (const unary_t &) const { return false; }
          bool operator () (const binary_t &) const { return false; }
          bool operator () (const ternary_t &) const { return false; }
        };
      }

      bool is_value (const type& node)
      {
        return boost::apply_visitor (visitor::is_value(), node);
      }

      namespace visitor
      {
        class is_ref : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const value::type &) const { return false; }
          bool operator () (const key_vec_t &) const { return true; }
          bool operator () (const unary_t &) const { return false; }
          bool operator () (const binary_t &) const { return false; }
          bool operator () (const ternary_t &) const { return false; }
        };
      }

      bool is_ref (const type& node)
      {
        return boost::apply_visitor (visitor::is_ref(), node);
      }

      namespace visitor
      {
        class rename : public boost::static_visitor<void>
        {
        private:
          const key_vec_t::value_type from;
          const key_vec_t::value_type to;

        public:
          rename ( const key_vec_t::value_type & _from
                 , const key_vec_t::value_type & _to
                 )
            : from (_from), to (_to)
          {}

          void operator () (value::type &) const
          {
            return;
          }

          void operator () (key_vec_t & v) const
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
        };
      }

      void rename ( type & t
                  , const key_vec_t::value_type & from
                  , const key_vec_t::value_type & to
                  )
      {
        boost::apply_visitor (visitor::rename (from, to), t);
      }
    }
  }
}
