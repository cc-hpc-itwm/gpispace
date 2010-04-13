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
#include <we/type/signature.hpp>
#include <we/mgmt/bits/pid_map_t.hpp>

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

      typedef petri_net::pid_t pid_t;
      typedef typename we::mgmt::detail::traits::pid_map_traits<pid_t>::type pid_map_t;
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
        : name ("unknown")
        , type (UNKNOWN)
        , port_id_counter_(0)
      {
        data.ptr = 0;
      }

      template <typename Type>
	  transition_t (const std::string & name_, Type const & typ, bool intern = false)
		: name(name_)
        , port_id_counter_(0)
	  {
        data.ptr = 0;
        assign(typ);
        flags.internal = intern;
      }

      transition_t (const transition_t &other)
        : name(other.name)
        , type(other.type)
        , flags(other.flags)
        , i_mapping(other.i_mapping)
        , o_mapping(other.o_mapping)
        , port_id_counter_(0)
      {
        data.ptr = 0;
        if (other.data.ptr)
        {
          switch (type)
          {
            case MOD_CALL:
              assign ( *other.data.mod );
              break;
            case NET:
              assign ( *other.data.net );
              break;
            case EXPRESSION:
              assign ( *other.data.expr );
              break;
            default:
              assert(false);
          }
        }
      }

      template <typename Choice>
      bool condition (Choice const &) const
      {
        return true;
      }

      void assign( net_type const & net )
      {
        clear();
        type = NET;
        data.net = new net_type (net);
      }

      void assign( expr_type const & expr )
      {
        clear();
        type = EXPRESSION;
        data.expr = new expr_type (expr);
      }

      void assign( mod_type const & mod )
      {
        clear();
        type = MOD_CALL;
        data.mod = new mod_type (mod);
      }

      bool is_net (void) const
      {
        return type == NET;
      }

      bool is_mod_call (void) const
      {
        return type == MOD_CALL;
      }

      bool is_expr (void) const
      {
        return type == EXPRESSION;
      }

      template <typename T>
      T * as(void)
      {
        return (T*)(data.ptr);
      }

      template <typename T>
      const T * as(void) const
      {
        return (const T*)(data.ptr);
      }

      inline
      void clear()
      {
        if (data.ptr)
        {
          switch (type)
          {
            case NET:
              delete data.net;
              break;
            case MOD_CALL:
              delete data.mod;
              break;
            case EXPRESSION:
              delete data.expr;
              break;
            default:
              break;
          }
        }
        type = UNKNOWN;
        data.ptr = 0;
      }

      transition_t & operator=(const transition_t & other)
      {
        if (this != &other)
        {
          name = other.name;
          flags = other.flags;
          i_mapping = other.i_mapping;
          o_mapping = other.o_mapping;
          clear();

          type = other.type;

          if (other.data.ptr)
          {
            switch (type) // new type
            {
              case MOD_CALL:
                assign ( *other.data.mod );
                break;
              case NET:
                assign ( *other.data.net );
                break;
              case EXPRESSION:
                assign ( *other.data.expr );
                break;
              default:
                break;
            }
          }
        }
        return *this;
      }

      ~transition_t ()
      {
        try
        {
          clear();
        }
        catch (...)
        {

        }
      }

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

      template <typename Pid>
      void connect_in(const Pid outer, const Pid inner)
      {
        i_mapping.insert (pid_map_t::value_type(outer, inner));
      }

      template <typename Pid>
      void connect_out(const Pid outer, const Pid inner)
      {
        o_mapping.insert (pid_map_t::value_type(outer, inner));
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

      pid_t input_port_by_name (const std::string & name) const
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

      pid_t output_port_by_name (const std::string & name) const
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

      // WORK: replace by boost::variant
      union
      {
        net_type *net;
        expr_type *expr;
        mod_type *mod;
        void *ptr;
      } data;

	  std::string name;
	  category_t type;
      flags_t flags;
      pid_map_t i_mapping;
      pid_map_t o_mapping;

      port_map_t ports_;
      pid_t port_id_counter_;

    private:
      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(name);
        ar & BOOST_SERIALIZATION_NVP(type);
        ar & BOOST_SERIALIZATION_NVP(flags);
        ar & BOOST_SERIALIZATION_NVP(i_mapping);
        ar & BOOST_SERIALIZATION_NVP(o_mapping);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
      }
	};

    template <typename P, typename E, typename T>
	inline bool operator==(const transition_t<P,E,T> & a, const transition_t<P,E,T> & b)
	{
	  return a.name == b.name;
	}
    template <typename P, typename E, typename T>
	inline std::size_t hash_value(transition_t<P,E,T> const & t)
	{
	  boost::hash<std::string> hasher;
	  return hasher(t.name);
	}
    template <typename P, typename E, typename T>
	inline std::ostream & operator<< (std::ostream & s, const transition_t<P,E,T> & t)
	{
      typedef transition_t<P,E,T> trans_t;
      s << t.name << "=";
      switch (t.type)
      {
        case trans_t::MOD_CALL:
          return s << "mod:" << *t.template as<typename trans_t::mod_type>();
        case trans_t::EXPRESSION:
          return s << "expr:" << *t.template as<typename trans_t::expr_type>();
        case trans_t::NET:
//          return s << "net:" << *t.template as<typename trans_t::net_type>();
          return s << "net-place-holder";
        default:
          return s << "unknown";
      }
	}
}}

#endif
