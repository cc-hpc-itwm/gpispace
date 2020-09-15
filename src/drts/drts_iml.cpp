#include <drts/drts_iml.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>

#include <sstream>

namespace gspc
{
  namespace pnet
  {
    namespace vmem
    {
      ::pnet::type::value::value_type handle_to_value
        (gpi::pc::type::handle_id_t handle)
      {
        // taken from gpi-space/pc/type/handle.hpp
        std::ostringstream oss;

        oss << "0x";
        oss.flags (std::ios::hex);
        oss.width (18);
        oss.fill ('0');
        oss << handle;

        return oss.str();
      }

      ::pnet::type::value::value_type range_to_value
        (gpi::pc::type::range_t range)
      {
        ::pnet::type::value::value_type result;

        ::pnet::type::value::poke
            ("handle.name", result, handle_to_value (range.handle));
        ::pnet::type::value::poke ("offset", result, range.offset);
        ::pnet::type::value::poke ("size", result, range.size);

        return result;
      }

      ::pnet::type::value::value_type stream_slot_to_value
        ( gpi::pc::type::range_t const& metadata
        , gpi::pc::type::range_t const& data
        , char const flag
        , std::size_t const id
        )
      {
        ::pnet::type::value::value_type result;

        ::pnet::type::value::poke
            ("meta", result, range_to_value (metadata));
        ::pnet::type::value::poke
            ("data", result, range_to_value (data));
        ::pnet::type::value::poke ("flag", result, flag);
        ::pnet::type::value::poke ("id", result, id);

        return result;
      }
    }
  }
}
