// {petry,rahn}@itwm.fhg.de

#ifndef WE_UTIL_CODEC_HPP
#define WE_UTIL_CODEC_HPP 1

#include <sstream>
#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/mgmt/type/activity.hpp>

namespace we
{
  namespace util
  {
    namespace codec
    {
      template<typename T>
      void decode (std::istream& s, T& t)
      {
        boost::archive::text_iarchive ar (s);
        try
        {
          ar >> BOOST_SERIALIZATION_NVP (t);
        }
        catch (boost::archive::archive_exception const &ex)
        {
          throw std::runtime_error
            (std::string ("deserialization error: ") + ex.what ());
        }
      }
      template<typename T>
      void decode (const std::string& s, T& t)
      {
        std::istringstream iss (s);

        decode (iss, t);
      }

      inline we::mgmt::type::activity_t decode (std::istream& s)
      {
        we::mgmt::type::activity_t t;
        decode (s, t);
        return t;
      }
      inline we::mgmt::type::activity_t decode (const std::string& s)
      {
        std::istringstream iss (s);

        return decode (iss);
      }
    };
  }
}

#endif
