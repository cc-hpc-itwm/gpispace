/*
 * =====================================================================================
 *
 *       Filename:  LogLevel.hpp
 *
 *    Description:  defines available severity levels
 *
 *        Version:  1.0
 *        Created:  08/11/2009 05:24:16 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef  FHG_LOG_LOGLEVEL_INC
#define  FHG_LOG_LOGLEVEL_INC

#include <iostream>
#include <string>
#include <stdexcept>

// serialization
#include <boost/serialization/nvp.hpp>

namespace fhg { namespace log {
  class LogLevel {
    public:
      enum Level {
          TRACE
        , DEBUG
        , INFO
        , WARN
        , ERROR
        , FATAL

        // keep the following definitions always up-to-date
        , MIN_LEVEL = TRACE
        , DEF_LEVEL = INFO
        , MAX_LEVEL = FATAL
      };

      LogLevel(Level level = DEF_LEVEL)
        : lvl_(level)
      {
        if (lvl_ < MIN_LEVEL || lvl_ > MAX_LEVEL)
        {
          throw std::runtime_error("the specified log-level is out of range!");
        }
      }

      explicit
      LogLevel(const std::string &level_name);

      const std::string &str() const;

      inline const Level &lvl() const { return lvl_; }
      inline Level &lvl() { return lvl_; }
      operator Level () const { return lvl_; }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int /* version */ )
    {
      ar & BOOST_SERIALIZATION_NVP( lvl_ );
    }

    private:
      Level lvl_;
  };
}}

inline std::ostream &operator<<(std::ostream &os, const fhg::log::LogLevel &lvl)
{
  os << lvl.str();
  return os;
}

#endif   /* ----- #ifndef FHG_LOG_LOGLEVEL_INC  ----- */
