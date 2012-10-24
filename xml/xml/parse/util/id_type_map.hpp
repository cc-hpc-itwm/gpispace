// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_UTIL_ID_TYPE_MAP_HPP
#define XML_UTIL_ID_TYPE_MAP_HPP

#include <xml/parse/util/id_type_map.fwd.hpp>

#include <xml/parse/util/id_type.fwd.hpp>

#include <xml/parse/type/connect.fwd.hpp>
#include <xml/parse/type/expression.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/mod.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place.fwd.hpp>
#include <xml/parse/type/place_map.fwd.hpp>
#include <xml/parse/type/port.fwd.hpp>
#include <xml/parse/type/specialize.fwd.hpp>
#include <xml/parse/type/struct.fwd.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/token.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/use.fwd.hpp>

#include <boost/optional/optional_fwd.hpp>
#include <boost/scoped_ptr.hpp>

namespace xml
{
  namespace parse
  {
    namespace id_map
    {
      class mapper
      {
      public:
        mapper();
        //! \note This does nothing but boost::~scoped_ptr<maps>,
        //! which needs ~maps, which is only defined in the C++, thus
        //! not visible to the implicit version defined where mapper
        //! is used.
        ~mapper();

#define ITEM(NAME,__IGNORE,TYPE)                                      \
                                                                      \
        boost::optional<type::TYPE> get (const id::NAME&) const;      \
        void put (const id::NAME&, const type::TYPE& elem);           \
                                                                      \
        void add_reference (const id::NAME&);                         \
        void remove_reference (const id::NAME&);

#include <xml/parse/util/id_type_map_helper.lst>
#undef ITEM

      private:

        //! \note We need to use pimpl, as there is an include loop.
        //! \todo C++11: std::unique_ptr
        struct maps;
        boost::scoped_ptr<maps> _maps;
      };
    }
  }
}

#endif
