#pragma once

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>

#include <string>

namespace we
{
  namespace type
  {
    struct requirement_t
    {
      requirement_t()
        : _value()
        , _mandatory (false)
      {}

      explicit
      requirement_t (std::string value, const bool mandatory = false)
        : _value (std::move (value))
        , _mandatory (mandatory)
      {}

      bool is_mandatory() const
      {
        return _mandatory;
      }

      std::string const& value() const
      {
        return _value;
      }

    private:
      friend class boost::serialization::access;

      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP (_value);
        ar & BOOST_SERIALIZATION_NVP (_mandatory);
      }

      std::string _value;
      bool _mandatory;
    };
  }
}
