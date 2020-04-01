#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>

namespace we
{
  namespace type
  {
    template<class Archive>
      void activity_t::serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_transition);
      ar & BOOST_SERIALIZATION_NVP (_transition_id);
      ar & BOOST_SERIALIZATION_NVP (_input);
      ar & BOOST_SERIALIZATION_NVP (_output);
    }
  }
}
