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
#include <we/util/show.hpp>
#include <we/util/warnings.hpp>
#include <we/type/port.hpp>
#include <we/type/module_call.hpp>
#include <we/type/expression.hpp>
#include <we/type/condition.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/id.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/nvp.hpp>

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

    template <typename Place, typename Edge, typename Token>
    struct transition_t
    {
      typedef Place place_type;
      typedef Edge edge_type;
      typedef Token token_type;

      typedef module_call_t mod_type;
      typedef expression_t expr_type;
      typedef transition_t<Place, Edge, Token> this_type;
      typedef petri_net::net<Place, this_type, Edge, Token> net_type;
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
      typedef boost::unordered_map<pid_t, port_t> port_map_t;
      typedef typename port_map_t::const_iterator const_iterator;
      typedef typename port_map_t::iterator port_iterator;

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

      transition_t & operator=(const transition_t & other)
      {
        if (this != &other)
        {
          name_ = other.name_;
          internal_ = other.internal_;
          outer_to_inner_ = other.outer_to_inner_;
          inner_to_outer_ = other.inner_to_outer_;
          ports_ = other.ports_;
          data_ = other.data_;
          condition_ = condition::type
            ( other.condition_.expression()
            , boost::bind
              ( &detail::translate_place_to_port_name<this_type, pid_t>
              , boost::ref(*this)
              , _1
              )
            );
          prop_ = other.prop_;
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
          throw exception::not_connected<Outer> ("trans: " + name() + ": not connected: " + ::util::show(outer), outer);
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
          throw exception::not_connected<Inner> ("trans: " + name() + ": port not connected: " + ::util::show(inner), inner);
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

      typename inner_to_outer_t::const_iterator
      inner_to_outer_begin (void) const
      {
        return inner_to_outer_.begin();
      }

      typename inner_to_outer_t::const_iterator
      inner_to_outer_end (void) const
      {
        return inner_to_outer_.end();
      }

      typename outer_to_inner_t::const_iterator
      outer_to_inner_begin (void) const
      {
        return outer_to_inner_.begin();
      }

      typename outer_to_inner_t::const_iterator
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

      template <typename SignatureType, typename Direction>
      void add_port ( const std::string & port_name
                    , SignatureType const & signature
                    , Direction direction
                    , const we::type::property::type & prop
                    )
      {
        if (direction == PORT_IN)
          this->add_input_port (port_name, signature, prop);
        if (direction == PORT_OUT)
          this->add_output_port (port_name, signature, prop);
        if (direction == PORT_READ)
          this->add_read_port (port_name, signature, prop);
        else
          this->add_input_output_port (port_name, signature, prop);
      }

      template <typename SignatureType, typename Direction, typename PlaceId>
      void add_port ( const std::string & name
                    , SignatureType const & signature
                    , const Direction direction
                    , const PlaceId associated_place
                    , const we::type::property::type & prop
                    )
      {
        if (direction == PORT_IN)
          this->add_input_port ( name
                               , signature
                               , associated_place
                               , prop
                               );
        if (direction == PORT_OUT)
          this->add_output_port ( name
                                , signature
                                , associated_place
                                , prop
                                );
        if (direction == PORT_READ)
          this->add_read_port ( name
                              , signature
                              , associated_place
                              , prop
                              );
        else
          this->add_input_output_port ( name
                                      , signature
                                      , associated_place
                                      , prop
                                      );
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
        pid_t port_id = port_id_counter_++;

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
        pid_t port_id = port_id_counter_++;

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
        pid_t port_id = port_id_counter_++;

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
        pid_t port_id = port_id_counter_++;

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
        pid_t port_id = port_id_counter_++;

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
        pid_t port_id = port_id_counter_++;

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

        throw exception::port_undefined("trans: "+name()+": input port not connected by pid: "+ ::util::show (pid), ::util::show (pid));
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

        throw exception::port_undefined("trans: "+name()+": output port not connected by pid: "+ ::util::show (pid), ::util::show (pid));
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
          const std::string port_name (::util::show (port_id) );
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
        throw exception::port_undefined("trans: "+name()+": port not associated with:"+::util::show(pid), ::util::show(pid));
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
        throw exception::port_undefined("trans: "+name()+": during re_connect port not associated with:"+::util::show(pid_old), ::util::show(pid_old));
      }

      // TODO implement port accessor iterator
      const_iterator ports_begin() const
      {
        return ports_.begin();
      }
      const_iterator ports_end() const
      {
        return ports_.end();
      }

      const we::type::property::type & prop (void) const { return prop_; }

    private:
      std::string name_;
      data_type data_;
      bool internal_;
      condition::type condition_;

      outer_to_inner_t outer_to_inner_;
      inner_to_outer_t inner_to_outer_;
      port_map_t ports_;
      pid_t port_id_counter_;

      we::type::property::type prop_;

    private:
      template <typename P, typename E, typename T>
      friend std::ostream & operator<< ( std::ostream &
                                       , const transition_t<P,E,T> &
                                       );

      friend class boost::serialization::access;
      template <typename Archive>
      void save(Archive & ar, const unsigned int) const
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        ar & BOOST_SERIALIZATION_NVP(condition_.expression());
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);
      }

      template <typename Archive>
      void load(Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        std::string cond_expr;
        ar & BOOST_SERIALIZATION_NVP(cond_expr);
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
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    template <typename P, typename E, typename T>
    inline bool operator==(const transition_t<P,E,T> & a, const transition_t<P,E,T> & b)
    {
      return a.name() == b.name();
    }
    template <typename P, typename E, typename T>
    inline std::size_t hash_value(transition_t<P,E,T> const & t)
    {
      boost::hash<std::string> hasher;
      return hasher(t.name());
    }

    namespace detail
    {
      class transition_visitor_show : public boost::static_visitor<std::string>
      {
      public:
        std::string operator () (const expression_t & expr) const
        {
          return "{expr, " + ::util::show (expr) + "}";
        }

        std::string operator () (const module_call_t & mod_call) const
        {
          return "{mod, " + ::util::show (mod_call) + "}";
        }

        template <typename Place, typename Edge, typename Token>
        std::string operator () ( const petri_net::net< Place
                                , transition_t<Place, Edge, Token>
                                , Edge
                                , Token
                                > & net
                                ) const
        {
          return std::string("{net, ") + ::util::show(net) + "}";
        }
      };
    }

    template <typename P, typename E, typename T>
    inline std::ostream & operator<< ( std::ostream & s
                                     , const transition_t<P,E,T> & t
                                     )
    {
      typedef transition_t<P,E,T> trans_t;
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
      for ( typename trans_t::port_map_t::const_iterator p (t.ports_.begin())
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

    template<typename P, typename E, typename T>
    std::ostream & operator << ( std::ostream & s
                               , const petri_net::net<P, transition_t<P, E, T>, E, T> & n
                               )
    {
      typedef petri_net::net<P, transition_t<P, E, T>, E, T> pnet_t;

      for (typename pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
      {
        s << "[" << n.get_place (*p) << ":";

        typedef boost::unordered_map<T, size_t> token_cnt_t;
        token_cnt_t token;
        for (typename pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        {
          token[*tp]++;
        }

        for (typename token_cnt_t::const_iterator t (token.begin()); t != token.end(); ++t)
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

      for (typename pnet_t::transition_const_it t (n.transitions()); t.has_more(); ++t)
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

        template <typename P, typename E, typename T>
        kind operator ()
        (const petri_net::net<P, transition_t<P, E, T>, E, T> &) const
        {
          return subnet;
        }
      };
    }
  }
}

#endif
