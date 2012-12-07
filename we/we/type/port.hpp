/*
 * =====================================================================================
 *
 *       filename:  port.hpp
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
#include <boost/optional.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <fhg/util/xml.hpp>

#include <limits>

namespace xml_util = ::fhg::util::xml;

namespace we
{
  namespace type {

    enum PortDirection
      {
        PORT_IN
      , PORT_OUT
      , PORT_READ
      , PORT_IN_OUT
      , PORT_TUNNEL
      };

    inline
    std::ostream & operator << (std::ostream & s, const PortDirection & p)
    {
      switch (p)
        {
        case PORT_IN: return s << "port-in";
        case PORT_OUT: return s << "port-out";
        case PORT_READ: return s << "port-read";
        case PORT_IN_OUT: return s << "port-inout";
        case PORT_TUNNEL: return s << "port-tunnel";
        default:
          throw std::runtime_error ("STRANGE: unknown PortDirection");
        }
    }

    struct port_t
    {
    public:
      typedef std::string name_type;

      port_t ()
        : name_("default")
        , direction_(PORT_IN)
        , associated_place_(petri_net::pid_invalid())
      {}

      port_t ( const name_type & name
           , PortDirection direction
           , const signature::type& signature
           )
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(petri_net::pid_invalid())
      {}

      port_t ( const name_type & name
           , PortDirection direction
           , const signature::type& signature
           , const we::type::property::type & prop
           )
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(petri_net::pid_invalid())
        , prop_(prop)
      {}

      template <typename PlaceId>
      port_t ( const name_type & name
           , PortDirection direction
           , const signature::type& signature
           , const PlaceId place_id
           )
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(place_id)
      {}

      template <typename PlaceId>
      port_t ( const name_type & name
           , PortDirection direction
           , const signature::type& signature
           , const PlaceId place_id
           , const we::type::property::type prop
           )
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(place_id)
        , prop_(prop)
      {}

      name_type & name() { return name_; }
      const name_type & name() const { return name_; }

      PortDirection direction() const { return direction_; }
      const signature::type& signature() const { return signature_; }
      const petri_net::pid_t& associated_place() const { return associated_place_; }
      petri_net::pid_t& associated_place() { return associated_place_; }
      const we::type::property::type & property() const { return prop_; }

      inline bool is_input (void) const { return direction_ == PORT_IN || direction_ == PORT_IN_OUT || direction_ == PORT_READ; }
      inline bool is_output (void) const { return direction_ == PORT_OUT || direction_ == PORT_IN_OUT; }
      inline bool is_tunnel (void) const { return direction_ == PORT_TUNNEL; }
      inline bool has_associated_place (void) const { return associated_place_ != std::numeric_limits<petri_net::pid_t>::max(); }
    private:
      name_type name_;
      PortDirection direction_;
      signature::type signature_;
      //! associated to a place within a network, only reasonable for transitions with a subnet
      petri_net::pid_t associated_place_;
      we::type::property::type prop_;

      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(direction_);
        ar & BOOST_SERIALIZATION_NVP(signature_);
        ar & BOOST_SERIALIZATION_NVP(associated_place_);
        ar & BOOST_SERIALIZATION_NVP(prop_);
      }
    };

    inline std::ostream & operator << ( std::ostream & os, const port_t& p )
    {
      os << "{port, "
         << p.direction()
         << ", "
         << p.name()
         << ", "
         << p.signature()
         << ", "
        ;

      if (p.associated_place() == petri_net::pid_invalid())
      {
        os << "not associated";
      }
      else
      {
        os << "associated with place " << p.associated_place();
      }
      os << "}";
      return os;
    }
  }
}

#endif
