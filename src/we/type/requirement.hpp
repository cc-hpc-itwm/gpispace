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
      requirement_t ()
        : value_()
        , mandatory_(false)
      {}

      explicit
      requirement_t (std::string arg, const bool _mandatory = false)
        : value_(arg)
        , mandatory_(_mandatory)
      {}

      bool is_mandatory (void) const
      {
        return mandatory_;
      }

      const std::string & value(void) const
      {
        return value_;
      }

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(value_);
        ar & BOOST_SERIALIZATION_NVP(mandatory_);
      }

      std::string value_;
      bool mandatory_;
    };
  }
}
