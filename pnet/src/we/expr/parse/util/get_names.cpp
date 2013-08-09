// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/util/get_names.hpp>

#include <we/type/value.hpp>

#include <boost/variant.hpp>

#include <iostream>

namespace expr
{
  namespace parse
  {
    namespace util
    {
      typedef boost::unordered_set<std::list<std::string> > name_set_t;

      namespace
      {
        class visitor_get_names : public boost::static_visitor<void>
        {
        private:
          name_set_t & _names;

        public:
          visitor_get_names (name_set_t & names) : _names (names) {}

          void operator () (const pnet::type::value::value_type &) const
          {}

          void operator () (const std::list<std::string>& key) const
          {
            _names.insert (key);
          }

          void operator () (const node::unary_t & u) const
          {
            boost::apply_visitor (*this, u.child);
          }

          void operator () (const node::binary_t & b) const
          {
            boost::apply_visitor (*this, b.l);
            boost::apply_visitor (*this, b.r);
          }

          void operator () (const node::ternary_t & t) const
          {
            boost::apply_visitor (*this, t.child0);
            boost::apply_visitor (*this, t.child1);
            boost::apply_visitor (*this, t.child2);
          }
        };
      }

      name_set_t get_names (const node::type& nd)
      {
        name_set_t names;

        boost::apply_visitor (visitor_get_names (names), nd);

        return names;
      }

      name_set_t get_names (const parser& p)
      {
        name_set_t names;

        for ( parser::nd_const_it_t it (p.begin()), end (p.end())
            ; it != end
            ; ++it
            )
        {
          boost::apply_visitor (visitor_get_names (names), *it);
        }

        return names;
      }

      std::string write_key_vec (const name_set_t::value_type& vec)
      {
        std::string name;
        name_set_t::value_type::const_iterator it (vec.begin());
        const name_set_t::value_type::const_iterator end (vec.end());

        while (it != end)
        {
          name += *it;
          ++it;
          if (it != end)
          {
            name += ".";
          }
        }
        return name;
      }
    }

    std::ostream& operator<< ( std::ostream& stream
                             , util::name_set_t::value_type ob
                             )
    {
      stream << util::write_key_vec (ob);
      return stream;
    }
  }
}
