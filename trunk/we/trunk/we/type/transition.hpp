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

#include <boost/bind.hpp>
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
        port_adder<Transition> & operator() ( const std::string & name
                                            , SignatureType const & signature
                                            , Direction direction
                                            )
        {
          transition_.add_port ( name
                               , signature
                               , direction
                               );
          return *this;
        }

        template <typename SignatureType, typename Direction, typename PlaceId>
        port_adder<Transition> & operator() ( const std::string & name
                                            , SignatureType const & signature
                                            , Direction direction
                                            , const PlaceId associated_place
                                            )
        {
          transition_.add_port ( name
                               , signature
                               , direction
                               , associated_place
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

        connection_adder<Transition, from_type, to_type> & operator() (const from_type outer, const std::string & name)
        {
          transition_.connect_outer_to_inner (outer, transition_.input_port_by_name (name));
          return *this;
        }

        connection_adder<Transition, from_type, to_type> & operator() (const std::string & name, const from_type outer)
        {
          transition_.connect_inner_to_outer (transition_.output_port_by_name (name), outer);
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

      typedef boost::unordered_map<pid_t, port_id_t> outer_to_inner_t;
      typedef boost::unordered_map<port_id_t, pid_t> inner_to_outer_t;

      typedef signature::type signature_type;
      typedef port<signature_type> port_t;
      typedef boost::unordered_map<pid_t, port_t> port_map_t;
      typedef typename port_map_t::const_iterator const_iterator;

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
        }
        return *this;
      }

      ~transition_t () { }

      template <typename Outer, typename Inner>
      void connect_outer_to_inner(const Outer outer, const Inner inner)
      {
        if (outer_to_inner_.find (outer) != outer_to_inner_.end())
        {
          throw exception::already_connected<Outer, Inner>("already connected", outer, inner);
        }
        else
        {
          outer_to_inner_.insert (outer_to_inner_t::value_type (outer, inner));
        }
      }

      template <typename Inner, typename Outer>
      void connect_inner_to_outer(const Inner inner, const Outer outer)
      {
        if (inner_to_outer_.find (inner) != inner_to_outer_.end())
        {
          throw exception::already_connected<Inner, Outer>("already connected", inner, outer);
        }
        else
        {
          inner_to_outer_.insert (inner_to_outer_t::value_type (inner, outer));
        }
      }

      template <typename Outer>
      typename outer_to_inner_t::mapped_type outer_to_inner (Outer outer) const
      {
        try
        {
          return outer_to_inner_.at(outer);
        } catch (const std::out_of_range &)
        {
          throw exception::not_connected<Outer> ("trans: " + name() + ": not connected: " + ::util::show(outer), outer);
        }
      }

      template <typename Inner>
      typename inner_to_outer_t::mapped_type inner_to_outer (Inner inner) const
      {
        try
        {
          return inner_to_outer_.at(inner);
        } catch (const std::out_of_range &)
        {
          throw exception::not_connected<Inner> ("trans: " + name() + ": port not connected: " + ::util::show(inner), inner);
        }
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
                    )
      {
        if (direction == PORT_IN)
          this->add_input_port (port_name, signature);
        if (direction == PORT_OUT)
          this->add_output_port (port_name, signature);
        if (direction == PORT_READ)
          this->add_read_port (port_name, signature);
        else
          this->add_input_output_port (port_name, signature);
      }

      template <typename SignatureType, typename Direction, typename PlaceId>
      void add_port ( const std::string & name
                    , SignatureType const & signature
                    , const Direction direction
                    , const PlaceId associated_place
                    )
      {
        if (direction == PORT_IN)
          this->add_input_port ( name
                               , signature
                               , associated_place
                               );
        if (direction == PORT_OUT)
          this->add_output_port ( name
                                , signature
                                , associated_place
                                );
        if (direction == PORT_READ)
          this->add_read_port ( name
                              , signature
                              , associated_place
                              );
        else
          this->add_input_output_port ( name
                                      , signature
                                      , associated_place
                                      );
      }

      template <typename SignatureType>
      pid_t add_input_port (const std::string & port_name, const SignatureType & signature)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": input port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_IN, signature);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_input_port (const std::string & port_name, const SignatureType & signature, const PlaceId associated_place)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": input port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_IN, signature, associated_place);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      pid_t add_read_port (const std::string & port_name, const SignatureType & signature)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": read port " + port_name + " already defined: ", port_name);
          }
        }
        port_t port (port_name, PORT_READ, signature);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_read_port (const std::string & port_name, const SignatureType & signature, const PlaceId associated_place)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_input()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": read port " + port_name + " already defined: ", port_name);
          }
        }
        port_t port (port_name, PORT_READ, signature, associated_place);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      pid_t add_output_port (const std::string & port_name, const SignatureType & signature)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_output()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": output port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_OUT, signature);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType, typename PlaceId>
      pid_t add_output_port (const std::string & port_name, const SignatureType & signature, const PlaceId associated_place)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.is_output()) && p->second.name() == port_name)
          {
            throw exception::port_already_defined("trans: " + name() + ": output port " + port_name + " already defined", port_name);
          }
        }
        port_t port (port_name, PORT_OUT, signature, associated_place);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      void add_input_output_port (const std::string & port_name, const SignatureType & signature)
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
            add_input_port (port_name, signature);
            add_output_port (port_name, signature);
          }
        }
      }

      template <typename SignatureType, typename PlaceId>
      void add_input_output_port (const std::string & port_name, const SignatureType & signature, const PlaceId associated_place)
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
            add_input_port (port_name, signature, associated_place);
            add_output_port (port_name, signature, associated_place);
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

      // TODO implement port accessor iterator
      const_iterator ports_begin() const
      {
        return ports_.begin();
      }
      const_iterator ports_end() const
      {
        return ports_.end();
      }

    private:
      std::string name_;
      data_type data_;
      bool internal_;
      condition::type condition_;

      outer_to_inner_t outer_to_inner_;
      inner_to_outer_t inner_to_outer_;
      port_map_t ports_;
      pid_t port_id_counter_;

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

    // ********************************************************************** //
    // toDot

    namespace dot {
      typedef unsigned long id_type;
      typedef unsigned long level_type;

      static const std::string endl = "\\n";
      static const std::string arrow = " -> ";

      // ******************************************************************* //

      std::ostream & level (std::ostream & s, const level_type & level)
      {
        for (level_type i (0); i < level; ++i)
          {
            s << "  ";
          }

        return s;
      }

      // ******************************************************************* //

      inline std::string parens ( const std::string & s
                                , const std::string open = "("
                                , const std::string close = ")"
                                )
      {
        return open + s + close;
      }

      inline std::string brackets (const std::string & s)
      {
        return " " + parens (s, "[", "]");
      }

      inline std::string dquote (const std::string & s)
      {
        return parens (s, "\"", "\"");
      }

      // ******************************************************************* //

      inline std::string lines (const char & b, const std::string & s)
      {
        std::string l;
        bool was_b (false);

        for (std::string::const_iterator pos (s.begin()); pos != s.end(); ++pos)
          {
            if (*pos == b)
              {
                l += endl;

                was_b = true;
              }
            else
              {
                if (was_b)
                  {
                    was_b = (*pos != ' ');
                  }
                else
                  {
                    l += *pos;
                  }
              }
          }

        return l;
      }

      // ******************************************************************* //

      inline std::string quote (const char & c)
      {
        switch (c)
          {
          case '{': return "\\{";
          case '}': return "\\}";
          case '>': return "\\>";
          case '<': return "\\<";
          case '"': return "\\\"";
          default: return ::util::show (c);
          }
      }

      inline std::string quote (const std::string & s)
      {
        std::string q;

        for (std::string::const_iterator pos (s.begin()); pos != s.end(); ++pos)
          {
            q += quote (*pos);
          }

        return lines (';', q);
      }

      // ******************************************************************* //

      inline std::string keyval (const std::string & key, const std::string & val)
      {
        return key + " = " + dquote (val);
      }

      inline std::string name (const id_type & id, const std::string & _name)
      {
        std::ostringstream s;

        s << "n" << id << "_" << _name;

        return s.str();
      }

      inline std::string bgcolor (const std::string & color)
      {
        std::ostringstream s;

        s << keyval ("bgcolor", color) << std::endl;

        return s.str();
      }

      // ******************************************************************* //

      namespace shape
      {
        static const std::string condition = "record";
        static const std::string port = "hexagon";
        static const std::string expression = "none";
        static const std::string mod = "box";
        static const std::string place = "circle";
      }

      namespace color
      {
        static const std::string internal = "white";
        static const std::string external = "grey";
        static const std::string modcall = "yellow";
        static const std::string expression = "white";
        static const std::string node = "white";
      }

      // ******************************************************************* //

      inline std::string node ( const std::string & shape
                              , const std::string & label
                              )
      {
        std::ostringstream s;

        s << brackets ( keyval ("shape", shape) 
                      + ", " 
                      + keyval ("label", label)
                      + ", "
                      + keyval ("style", "filled")
                      + ", "
                      + keyval ("fillcolor", color::node)
                      )
          << std::endl
          ;

        return s.str();
      }

      template<typename Sig>
      inline std::string with_signature ( const std::string & name
                                        , const Sig & sig
                                        )
      {
        std::ostringstream s;

        s << name << endl << lines (',', ::util::show (sig));

        return s.str();
      };

      inline std::string association (void)
      {
        return brackets ( keyval ("style", "dotted")
                        + ", " 
                        + keyval ("dir","none")
                        );
      }

      // ******************************************************************* //

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

      // ******************************************************************* //
      // predicates about when to expand a transition

      template<typename T>
      class all
      {
      public:
        bool operator () (const T &) const { return true; }
      };

      // ******************************************************************* //

      template <typename P, typename E, typename T, typename Pred>
      inline std::string dot ( const transition_t<P,E,T> &
                             , id_type &
                             , const Pred & pred
                             , const level_type = 1
                             );


      template<typename Pred>
      class transition_visitor_dot : public boost::static_visitor<std::string>
      {
      private:
        id_type & id;
        const level_type l;
        const Pred & pred;

      public:
        transition_visitor_dot ( id_type & _id
                               , const level_type & _l
                               , const Pred & _pred
                               )
          : id (_id)
          , l (_l)
          , pred (_pred)
        {}

        // ----------------------------------------------------------------- //

        std::string operator () (const expression_t & expr) const
        {
          std::ostringstream s;

          level (s, l)
            << name (id, "expression")
            << node (shape::expression, quote (::util::show (expr)))
            ;

          return s.str();
        }

        // ----------------------------------------------------------------- //

        std::string operator () (const module_call_t & mod_call) const
        {
          std::ostringstream s;

          level (s, l)
            << name (id, "modcall")
            << node (shape::mod, ::util::show (mod_call))
            ;

          return s.str();
        }

        // ----------------------------------------------------------------- //

        template <typename P, typename E, typename T>
        std::string operator ()
        (const petri_net::net<P, transition_t<P, E, T>, E, T> & net) const
        {
          typedef transition_t<P, E, T> transition_t;
          typedef petri_net::net<P, transition_t, E, T> pnet_t;
          typedef typename pnet_t::place_const_it place_const_it;
          typedef typename pnet_t::transition_const_it transition_const_it;
          typedef typename pnet_t::token_place_it token_place_it;

          std::ostringstream s;

          const id_type id_net (id);

          level (s, l)
            << "subgraph cluster_net_" << id_net << " {"
            << std::endl;

          for (place_const_it p (net.places()); p.has_more(); ++p)
            {
              const P place (net.get_place (*p));

              std::ostringstream token;
              typedef boost::unordered_map<T, size_t> token_cnt_t;
              token_cnt_t token_cnt;
              for (token_place_it tp (net.get_token (*p)); tp.has_more(); ++tp)
                {
                  ++token_cnt[*tp];
                }

              for ( typename token_cnt_t::const_iterator tok (token_cnt.begin())
                  ; tok != token_cnt.end()
                  ; ++tok
                  )
                {
                  token << endl;

                  if (tok->second > 1)
                    {
                      token << tok->second << " x ";
                    }

                  token << quote (::util::show (tok->first));
                }

              level (s, l + 1)
                << name (id_net, "place_" + ::util::show (*p))
                << node ( shape::place
                        , with_signature ( place.get_name()
                                         , place.get_signature()
                                         )
                        + token.str()
                        )
                ;
            }

          for (transition_const_it t (net.transitions()); t.has_more(); ++t)
            {
              const transition_t trans (net.get_transition (*t));
              const id_type id_trans (++id);

              s << dot<P, E, T> (trans, id, pred, l + 1);

              for ( typename transition_t::inner_to_outer_t::const_iterator
                      connection (trans.inner_to_outer_begin())
                  ; connection != trans.inner_to_outer_end()
                  ; ++connection
                  )
                {
                  level (s, l + 1)
                    << name ( id_trans
                            , "port_" + ::util::show (connection->first)
                            )
                    << arrow
                    << name ( id_net
                            , "place_" + ::util::show (connection->second)
                            )
                    << std::endl
                    ;
                }

              for ( typename transition_t::outer_to_inner_t::const_iterator
                      connection (trans.outer_to_inner_begin())
                  ; connection != trans.outer_to_inner_end()
                  ; ++connection
                  )
                {
                  level (s, l + 1)
                    << name ( id_net
                            , "place_" + ::util::show (connection->first)
                            )
                    << arrow
                    << name ( id_trans
                            , "port_" + ::util::show (connection->second)
                            )
                    << std::endl
                    ;
                }
            }

          level (s, l + 1) << bgcolor (color::internal);

          level (s, l)
            << "} /* " << "cluster_net_" << id_net << " */"
            << std::endl;

          return s.str();
        }
      };

      // ******************************************************************* //

      template <typename P, typename E, typename T, typename Pred>
      inline std::string dot ( const transition_t<P,E,T> & t
                             , id_type & id
                             , const Pred & pred
                             , const level_type l = 1
                             )
      {
        typedef transition_t<P,E,T> trans_t;

        std::ostringstream s;

        const id_type id_trans (id);

        level (s, l)
          << "subgraph cluster_" << id_trans << " {"
          << std::endl;

        level (s, l + 1)
          << name (id_trans, "condition")
          << node ( shape::condition
                  , t.name() 
                  + "|" 
                  + quote (::util::show (t.condition()))
                  + "|"
                  + (t.is_internal() ? "internal" : "external")
                  )
          ;

        for ( typename trans_t::const_iterator p (t.ports_begin())
            ; p != t.ports_end()
            ; ++p
            )
          {
            level (s, l + 1)
              << name (id_trans, "port_" + ::util::show(p->first))
              << node ( shape::port
                      , with_signature ( p->second.name()
                                       , p->second.signature()
                                       )
                      )
              ;
          }

        if (pred (t))
          {
            s << boost::apply_visitor 
                 (transition_visitor_dot<Pred> (id, l + 1, pred), t.data());

            for ( typename trans_t::const_iterator p (t.ports_begin())
                ; p != t.ports_end()
                ; ++p
                )
              {
                if (p->second.has_associated_place())
                  {
                    level (s, l + 1)
                      << name (id_trans, "port_" + ::util::show (p->first))
                      << arrow
                      << name (id_trans
                              , "place_" 
                              + ::util::show (p->second.associated_place())
                              )
                      << association()
                      << std::endl
                      ;
                  }
              }
          }

        switch (boost::apply_visitor (content::visitor (), t.data()))
          {
          case content::modcall:
            level (s, l + 1) << bgcolor (color::modcall);
            break;
          case content::expression:
            level (s, l + 1) << bgcolor (color::expression);
            break;
          case content::subnet:
            level (s, l + 1) << 
              bgcolor (t.is_internal() ? color::internal : color::external)
              ;
            break;
          default: throw std::runtime_error
              ("STRANGE: unknown type of transition content");
          }

        level (s, l)
          << "} /* " << "cluster_" << id_trans << " == " << t.name() << " */"
          << std::endl;

        return s.str();
      }
    }
  }
}

#endif
