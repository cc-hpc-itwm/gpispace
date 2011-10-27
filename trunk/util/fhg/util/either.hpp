// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_EITHER_HPP
#define _FHG_UTIL_EITHER_HPP 1

#include <boost/variant.hpp>

namespace fhg
{
  namespace util
  {
    namespace either
    {
      namespace detail
      {
        template<typename T>
        class is_a : public boost::static_visitor<bool>
        {
        public:
          bool operator () (T) const { return true; }
          template<typename X> bool operator () (X) const { return false; }
        };
      }

      template<typename Left, typename Right>
      class type
      {
      private:
        typedef boost::variant<Left, Right> variant_type;

        variant_type _value;

      public:
        explicit type (Left value) : _value (value) {}
        explicit type (Right value) : _value (value) {}

        bool is_left () const
        {
          return boost::apply_visitor (detail::is_a<Left>(), _value);
        }

        bool is_right () const
        {
          return boost::apply_visitor (detail::is_a<Right>(), _value);
        }

        Left left () const { return boost::get<Left> (_value); }
        Right right () const { return boost::get<Right> (_value); }

        const type& operator = (Left l) { _value = l; return *this; }
        const type& operator = (Right r) { _value = r; return *this; }
      };
    }
  }
}

#endif
