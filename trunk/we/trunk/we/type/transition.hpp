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
#include <we/util/net_output_operator.hpp>
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
        explicit port_already_defined (const std::string & msg)
          : std::runtime_error (msg)
        {}
      };

      struct port_undefined : std::runtime_error
      {
        explicit port_undefined (const std::string & msg)
          : std::runtime_error (msg)
        {}
      };

      template <typename From>
      struct not_connected : std::runtime_error
      {
        typedef From from_type;

        explicit not_connected(const std::string & msg, const from_type from_)
          : std::runtime_error (msg)
          , from(from_)
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
        port_adder<Transition> & operator() (const std::string & name
                                           , SignatureType const & signature
                                           , Direction direction)
        {
          if (direction == PORT_IN)
            transition_.add_input_port (name, signature);
          if (direction == PORT_OUT)
            transition_.add_output_port (name, signature);
          else
            transition_.add_input_output_port (name, signature);
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

        template <typename Net>
        std::string operator () (const Net & net) const
        {
          we::util::remove_unused_variable_warning (net);
          return std::string("{net, ") + "placeholder" + "}";
        }
      };
    }

    template <typename Place, typename Edge, typename Token>
	struct transition_t
	{
	  enum Category
	  {
        UNKNOWN
      , MOD_CALL
	  , EXPRESSION
      , NET
	  };

      typedef module_call_t mod_type;
      typedef expression_t expr_type;
      typedef transition_t<Place, Edge, Token> this_type;
      typedef petri_net::net<Place, this_type, Edge, Token> net_type;
      typedef boost::variant<mod_type, expr_type, boost::recursive_wrapper<net_type> > data_type;

      typedef petri_net::pid_t pid_t;
      typedef pid_t port_id_t;

      typedef boost::unordered_map<pid_t, port_id_t> outer_to_inner_t;
      typedef boost::unordered_map<port_id_t, pid_t> inner_to_outer_t;
      typedef Category category_t;

      typedef signature::type signature_type;
      typedef port<signature_type> port_t;
      typedef boost::unordered_map<pid_t, port_t> port_map_t;

      struct flags_t
      {
        flags_t ()
          : internal(0)
        {}

        bool internal;

      private:
        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive & ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP(internal);
        }
      };

      transition_t ()
        : name_ ("unknown")
        , type_ (UNKNOWN)
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
	  transition_t (const std::string & name
                  , Type const & typ
                  , const std::string & condition = "true"
                  , bool intern = false)
		: name_ (name)
        , type_ (UNKNOWN) //type_of (typ))
        , data_ (typ)
        , condition_( condition
                    , boost::bind 
                      ( &detail::translate_place_to_port_name<this_type, pid_t>
                      , boost::ref(*this)
                      , _1
                      )
                    )
        , port_id_counter_(0)
	  {
        flags_.internal = intern;
      }

      transition_t (const transition_t &other)
        : name_(other.name_)
        , type_(other.type_)
        , data_(other.data_)
        , flags_(other.flags_)
        , condition_(other.condition_)
        , outer_to_inner_(other.outer_to_inner_)
        , inner_to_outer_(other.inner_to_outer_)
        , ports_(other.ports_)
        , port_id_counter_(0)
      { }

      template <typename Choices>
      bool condition (Choices const & choices) const
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

      category_t type (void) const
      {
        return type_;
      }

      bool is_internal (void) const
      {
        return flags_.internal;
      }

      bool is_net (void) const
      {
        return type_ == NET;
      }

      bool is_mod_call (void) const
      {
        return type_ == MOD_CALL;
      }

      bool is_expr (void) const
      {
        return type_ == EXPRESSION;
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
          flags_ = other.flags_;
          outer_to_inner_ = other.outer_to_inner_;
          inner_to_outer_ = other.inner_to_outer_;
          ports_ = other.ports_;
          type_ = other.type_;
          data_ = other.data_;
        }
        return *this;
      }

      ~transition_t () { }

	  template <typename Input, typename OutputDescription, typename OutputIterator>
	  void operator ()(Input const & input, OutputDescription const & desc, OutputIterator output) const
	  {
		for ( typename Input::const_iterator in (input.begin())
			; in != input.end()
			; ++in)
		{
		  for ( typename OutputDescription::const_iterator out (desc.begin())
			  ; out != desc.end()
			  ; ++out)
		  {
			std::cerr << "D: putting " << in->first << " to place " << out->first << std::endl;
			*output++ = std::make_pair(in->first, out->first);
		  }
		}
	  }

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
          throw exception::not_connected<Outer> ("place not connected: " + ::util::show(outer), outer);
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
          throw exception::not_connected<Inner> ("port not connected " + ::util::show(inner), inner);
        }
      }

      detail::connection_adder<this_type, pid_t, port_id_t> add_connections()
      {
        return detail::connection_adder<this_type, pid_t, port_id_t>(*this);
      }

      detail::port_adder<this_type> add_ports()
      {
        return detail::port_adder<this_type>(*this);
      }

      template <typename SignatureType>
      pid_t add_input_port (const std::string & name, const SignatureType & signature)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.direction() == PORT_IN) && p->second.name() == name)
          {
            throw exception::port_already_defined(name);
          }
        }
        port_t port (name, PORT_IN, signature);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      pid_t add_output_port (const std::string & name, const SignatureType & signature)
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.direction() == PORT_OUT) && p->second.name() == name)
          {
            throw exception::port_already_defined(name);
          }
        }
        port_t port (name, PORT_OUT, signature);
        pid_t port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));
        return port_id;
      }

      template <typename SignatureType>
      void add_input_output_port (const std::string & name, const SignatureType & signature)
      {
        try
        {
          input_port_by_name (name);
        }
        catch (const exception::port_undefined &)
        {
          try
          {
            output_port_by_name (name);
          }
          catch (const exception::port_undefined &)
          {
            add_input_port (name, signature);
            add_output_port (name, signature);
          }
        }
      }

      port_id_t input_port_by_name (const std::string & name) const
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.direction() == PORT_IN) && p->second.name() == name)
          {
            return p->first;
          }
        }
        throw exception::port_undefined(name);
      }

      port_id_t output_port_by_name (const std::string & name) const
      {
        for (port_map_t::const_iterator p = ports_.begin(); p != ports_.end(); ++p)
        {
          if ((p->second.direction() == PORT_OUT) && p->second.name() == name)
          {
            return p->first;
          }
        }
        throw exception::port_undefined(name);
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
          throw exception::port_undefined (::util::show (port_id));
        }
      }

    private:
	  std::string name_;
	  category_t type_;
      data_type data_;

      flags_t flags_;
      condition::type condition_;

      outer_to_inner_t outer_to_inner_;
      inner_to_outer_t inner_to_outer_;
      port_map_t ports_;
      pid_t port_id_counter_;

    private:
      // WORK TODO split up into save/load, since we need to initialize the
      // condition-translate function
      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(type_);
        ar & BOOST_SERIALIZATION_NVP(flags_);
        ar & BOOST_SERIALIZATION_NVP(condition_);
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(data_);
      }
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
    template <typename P, typename E, typename T>
	inline std::ostream & operator<< (std::ostream & s, const transition_t<P,E,T> & t)
	{
      static const detail::transition_visitor_show visitor;
      typedef transition_t<P,E,T> trans_t;
      s << "{";
      s << t.name() << ", ";
      s << boost::apply_visitor (visitor, t.data());
      s << ", {cond, " << t.condition() << "}";
      s << "}";
      return s;
	}
}}

#endif
