// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/serialization/split_free.hpp>

#include <boost/optional.hpp>

namespace boost
{
  namespace serialization
  {
    template<typename Archive, typename T>
      void load (Archive& ar, boost::optional<T>& opt, const unsigned int)
    {
      bool just;
      ar & just;
      if (just)
      {
        T value;
        ar & value;
        opt = value;
      }
      else
      {
        opt = boost::none;
      }
    }
    template<typename Archive, typename T>
      void save (Archive& ar, boost::optional<T> const& opt, const unsigned int)
    {
      bool const just (!!opt);
      ar & just;
      if (just)
      {
        T const& value (opt.get());
        ar & value;
      }
    }

    template<typename Archive, typename T>
      void serialize
      (Archive& ar, boost::optional<T>& opt, const unsigned int version)
    {
      boost::serialization::split_free (ar, opt, version);
    }
  }
}
