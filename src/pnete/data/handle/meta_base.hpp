// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_DATA_HANDLE_META_BASE_HPP
#define FHG_PNETE_DATA_HANDLE_META_BASE_HPP

#include <pnete/data/handle/meta_base.fwd.hpp>

#include <pnete/data/handle/base.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        template<typename ID_TYPE, typename XML_TYPE>
          class meta_base : public base
        {
        protected:
          typedef ID_TYPE id_type;
          typedef XML_TYPE xml_type;

        public:
          meta_base (const id_type& id, internal_type* document)
            : base (document)
            , _id (id)
          { }

          const xml_type& get() const
          {
            return id().get();
          }
          xml_type& get_ref() const
          {
            return id().get_ref();
          }

          bool operator== (const meta_base<id_type, xml_type>& other) const
          {
            return id() == other.id();
          }

          const id_type& id() const
          {
            return _id;
          }

        private:
          id_type _id;
        };
      }
    }
  }
}

#endif
