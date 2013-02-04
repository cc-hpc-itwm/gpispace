// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_WRITE_HPP
#define _FHG_UTIL_RMAP_WRITE_HPP

#include <fhg/util/rmap/type.hpp>

#include <boost/foreach.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      template<typename Key, typename Mapped>
      std::ostream&
      write_to ( std::ostream&
               , const typename traits<Key, Mapped>::variant_type&
               , unsigned int
               );

      template<typename Key, typename Mapped>
      std::ostream& operator<< (std::ostream& os, const type<Key, Mapped>& x)
      {
        return write_to<Key, Mapped> (os, x._variant, 0);
      }

      namespace visitor
      {
        template<typename Key, typename Mapped>
        class writer : public boost::static_visitor<std::ostream&>
        {
        public:
          FHG_UTIL_RMAP_TRAITS();

          writer (std::ostream& os, unsigned int level)
            : _os (os)
            , _level (level)
          {}

          std::ostream& operator() (const mapped_type& m) const
          {
            add_header();
            return _os << m << std::endl;
          }
          std::ostream& operator() (const structured_type& s) const
          {
            BOOST_FOREACH (const kv_type& kv, s.map())
            {
              add_header();
              _os << kv.first << ":" << std::endl;
              write_to<Key, Mapped> (_os, kv.second, _level + 1);
            }

            return _os;
          }

        private:
          std::ostream& _os;
          unsigned int _level;

          void add_header() const
          {
            for (unsigned int i (0); i < _level; ++i)
            {
              _os << " | ";
            }
          }
        };
      }

      template<typename Key, typename Mapped>
      std::ostream&
      write_to ( std::ostream& os
               , const typename traits<Key, Mapped>::variant_type& variant
               , unsigned int level
               )
      {
        return boost::apply_visitor
          (visitor::writer<Key, Mapped> (os, level), variant);
      }
    }
  }
}

#endif
