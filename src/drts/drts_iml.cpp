#include <drts/drts_iml.hpp>

#include <we/type/value/poke.hpp>
#include <we/type/range.hpp>

#include <boost/range/adaptor/map.hpp>

#include <sstream>

namespace gspc
{
  namespace iml
  {
    pnet::type::value::value_type remote_iml_gpi_global_memory_range
      (gpi::pc::type::range_t iml_range)
    {
      //\todo external check for (offset + size) > _->_size

      pnet::type::value::value_type range;
      {
        // taken from gpi-space/pc/type/handle.hpp
        std::ostringstream oss;

        oss << "0x";
        oss.flags (std::ios::hex);
        oss.width (18);
        oss.fill ('0');
        oss << iml_range.handle;

        pnet::type::value::poke (std::list<std::string> {"handle", "name"}, range, oss.str());
      }
      pnet::type::value::poke (std::list<std::string> {"offset"}, range, iml_range.offset);
      pnet::type::value::poke (std::list<std::string> {"size"}, range, iml_range.size);

      return range;
    }
  }
}
