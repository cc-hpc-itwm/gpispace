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
  enum PortDirection
  {
    PORT_IN
  , PORT_OUT
  , PORT_IN_OUT
  };

  template <typename SignatureType>
  struct port
  {
  public:
    typedef SignatureType sig_type;

    port ()
      : name_("default")
      , direction_(PORT_IN_OUT)
    {}

    template <typename Signature>
    port (const std::string & name, PortDirection direction, const Signature & signature)
      : name_(name)
      , direction_(direction)
      , signature_(signature)
    {}

    const std::string & name() const { return name_; }
    PortDirection direction() const { return direction_; }
    const sig_type & signature() const { return signature_; }

  private:
    std::string name_;
    PortDirection direction_;
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
    os << "{" << (p.direction() == PORT_IN ? "in" : (p.direction() == PORT_OUT ? "out" : "inout")) << ", " << p.name() << ", " << p.signature() << "}";
    return os;
  }
}}

#endif
