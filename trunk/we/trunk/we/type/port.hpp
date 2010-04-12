/*
 * =====================================================================================
 *
 *       Filename:  port.hpp
 *
 *    Description:  port description of a transition
 *
 *        Version:  1.0
 *        Created:  04/12/2010 11:48:30 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_TYPE_PORT_HPP
#define WE_TYPE_PORT_HPP 1

#include <string>
#include <ostream>
#include <boost/serialization/nvp.hpp>

namespace we { namespace type {
  template <typename SignatureType>
  struct port
  {
  public:
    typedef SignatureType sig_type;

    enum Direction
    {
      IN
    , OUT
    , IN_OUT
    };

    port ()
      : name_("default")
      , direction_(IN_OUT)
    {}

    port (const std::string & name, Direction direction, const sig_type & signature)
      : name_(name)
      , direction_(direction)
      , signature_(signature)
    {}

    const std::string & name() const { return name_; }
    Direction direction() const { return direction_; }
    const sig_type & signature() const { return signature_; }

  private:
    std::string name_;
    Direction direction_;
    sig_type signature_;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(name_);
      ar & BOOST_SERIALIZATION_NVP(direction_);
      ar & BOOST_SERIALIZATION_NVP(signature_);
    }
  };

  template <typename T>
  std::ostream & operator << ( std::ostream & os, const port <T> & p )
  {
    os << "{" << (p.direction() == port<T>::IN ? "in" : (p.direction() == port<T>::OUT ? "out" : "inout")) << ", " << p.name() << ", " << p.signature() << "}";
    return os;
  }
}}

#endif
