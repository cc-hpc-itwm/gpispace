// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_SERIALIZATION_PATH_HPP
#define FHG_UTIL_BOOST_SERIALIZATION_PATH_HPP

#include <boost/filesystem/path.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load (Archive& ar, boost::filesystem::path& path, const unsigned int)
    {
      std::string data;
      ar & data;
      path = boost::filesystem::path (data);
    }
    template<typename Archive>
      void save (Archive& ar, boost::filesystem::path const& path, const unsigned int)
    {
      ar & path.string();
    }

    template<typename Archive>
      void serialize
      (Archive& ar, boost::filesystem::path& path, const unsigned int version)
    {
      boost::serialization::split_free (ar, path, version);
    }
  }
}

#endif
