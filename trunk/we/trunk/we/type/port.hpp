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
#include <we/type/id.hpp>
#include <we/type/property.hpp>

namespace we
{
  namespace type {

    enum PortDirection
      {
        PORT_IN
      , PORT_OUT
      , PORT_READ
      , PORT_IN_OUT
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
        default:
          throw std::runtime_error ("STRANGE: unknown PortDirection");
        }
    }

    namespace detail {
      namespace direction
      {
        struct i_port
        {
          static const PortDirection value = PORT_IN;
        };

        struct r_port
        {
          static const PortDirection value = PORT_READ;
        };

        struct o_port
        {
          static const PortDirection value = PORT_OUT;
        };

        struct io_port
        {
          static const PortDirection value = PORT_IN_OUT;
        };

        template <int Dir>
        struct tag {};

        template <>
        struct tag<PORT_IN>
        {
          typedef i_port type;
        };

        template <>
        struct tag<PORT_READ>
        {
          typedef r_port type;
        };

        template <>
        struct tag<PORT_OUT>
        {
          typedef o_port type;
        };

        template <>
        struct tag<PORT_IN_OUT>
        {
          typedef io_port type;
        };
      }
    }
    template < typename SignatureType
             , typename IdType = petri_net::pid_t
             , typename Traits = petri_net::traits::id_traits<IdType>
             >
    struct port
    {
    public:
      typedef SignatureType sig_type;
      typedef IdType pid_type;
      typedef Traits pid_traits;

      typedef detail::direction::i_port I_PORT;
      typedef detail::direction::o_port O_PORT;

      typedef std::string name_type;

      port ()
        : name_("default")
        , direction_(I_PORT::value)
        , associated_place_(pid_traits::invalid())
      {}

      template <typename Signature>
      port (const name_type & name, PortDirection direction, const Signature & signature)
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(pid_traits::invalid())
      {}

      template <typename Signature>
      port ( const name_type & name
           , PortDirection direction
           , const Signature & signature
           , const we::type::property::type & prop
           )
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(pid_traits::invalid())
        , prop_(prop)
      {}

      template <typename Signature, typename PlaceId>
      port (const name_type & name, PortDirection direction, const Signature & signature, const PlaceId place_id)
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(place_id)
      {}

      template <typename Signature, typename PlaceId>
      port ( const name_type & name
           , PortDirection direction
           , const Signature & signature
           , const PlaceId place_id
           , const we::type::property::type prop
           )
        : name_(name)
        , direction_(direction)
        , signature_(signature)
        , associated_place_(place_id)
        , prop_(prop)
      {}

      template <typename Signature, typename Dir>
      static
      port create ( const name_type & name, const Signature & signature )
      {
        return port<Signature, IdType, Traits> ( name, Dir::value, signature );
      }

      template <typename Signature, typename PlaceId, typename Dir>
      static
      port create ( const name_type & name, const Signature & signature, const PlaceId place_id )
      {
        return port<Signature, IdType, Traits> ( name, Dir::value, signature, place_id );
      }

      name_type & name() { return name_; }
      const name_type & name() const { return name_; }

      PortDirection direction() const { return direction_; }
      const sig_type & signature() const { return signature_; }
      const pid_type & associated_place() const { return associated_place_; }
      pid_type & associated_place() { return associated_place_; }
      const we::type::property::type & property() const { return prop_; }

      inline bool is_input (void) const { return direction_ == PORT_IN || direction_ == PORT_IN_OUT || direction_ == PORT_READ; }
      inline bool is_output (void) const { return direction_ == PORT_OUT || direction_ == PORT_IN_OUT; }
      inline bool has_associated_place (void) const { return associated_place_ != pid_traits::invalid(); }
    private:
      name_type name_;
      PortDirection direction_;
      sig_type signature_;
      //! associated to a place within a network, only reasonable for transitions with a subnet
      pid_type associated_place_;
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

    template <typename T, typename I>
    std::ostream & operator << ( std::ostream & os, const port <T, I> & p )
    {
      os << "{port, "
         << p.direction()
         << ", "
         << p.name()
         << ", "
         << p.signature()
         << ", "
        ;

      if (p.associated_place() == port<T,I>::pid_traits::invalid())
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
