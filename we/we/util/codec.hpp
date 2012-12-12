/*
 * =====================================================================================
 *
 *       Filename:  codec.hpp
 *
 *    Description:  encode and decode functions
 *
 *        Version:  1.0
 *        Created:  04/19/2010 09:58:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_UTIL_CODEC_HPP
#define WE_UTIL_CODEC_HPP 1

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace we
{
  namespace util
  {
    struct base_codec
    {
      template <typename T>
      static void encode (std::ostream & s, const T & t)
      {
        boost::archive::text_oarchive ar (s);
        ar << BOOST_SERIALIZATION_NVP (t);
      }

      template <typename T>
      static std::string encode (const T & t)
      {
        std::ostringstream oss;
        encode (oss, t);
        return oss.str();
      }

      template <typename T>
      static void decode (std::istream & s, T & t)
      {
        boost::archive::text_iarchive ar (s);
        ar >> BOOST_SERIALIZATION_NVP (t);
      }

      template <typename T>
      static void decode (const std::string & s, T & t)
      {
        std::istringstream iss (s);
        decode (iss, t);
      }

      template <typename T>
      static T decode (std::istream & s)
      {
        T t;
        decode (s, t);
        return t;
      }

      template <typename T>
      static T decode (const std::string & s)
      {
        T t;
        decode (s, t);
        return t;
      }
    };

    typedef base_code text_codec;
    typedef base_codec codec;
  }
}

#endif
