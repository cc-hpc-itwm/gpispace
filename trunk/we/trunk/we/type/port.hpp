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
#include <we/type/id.hpp>

namespace we { namespace type {
  enum PortDirection
  {
    PORT_IN
  , PORT_OUT
  , PORT_IN_OUT
  };

  template <typename SignatureType, typename IdType = petri_net::pid_t, typename Traits = petri_net::traits::id_traits<IdType> >
  struct port
  {
  public:
    typedef SignatureType sig_type;
    typedef IdType pid_type;
    typedef Traits pid_traits;

    port ()
      : name_("default")
      , direction_(PORT_IN_OUT)
      , associated_place_(pid_traits::invalid())
    {}

    template <typename Signature>
    port (const std::string & name, PortDirection direction, const Signature & signature)
      : name_(name)
      , direction_(direction)
      , signature_(signature)
      , associated_place_(pid_traits::invalid())
    {}

    template <typename Signature, typename PlaceId>
    port (const std::string & name, PortDirection direction, const Signature & signature, const PlaceId place_id)
      : name_(name)
      , direction_(direction)
      , signature_(signature)
      , associated_place_(place_id)
    {}

    const std::string & name() const { return name_; }
    PortDirection direction() const { return direction_; }
    const sig_type & signature() const { return signature_; }
    const pid_type & associated_place() const { return associated_place_; }

    inline bool is_input (void) const { return direction_ == PORT_IN || direction_ == PORT_IN_OUT; }
    inline bool is_output (void) const { return direction_ == PORT_OUT || direction_ == PORT_IN_OUT; }
  private:
    std::string name_;
    PortDirection direction_;
    sig_type signature_;
    pid_type associated_place_; //! associated to a place within a network, only reasonable for transitions with a subnet

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(name_);
      ar & BOOST_SERIALIZATION_NVP(direction_);
      ar & BOOST_SERIALIZATION_NVP(signature_);
      ar & BOOST_SERIALIZATION_NVP(associated_place_);
    }
  };

  template <typename T, typename I>
  std::ostream & operator << ( std::ostream & os, const port <T, I> & p )
  {
    os << "{port, "
       << (p.direction() == PORT_IN ? "in" : (p.direction() == PORT_OUT ? "out" : "inout"))
       << ", "
       << p.name()
       << ", "
       << p.signature()
       << ", "
       ;

       if (p.associated_place() == port<T,I>::pid_traits::invalid())
       {
          os << "n/a";
       }
       else
       {
          os << p.associated_place();
       }
    os << "}";
    return os;
  }
}}

#endif
