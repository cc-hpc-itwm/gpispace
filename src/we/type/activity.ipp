#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>

namespace we
{
  namespace type
  {
    template<class Archive>
      void activity_t::serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_data);
      ar & BOOST_SERIALIZATION_NVP (_transition_id);
      ar & BOOST_SERIALIZATION_NVP (_input);
      ar & BOOST_SERIALIZATION_NVP (_output);
      ar & BOOST_SERIALIZATION_NVP (_evaluation_context_requested);
    }
  }
}
