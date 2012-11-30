// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_PARSE_ID_MAPPER_HPP
#define XML_PARSE_ID_MAPPER_HPP

#include <xml/parse/id/mapper.fwd.hpp>

#include <xml/parse/id/types.hpp>

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
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/use.fwd.hpp>

#include <fhg/util/counter.hpp>

#include <boost/optional/optional_fwd.hpp>
#include <boost/scoped_ptr.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
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

#define ITEM(NAME,__IGNORE,TYPE,__IGNORE2)                              \
                                                                        \
        boost::optional<const type::TYPE&>                              \
        get (const boost::optional<NAME>&) const;                       \
                                                                        \
        boost::optional<type::TYPE&>                                    \
        get_ref (const boost::optional<NAME>&) const;                   \
                                                                        \
        boost::optional<const type::TYPE&>                              \
        get (const NAME&) const;                                        \
                                                                        \
        boost::optional<type::TYPE&>                                    \
        get_ref (const NAME&) const;                                    \
                                                                        \
        boost::optional<const type::TYPE&>                              \
        get (const ref::NAME&) const;                                   \
                                                                        \
        boost::optional<type::TYPE&>                                    \
        get_ref (const ref::NAME&) const;                               \
                                                                        \
        void put (const NAME&, const type::TYPE& elem);

#include <xml/parse/id/helper.lst>
#undef ITEM

        id::base_id_type next_id();

      private:

#define ITEM(NAME,__IGNORE,TYPE,__IGNORE2)                            \
                                                                      \
        void add_reference (const ref::NAME&);                        \
        void remove_reference (const ref::NAME&);                     \
                                                                      \
        friend struct ref::NAME;

#include <xml/parse/id/helper.lst>
#undef ITEM

        //! \note We need to use pimpl, as there is an include loop.
        //! \todo C++11: std::unique_ptr
        struct maps;
        boost::scoped_ptr<maps> _maps;
        ::fhg::util::counter<id::base_id_type> _counter;
      };
    }
  }
}

#endif
