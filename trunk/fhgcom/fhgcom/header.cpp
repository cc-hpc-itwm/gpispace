#include "ns_uuid.hpp"
#include "header.hpp"

#include <cstring>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      static boost::uuids::uuid fhg_com_uuid ()
      {
        static boost::uuids::string_generator g;
        static boost::uuids::uuid u = g(FHG_COM_NS_UUID);
        return u;
      }

      static boost::uuids::uuid fhg_com_gen_uuid(std::string const & name)
      {
        static boost::uuids::name_generator gen (fhg_com_uuid());
        return gen(name);
      }

      void translate_name (std::string const & name, address_t & addr)
      {
        boost::uuids::uuid u = fhg_com_gen_uuid(name);
        memcpy (&addr, &u, sizeof(u));
      }
    }
  }
}
