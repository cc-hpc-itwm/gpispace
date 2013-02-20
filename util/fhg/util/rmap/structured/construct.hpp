// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_STRUCTURED_CONSTRUCT_HPP
#define _FHG_UTIL_RMAP_STRUCTURED_CONSTRUCT_HPP

#include <fhg/util/rmap/traits.hpp>
#include <fhg/util/rmap/structured/type.hpp>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      namespace structured
      {
        namespace visitor
        {
          template<typename Key, typename Mapped>
          class construct : public boost::static_visitor
                          <typename traits<Key, Mapped>::node_type&>
          {
          public:
            FHG_UTIL_RMAP_TRAITS();

            construct (node_type& node)
              : _node (node)
            {}
            node_type& operator() (structured_type& s) const
            {
              return _node;
            }
            node_type& operator() (mapped_type&) const
            {
              return _node = structured_type();
            }

          private:
            node_type& _node;
          };
        }

        template<typename Key, typename Mapped>
        typename traits<Key, Mapped>::node_type& construct
        (typename traits<Key, Mapped>::node_type& node)
        {
          return boost::apply_visitor
            (visitor::construct<Key, Mapped> (node), node);
        }
      }
    }
  }
}

#endif
