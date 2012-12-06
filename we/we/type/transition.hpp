/*
 * =====================================================================================
 *
 *       Filename:  transition.hpp
 *
 *    Description:  generic transition implementation
 *
 *        Version:  1.0
 *        Created:  04/09/2010 01:21:44 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_TYPE_TRANSITION_HPP
#define WE_TYPE_TRANSITION_HPP 1

#include <we/net.hpp>
#include <we/type/port.hpp>
#include <we/type/module_call.hpp>
#include <we/type/expression.hpp>
#include <we/type/condition.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/id.hpp>
#include <we/type/requirement.hpp>
#include <we/type/token.hpp>

#include <fhg/util/show.hpp>
#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>

namespace we { namespace type {
    namespace exception {
      struct port_already_defined : std::runtime_error
      {
        explicit port_already_defined (const std::string & msg, const std::string & port_name)
          : std::runtime_error (msg)
          , port (port_name)
        {}

        ~port_already_defined () throw ()
        {}

        const std::string port;
      };

      struct port_undefined : std::runtime_error
      {
        explicit port_undefined (const std::string & msg, const std::string & port_name)
          : std::runtime_error (msg)
          , port(port_name)
        {}

        ~port_undefined () throw ()
        {}

        const std::string port;
      };

      template <typename From>
      struct not_connected : std::runtime_error
      {
        typedef From from_type;

        explicit not_connected(const std::string & msg, const from_type from_)
          : std::runtime_error (msg)
          , from(from_)
        {}

        ~not_connected () throw ()
        {}

        const from_type from;
      };

      template <typename From, typename To>
      struct already_connected : std::runtime_error
      {
        typedef From from_type;
        typedef To to_type;

        explicit already_connected(const std::string & msg, const from_type from_, const to_type to_)
          : std::runtime_error (msg)
          , from(from_)
          , to(to_)
        {}

        ~already_connected () throw ()
        {}

        const from_type from;
        const to_type to;
      };
    }

    namespace detail {
      template <typename Transition>
      struct port_adder
      {
        explicit port_adder (Transition & t)
          : transition_(t)
        {}

        template <typename SignatureType, typename Direction>
        port_adder<Transition> &
        operator()
          ( const std::string & name
          , SignatureType const & signature
          , Direction direction
          , const we::type::property::type & prop = we::type::property::type()
          )
        {
          transition_.add_port ( name
                               , signature
                               , direction
                               , prop
                               );
          return *this;
        }

        template <typename SignatureType, typename Direction, typename PlaceId>
        port_adder<Transition> &
        operator ()
          ( const std::string & name
          , SignatureType const & signature
          , Direction direction
          , const PlaceId associated_place
          , const we::type::property::type & prop = we::type::property::type()
          )
        {
          transition_.add_port ( name
                               , signature
                               , direction
                               , associated_place
                               , prop
                               );
          return *this;
        }

      private:
        Transition & transition_;
      };

      template <typename Transition, typename From, typename To>
      struct connection_adder
      {
        typedef From from_type;
        typedef To to_type;

        explicit connection_adder (Transition & t)
          : transition_(t)
        {}

        connection_adder<Transition, from_type, to_type> & operator()
          ( const from_type outer
          , const std::string & name
          , const we::type::property::type & prop = we::type::property::type()
          )
        {
          transition_.connect_outer_to_inner
            (outer, transition_.input_port_by_name (name), prop);
          return *this;
        }

        connection_adder<Transition, from_type, to_type> & operator()
          ( const std::string & name
          , const from_type outer
          , const we::type::property::type & prop = we::type::property::type()
          )
        {
          transition_.connect_inner_to_outer
            (transition_.output_port_by_name (name), outer, prop);
          return *this;
        }
      private:
        Transition & transition_;
      };

      template <typename Transition, typename Pid>
      std::string translate_place_to_port_name (const Transition & trans, const Pid pid)
      {
        return trans.get_port (trans.outer_to_inner (pid)).name();
      }

      template <typename Transition, typename PortId>
      std::string translate_port_to_name (const Transition & trans, const PortId port_id)
      {
        return trans.get_port (port_id).name();
      }

      template <typename Transition>
      typename Transition::port_id_t translate_name_to_output_port (const Transition & trans, const std::string & name)
      {
        return trans.output_port_by_name (name);
      }

      template <typename Transition>
      typename Transition::port_id_t translate_name_to_input_port (const Transition & trans, const std::string & name)
      {
        return trans.input_port_by_name (name);
      }
    }

    namespace detail
    {
      template <typename T>
      struct is_internal
      {
        static const bool value = true;
      };

      template <>
      struct is_internal<expression_t>
      {
        static const bool value = true;
      };

      template <>
      struct is_internal<module_call_t>
      {
        static const bool value = false;
      };

      template <typename T>
      struct condition
      {
        explicit condition (T const & _expr)
          : expr(_expr)
        { }

        operator T const & () const
        {
          return expr;
        }

        const T expr;
      };

      template <typename E, typename P>
      struct preparsed_condition
      {
        // should correspond!
        explicit preparsed_condition (E const & _expr, P const & _parser)
          : expr(_expr)
          , parser(_parser)
        { }

        operator E const & () const
        {
          return expr;
        }

        operator P const & () const
        {
          return parser;
        }

        const E expr;
        const P parser;
      };
    }

    struct transition_t
    {
      typedef unsigned int edge_type;

      typedef module_call_t mod_type;
      typedef expression_t expr_type;
      typedef transition_t this_type;
      typedef petri_net::net<this_type> net_type;
      typedef detail::condition<std::string> cond_type;
      typedef detail::preparsed_condition< std::string
                                         , condition::type::parser_t
                                         > preparsed_cond_type;
      typedef boost::variant< mod_type
                            , expr_type
                            , boost::recursive_wrapper<net_type>
                            > data_type;

      typedef petri_net::pid_t pid_t;
      typedef pid_t port_id_t;

      typedef std::pair< port_id_t
                       , we::type::property::type
                       > port_id_with_prop_t;
      typedef std::pair< pid_t
                       , we::type::property::type
                       > pid_with_prop_t;
      typedef boost::unordered_map<pid_t, port_id_with_prop_t> outer_to_inner_t;
      typedef boost::unordered_map<port_id_t, pid_with_prop_t> inner_to_outer_t;

      typedef signature::type signature_type;
      typedef port<signature_type> port_t;
      typedef boost::unordered_map<port_id_t, port_t> port_map_t;
      typedef port_map_t::const_iterator const_iterator;
      typedef port_map_t::iterator port_iterator;
      typedef boost::unordered_set<port_t::name_type> port_names_t;

      typedef we::type::requirement_t<std::string> requirement_t;
      typedef std::list<requirement_t> requirements_t;

      const static bool internal = true;
      const static bool external = false;

      transition_t ()
        : name_ ("unknown")
        , condition_( "true"
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (detail::is_internal<Type>::value)
        , condition_( "true"
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , cond_type const & _condition
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (detail::is_internal<Type>::value)
        , condition_( _condition
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , preparsed_cond_type const & _condition
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (detail::is_internal<Type>::value)
        , condition_( _condition
                    , _condition
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , bool intern
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_( "true"
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , cond_type const & _condition
                   , bool intern
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_( _condition
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , preparsed_cond_type const & _condition
                   , bool intern
                   , const we::type::property::type & prop
                   = we::type::property::type()
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_( _condition
                    , _condition
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
        , prop_(prop)
      { }

      transition_t (const this_type &other)
        : name_(other.name_)
        , data_(other.data_)
        , internal_ (other.internal_)
        , condition_( other.condition_.expression()
                    , boost::bind
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , outer_to_inner_(other.outer_to_inner_)
        , inner_to_outer_(other.inner_to_outer_)
        , ports_(other.ports_)
        , port_id_counter_(other.port_id_counter_)
        , prop_ (other.prop_)
        , m_requirements (other.m_requirements)
      { }

      template <typename Choices>
      bool condition (Choices & choices) const
      {
        return condition_ (choices);
      }

      const condition::type & condition() const
      {
        return condition_;
      }

      const std::string & name (void) const
      {
        return name_;
      }

      void set_name (const std::string &name)
      {
        name_ = name;
      }

      bool is_internal (void) const
      {
        return internal_;
      }

      void set_internal(bool internal)
      {
        internal_ = internal;
      }

      const data_type & data (void) const
      {
        return data_;
      }

      data_type & data (void)
      {
        return data_;
      }

      requirements_t const & requirements (void) const
      {
        return m_requirements;
      }

      this_type & operator=(const this_type & other)
      {
        if (this != &other)
        {
          name_ = other.name_;
          internal_ = other.internal_;
          outer_to_inner_ = other.outer_to_inner_;
          inner_to_outer_ = other.inner_to_outer_;
          ports_ = other.ports_;
          data_ = other.data_;
          port_id_counter_ = other.port_id_counter_;
          condition_ = condition::type
            ( other.condition_.expression()
            , boost::bind
              ( &detail::translate_place_to_port_name<this_type, pid_t>
              , boost::ref(*this)
              , _1
              )
            );
          prop_ = other.prop_;
          m_requirements = other.m_requirements;
        }
        return *this;
      }

      ~transition_t () { }

      template <typename Outer, typename Inner>
      void connect_outer_to_inner ( const Outer outer
                                  , const Inner inner
                                  , const we::type::property::type & prop
                                  )
      {
        if (outer_to_inner_.find (outer) != outer_to_inner_.end())
        {
          throw exception::already_connected<Outer, Inner>("already connected", outer, inner);
        }
        else
        {
          outer_to_inner_.insert
            (outer_to_inner_t::value_type (outer, std::make_pair(inner, prop)));
        }
      }

      template <typename Inner, typename Outer>
      void connect_inner_to_outer ( const Inner inner
                                  , const Outer outer
                                  , const we::type::property::type & prop
                                  )
      {
        if (inner_to_outer_.find (inner) != inner_to_outer_.end())
        {
          throw exception::already_connected<Inner, Outer>("already connected", inner, outer);
        }
        else
        {
          inner_to_outer_.insert
            (inner_to_outer_t::value_type (inner, std::make_pair(outer, prop)));
        }
      }

      template <typename Outer>
      void disconnect_outer_from_inner ( const Outer outer
                                       )
      {
      	outer_to_inner_t::iterator i = outer_to_inner_.find (outer);
        if (i == outer_to_inner_.end())
        {
          throw exception::not_connected<Outer>("already disconnected", outer);
        }
        else
        {
          outer_to_inner_.erase(i);
        }
      }

      template <typename Inner>
      void disconnect_inner_from_outer ( const Inner inner
                                       )
      {
      	inner_to_outer_t::iterator i = inner_to_outer_.find (inner);
        if (i == inner_to_outer_.end())
        {
          throw exception::not_connected<Inner>("already disconnected", inner);
        }
        else
        {
          inner_to_outer_.erase(i);
        }
      }

      template <typename Inner, typename Outer>
      void re_connect_inner_to_outer ( const Inner inner
                                     , const Outer outer
                                     , const we::type::property::type & prop
                                     )
      {
        inner_to_outer_.erase (inner);

        connect_inner_to_outer (inner, outer, prop);
      }

      template <typename Outer, typename Inner>
      void re_connect_outer_to_inner ( const Outer outer_old
                                     , const Outer outer_new
                                     , const Inner inner
                                     , const we::type::property::type & prop
                                     )
      {
        outer_to_inner_.erase (outer_old);

        connect_outer_to_inner (outer_new, inner, prop);
      }

      template <typename Outer>
      typename outer_to_inner_t::mapped_type
      gen_outer_to_inner (Outer outer) const
      {
        try
        {
          return outer_to_inner_.at(outer);
        } catch (const std::out_of_range &)
        {
          throw exception::not_connected<Outer> ("trans: " + name() + ": not connected: " + fhg::util::show(outer), outer);
        }
      }

      template <typename Outer>
      port_id_t outer_to_inner (Outer outer) const
      {
        return gen_outer_to_inner (outer).first;
      }

      template <typename Outer>
      const we::type::property::type & outer_to_inner_prop (Outer outer) const
      {
        return gen_outer_to_inner (outer).second;
      }

      template <typename Inner>
      typename inner_to_outer_t::mapped_type
      gen_inner_to_outer (Inner inner) const
      {
        try
        {
          return inner_to_outer_.at(inner);
        } catch (const std::out_of_range &)
        {
          throw exception::not_connected<Inner> ("trans: " + name() + ": port not connected: " + fhg::util::show(inner), inner);
        }
      }

      template <typename Inner>
      pid_t inner_to_outer (Inner inner) const
      {
        return gen_inner_to_outer (inner).first;
      }

      template <typename Inner>
      const we::type::property::type & inner_to_outer_prop (Inner inner) const
      {
        return gen_inner_to_outer (inner).second;
      }

      inner_to_outer_t::const_iterator
      inner_to_outer_begin (void) const
      {
        return inner_to_outer_.begin();
      }

      inner_to_outer_t::const_iterator
      inner_to_outer_end (void) const
      {
        return inner_to_outer_.end();
      }

      outer_to_inner_t::const_iterator
      outer_to_inner_begin (void) const
      {
        return outer_to_inner_.begin();
      }

      outer_to_inner_t::const_iterator
      outer_to_inner_end (void) const
      {
        return outer_to_inner_.end();
      }

      detail::connection_adder<this_type, pid_t, port_id_t> add_connections()
      {
        return detail::connection_adder<this_type, pid_t, port_id_t>(*this);
      }

      detail::port_adder<this_type> add_ports()
      {
        return detail::port_adder<this_type>(*this);
      }

      // UNSAFE: does not check for already existing port, use with care
      port_id_t UNSAFE_add_port (const port_t & port)
      {
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));

        return port_id;
      }

      void erase_port (const port_id_t & port_id)
      {
        ports_.erase (port_id);
        inner_to_outer_.erase (port_id);
      }

      template <typename SignatureType, typename Direction>
      void add_port ( const std::string & name
                    , SignatureType const & sig
                    , Direction direction
                    , const we::type::property::type & prop
                    )
      {
        switch (direction)
          {
          case PORT_IN: this->add_input_port (name, sig, prop); break;
          case PORT_OUT: this->add_output_port (name, sig, prop); break;
          case PORT_READ: this->add_read_port (name, sig, prop); break;
          case PORT_IN_OUT: this->add_input_output_port (name, sig, prop);
            break;
          case PORT_TUNNEL: this->add_tunnel (name, sig, prop); break;
          default: throw std::runtime_error ("STRANGE: unknown port direction");
          }
      }

      template <typename SignatureType, typename Direction, typename PlaceId>
      void add_port ( const std::string & name
                    , SignatureType const & sig
                    , const Direction direction
                    , const PlaceId pid
                    , const we::type::property::type & prop
                    )
      {
        switch (direction)
          {
          case PORT_IN: this->add_input_port (name, sig, pid, prop); break;
          case PORT_OUT: this->add_output_port (name, sig, pid, prop); break;
          case PORT_READ: this->add_read_port (name, sig, pid, prop); break;
          case PORT_IN_OUT: this->add_input_output_port (name, sig, pid, prop);
            break;
          case PORT_TUNNEL: this->add_tunnel (name, sig, pid, prop); break;
          default: throw std::runtime_error ("STRANGE: unknown port direction");
          }
      }

      template <typename SignatureType>
      pid_t add_input_port ( const std::string & port_name
                           , const SignatureType & signature
                           , const we::type::property::type & prop
                           )
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": input port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_IN, signature, prop);
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_input_port ( const std::string & port_name
                           , const SignatureType & signature
                           , const PlaceId associated_place
                           , const we::type::property::type & prop
                           )
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined
              ("trans: " + name() + ": input port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_IN, signature, associated_place, prop);
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      pid_t add_read_port ( const std::string & port_name
                          , const SignatureType & signature
                          , const we::type::property::type & prop
                          )
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": read port " + port_name + " already defined: ", port_name);
          }
        }
        port_t port (port_name, PORT_READ, signature, prop);
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_read_port ( const std::string & port_name
                          , const SignatureType & signature
                          , const PlaceId associated_place
                          , const we::type::property::type & prop
                          )
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": read port " + port_name + " already defined: ", port_name);
          }
        }
        port_t port (port_name, PORT_READ, signature, associated_place, prop);
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      pid_t add_output_port ( const std::string & port_name
                            , const SignatureType & signature
                            , const we::type::property::type & prop
                            )
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_output()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": output port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_OUT, signature, prop);
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_tunnel ( const std::string & port_name
                       , const SignatureType & signature
                       , const PlaceId associated_place
                       , const we::type::property::type & prop
                       )
      {
        BOOST_FOREACH (const port_map_t::value_type& p, ports_)
          {
            if (p.second.is_tunnel() && p.second.name() == port_name)
              {
                throw exception::port_already_defined ("trans: " + name() + ": tunnel " + port_name + " already defined", port_name);
              }
          }

        const port_t port (port_name, PORT_TUNNEL, signature, associated_place, prop);
        const port_id_t port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      pid_t add_tunnel ( const std::string & port_name
                       , const SignatureType & signature
                       , const we::type::property::type & prop
                       )
      {
        BOOST_FOREACH (const port_map_t::value_type& p, ports_)
          {
            if (p.second.is_tunnel() && p.second.name() == port_name)
              {
                throw exception::port_already_defined("trans: " + name() + ": tunnel " + port_name + " already defined", port_name);
              }
          }
        const port_t port (port_name, PORT_OUT, signature, prop);
        const port_id_t port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_output_port ( const std::string & port_name
                            , const SignatureType & signature
                            , const PlaceId associated_place
                            , const we::type::property::type & prop
                            )
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_output()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": output port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_OUT, signature, associated_place, prop);
        port_id_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }



      template <typename SignatureType>
      void add_input_output_port ( const std::string & port_name
                                 , const SignatureType & signature
                                 , const we::type::property::type & prop
                                 )
      {
        try
        {
          input_port_by_name (port_name);
        }
        catch (const exception::port_undefined &)
        {
          try
          {
            output_port_by_name (port_name);
          }
          catch (const exception::port_undefined &)
          {
            add_input_port (port_name, signature, prop);
            add_output_port (port_name, signature, prop);
          }
        }
      }

      template <typename SignatureType, typename PlaceId>
      void add_input_output_port ( const std::string & port_name
                                 , const SignatureType & signature
                                 , const PlaceId associated_place
                                 , const we::type::property::type & prop
                                 )
      {
        try
        {
          input_port_by_name (port_name);
        }
        catch (const exception::port_undefined &)
        {
          try
          {
            output_port_by_name (port_name);
          }
          catch (const exception::port_undefined &)
          {
            add_input_port (port_name, signature, associated_place, prop);
            add_output_port (port_name, signature, associated_place, prop);
          }
        }
      }

      port_id_t input_port_by_name (const std::string & port_name) const
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            return p->first;
          }
        }
        throw exception::port_undefined("trans: "+name()+": input port not defined:"+port_name, port_name);
      }

      port_id_t output_port_by_name (const std::string & port_name) const
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_output()) && p->second.name() == port_name)
          {
            return p->first;
          }
        }
        throw exception::port_undefined("trans: "+name()+": output port not defined:"+port_name, port_name);
      }

      port_id_with_prop_t input_port_by_pid (const pid_t & pid) const
      {
        for ( outer_to_inner_t::const_iterator p (outer_to_inner_.begin())
            ; p != outer_to_inner_.end()
            ; ++p
            )
        {
          if (p->first == pid)
          {
            return p->second;
          }
        }

        throw exception::not_connected<pid_t>("trans: "+name()+": input port not connected by pid: "+ fhg::util::show (pid), pid);
      }

      pid_t input_pid_by_port_id (const port_id_t & port_id) const
      {
        for ( outer_to_inner_t::const_iterator p (outer_to_inner_.begin())
            ; p != outer_to_inner_.end()
            ; ++p
            )
        {
          if (p->second.first == port_id)
          {
            return p->first;
          }
        }

        throw exception::not_connected<port_id_t>("trans: "+name()+": pid not connected by port_id: "+ fhg::util::show (port_id), port_id);
      }

      port_id_with_prop_t output_port_by_pid (const pid_t & pid) const
      {
        for ( inner_to_outer_t::const_iterator p (inner_to_outer_.begin())
            ; p != inner_to_outer_.end()
            ; ++p
            )
        {
          if (p->second.first == pid)
          {
            return std::make_pair (p->first, p->second.second);
          }
        }

        throw exception::not_connected<pid_t>("trans: "+name()+": output port not connected by pid: "+ fhg::util::show (pid), pid);
      }

      template <typename PortId>
      const port_t & get_port (const PortId port_id) const
      {
        try
        {
          return ports_.at (port_id);
        }
        catch (const std::out_of_range &)
        {
          const std::string port_name (fhg::util::show (port_id) );
          throw exception::port_undefined("trans: "+name()+": port not defined:"+port_name, port_name);
        }
      }

      template <typename PortId>
      port_t & get_port (const PortId port_id)
      {
        try
        {
          return ports_[port_id];
        }
        catch (const std::out_of_range &)
        {
          const std::string port_name (fhg::util::show (port_id) );
          throw exception::port_undefined("trans: "+name()+": port not defined:"+port_name, port_name);
        }
      }

      const port_t & get_port_by_associated_pid (const pid_t & pid) const
      {
        for ( const_iterator port (ports_.begin())
            ; port != ports_.end()
            ; ++port
            )
          {
            if (port->second.associated_place() == pid)
              {
                return port->second;
              }
          }
        throw exception::not_connected<pid_t>("trans: "+name()+": port not associated with:"+fhg::util::show(pid), pid);
      }

      // UNSAFE: does not check for multiple connections! Use with care!
      void UNSAFE_re_associate_port ( const pid_t & pid_old
                                    , const pid_t & pid_new
                                    )
      {
        for ( port_iterator port (ports_.begin())
            ; port != ports_.end()
            ; ++port
            )
          {
            if (port->second.associated_place() == pid_old)
              {
                port->second.associated_place() = pid_new;

                return;
              }
          }
        throw exception::not_connected<pid_t>("trans: "+name()+": during re_connect port not associated with:"+fhg::util::show(pid_old), pid_old);
      }

      // TODO implement port accessor iterator
      const_iterator ports_begin() const { return ports_.begin(); }
      const_iterator ports_end() const { return ports_.end(); }

      port_iterator ports_begin() { return ports_.begin(); }
      port_iterator ports_end() { return ports_.end(); }

      const we::type::property::type & prop (void) const { return prop_; }

      const port_map_t & ports () const { return ports_; }

      port_names_t port_names() const
      {
        port_names_t names;

        for (const_iterator port (ports_begin()); port != ports_end(); ++port)
          {
            names.insert (port->second.name());
          }

        return names;
      }

      port_names_t port_names (const PortDirection & d) const
      {
        port_names_t names;

        for (const_iterator port (ports_begin()); port != ports_end(); ++port)
          {
            if (d == port->second.direction())
              {
                names.insert (port->second.name());
              }
          }

        return names;
      }

      void add_requirement ( requirement_t const & r )
      {
        m_requirements.push_back (r);
      }

      void del_requirement ( requirement_t const & r )
      {
        m_requirements.remove (r);
      }

    private:
      std::string name_;
      data_type data_;
      bool internal_;
      condition::type condition_;

      outer_to_inner_t outer_to_inner_;
      inner_to_outer_t inner_to_outer_;
      port_map_t ports_;
      port_id_t port_id_counter_;

      we::type::property::type prop_;

      requirements_t m_requirements;

    private:
      friend std::ostream & operator<< ( std::ostream &
                                       , const transition_t &
                                       );

      friend class boost::serialization::access;
      template <typename Archive>
      void save(Archive & ar, const unsigned int version) const
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        ar & boost::serialization::make_nvp("condition", condition_.expression());
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);
        ar & BOOST_SERIALIZATION_NVP(m_requirements);
      }

      template <typename Archive>
      void load(Archive & ar, const unsigned int version)
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        std::string cond_expr;
        ar & boost::serialization::make_nvp("condition", cond_expr);
        condition_ = condition::type ( cond_expr
                                     , boost::bind
                                       ( &detail::translate_place_to_port_name<this_type, pid_t>
                                       , boost::ref(*this)
                                       , _1
                                       )
                                     );
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);

        if (version > 0)
        {
          ar & BOOST_SERIALIZATION_NVP(m_requirements);
        }
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    inline bool operator==(const transition_t & a, const transition_t & b)
    {
      return a.name() == b.name();
    }
    inline std::size_t hash_value(transition_t const & t)
    {
      boost::hash<std::string> hasher;
      return hasher(t.name());
    }

    namespace dump
    {
      inline void dump ( xml_util::xmlstream & s
                       , const transition_t & t
                       )
      {
        typedef transition_t trans_t;

        s.open ("defun");
        s.attr ("name", t.name());

        for ( trans_t::port_map_t::const_iterator p (t.ports().begin())
            ; p != t.ports().end()
            ; ++p
            )
          {
            ::we::type::dump::dump (s, p->second);
          }

        s.open ("condition");
        s.content (t.condition());
        s.close();

        s.close();
      }
    }

    namespace detail
    {
      class transition_visitor_show : public boost::static_visitor<std::string>
      {
      public:
        std::string operator () (const expression_t & expr) const
        {
          return "{expr, " + fhg::util::show (expr) + "}";
        }

        std::string operator () (const module_call_t & mod_call) const
        {
          return "{mod, " + fhg::util::show (mod_call) + "}";
        }

        std::string operator () ( const petri_net::net< transition_t
                                                      > & net
                                ) const
        {
          return std::string("{net, ") + fhg::util::show(net) + "}";
        }
      };
    }

    inline std::ostream & operator<< ( std::ostream & s
                                     , const transition_t & t
                                     )
    {
      typedef transition_t trans_t;
      s << "{";
      s << "trans";
      s << ", ";
      s << t.name();
      s << ", ";
      s << (t.is_internal() ? "intern" : "extern");
      s << ", ";
      s << boost::apply_visitor (detail::transition_visitor_show(), t.data());
      s << ", {cond, " << t.condition() << "}";
      s << ", {ports, ";
      s << "[";
      for ( trans_t::port_map_t::const_iterator p (t.ports_.begin())
          ; p != t.ports_.end()
          ; ++p
          )
      {
        if (p != t.ports_.begin())
          s << ", ";

        s << "(";
        s << p->first;
        s << ", ";
        s << p->second;
        s << ")";
      }

      s << "]";
      s << "}";

      s << "}";
      return s;
    }

    inline std::ostream & operator << ( std::ostream & s
                                      , const petri_net::net<transition_t> & n
                                      )
    {
      typedef petri_net::net<transition_t> pnet_t;

      for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
      {
        s << "[" << n.get_place (*p) << ":";

        typedef boost::unordered_map<token::type, size_t> token_cnt_t;
        token_cnt_t token;
        for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        {
          token[*tp]++;
        }

        for (token_cnt_t::const_iterator t (token.begin()); t != token.end(); ++t)
        {
          if (t->second > 1)
          {
            s << " " << t->second << "x " << t->first;
          }
          else
          {
            s << " " << t->first;
          }
        }
        s << "]";
      }

      for (pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
      {
        s << "/";
        s << n.get_transition (*t);
        s << "/";
      }

      return s;
    }

    // ********************************************************************* //

    namespace content
    {
      enum kind
        { expression
        , modcall
        , subnet
        };

      class visitor : public boost::static_visitor<kind>
      {
      public:
        kind operator () (const expression_t &) const
        {
          return expression;
        }

        kind operator () (const module_call_t &) const
        {
          return modcall;
        }

        kind operator () (const petri_net::net<transition_t> &) const
        {
          return subnet;
        }
      };

      inline bool is_expression (const transition_t & t)
      {
        return boost::apply_visitor (visitor(), t.data()) == expression;
      }

      inline bool is_subnet (const transition_t & t)
      {
        return boost::apply_visitor (visitor(), t.data()) == subnet;
      }
    }
  }
}

//! \todo is this used somewhere?
namespace boost {
  namespace serialization {
    template<>
    struct version<we::type::transition_t>
    {
      typedef mpl::int_<1> type;
      typedef mpl::integral_c_tag tag;
      BOOST_STATIC_CONSTANT(int, value = version::type::value);
    };
  }
}

#endif
