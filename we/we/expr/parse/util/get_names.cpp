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
      typedef boost::unordered_set<node::key_vec_t> name_set_t;

      namespace detail
      {
        class get_names : public boost::static_visitor<void>
        {
        private:
          name_set_t & _names;

        public:
          get_names (name_set_t & names) : _names (names) {}

          void operator () (const value::type &) const
          {}

          void operator () (const node::key_vec_t & key) const
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

        boost::apply_visitor (detail::get_names (names), nd);

        return names;
      }

      name_set_t get_names (const parser& p)
      {
        name_set_t names;

        for ( parser::nd_const_it_t it (p.begin()), end (p.end())
            ; it != end
            ; ++it )
        {
          boost::apply_visitor (detail::get_names (names), *it);
        }

        return names;
      }

      std::string write_key_vec (const name_set_t::value_type& vec)
      {
        std::string name;
        for ( name_set_t::value_type::const_iterator it (vec.begin()), end (vec.end())
            ; it != end
            ; ++it
            )
        {
          name += *it;
          if (it + 1 != end)
          {
            name += ".";
          }
        }
        return name;
      }

      node::key_vec_t get_normal_name (node::key_vec_t ssa_name)
      {
        if (ssa_name[1] == ".temp")
        {
          ssa_name.erase (ssa_name.begin());
          return ssa_name;
        }

        //! \note Yes, a size_t would be nicer, but would get termination-problems.
        signed long i = ssa_name.size() - 1;
        while (i >= 0)
        {
          ssa_name.erase (ssa_name.begin() + i);
          i -= 2;
        }
        return ssa_name;
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
