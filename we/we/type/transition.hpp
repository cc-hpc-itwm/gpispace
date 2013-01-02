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

#include <we/type/port.hpp>
#include <we/type/module_call.hpp>
#include <we/type/expression.hpp>
#include <we/type/condition.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/id.hpp>
#include <we/type/requirement.hpp>

#include <we/type/net.fwd.hpp>

#include <fhg/util/show.hpp>

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

#include <boost/format.hpp>

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

      struct already_connected : std::runtime_error
      {
        explicit already_connected (const std::string& msg)
          : std::runtime_error (msg)
        {}
        ~already_connected () throw ()
        {}
      };
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

      struct preparsed_condition
      {
        // should correspond!
        explicit preparsed_condition ( const std::string& _expr
                                     , const condition::type::parser_t& _parser
                                     )
          : expr(_expr)
          , parser(_parser)
        { }

        operator std::string const & () const
        {
          return expr;
        }

        operator condition::type::parser_t const & () const
        {
          return parser;
        }

        const std::string expr;
        const condition::type::parser_t parser;
      };
    }

    struct transition_t
    {
      typedef detail::preparsed_condition preparsed_cond_type;
      typedef transition_t this_type;

    private:
      typedef boost::variant< module_call_t
                            , expression_t
                            , boost::recursive_wrapper<petri_net::net>
                            > data_type;

      typedef std::pair< petri_net::place_id_type
                       , we::type::property::type
                       > pid_with_prop_t;

    public:
      typedef std::pair< petri_net::port_id_type
                       , we::type::property::type
                       > port_id_with_prop_t;
      typedef boost::unordered_map< petri_net::place_id_type
                                  , port_id_with_prop_t
                                  > outer_to_inner_t;
      typedef boost::unordered_map< petri_net::port_id_type
                                  , pid_with_prop_t
                                  > inner_to_outer_t;

      typedef boost::unordered_map<petri_net::port_id_type, port_t> port_map_t;
      typedef port_map_t::const_iterator const_iterator;
      typedef port_map_t::iterator port_iterator;
      typedef boost::unordered_set<std::string> port_names_t;

      typedef we::type::requirement_t requirement_t;
      typedef std::list<requirement_t> requirements_t;

      const static bool internal = true;
      const static bool external = false;

      transition_t ()
        : name_ ("<<transition unknown>>")
        , data_ (expression_t())
        , internal_ (true)
        , condition_( "true"
                    , boost::bind (&transition_t::name_of_place, this, _1)
                    )
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      {}

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , const std::string& _condition = "true"
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (detail::is_internal<Type>::value)
        , condition_( _condition
                    , boost::bind (&transition_t::name_of_place, this, _1)
                    )
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
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
                    , boost::bind (&transition_t::name_of_place, this, _1)
                    )
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      { }

      template <typename Type>
      transition_t ( const std::string & name
                   , Type const & typ
                   , const std::string& _condition
                   , bool intern
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_( _condition
                    , boost::bind (&transition_t::name_of_place, this, _1)
                    )
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
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
                    , boost::bind (&transition_t::name_of_place, this, _1)
                    )
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_(prop)
        , _requirements()
      { }

      transition_t (const transition_t &other)
        : name_(other.name_)
        , data_(other.data_)
        , internal_ (other.internal_)
        , condition_( other.condition_.expression()
                    , boost::bind (&transition_t::name_of_place, this, _1)
                    )
        , outer_to_inner_(other.outer_to_inner_)
        , inner_to_outer_(other.inner_to_outer_)
        , ports_(other.ports_)
        , port_id_counter_(other.port_id_counter_)
        , prop_ (other.prop_)
        , _requirements (other._requirements)
      { }

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
        return _requirements;
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
          port_id_counter_ = other.port_id_counter_;
          condition_ = condition::type
            ( other.condition_.expression()
            , boost::bind (&transition_t::name_of_place, this, _1)
            );
          prop_ = other.prop_;
          _requirements = other._requirements;
        }
        return *this;
      }

      ~transition_t () { }

      void connect_outer_to_inner ( const petri_net::place_id_type& pid
                                  , const petri_net::port_id_type& port
                                  , const we::type::property::type & prop
                                  )
      {
        if (outer_to_inner_.find (pid) != outer_to_inner_.end())
        {
          throw exception::already_connected
            ( (boost::format ("already connected: place %1% -> port %2%")
                             % pid % port
              ).str()
            );
        }
        else
        {
          outer_to_inner_.insert
            (outer_to_inner_t::value_type (pid, std::make_pair(port, prop)));
        }
      }

      void connect_inner_to_outer ( const petri_net::port_id_type& port
                                  , const petri_net::place_id_type& pid
                                  , const we::type::property::type & prop
                                  )
      {
        if (inner_to_outer_.find (port) != inner_to_outer_.end())
        {
          throw exception::already_connected
            ( (boost::format ("already connected: port %1% -> place %2%")
                             % port % pid
              ).str()
            );
        }
        else
        {
          inner_to_outer_.insert
            (inner_to_outer_t::value_type (port, std::make_pair(pid, prop)));
        }
      }

      void disconnect_outer_from_inner (const petri_net::place_id_type& pid)
      {
        outer_to_inner_t::iterator i (outer_to_inner_.find (pid));

        if (i == outer_to_inner_.end())
        {
          throw exception::not_connected<petri_net::place_id_type>("pid already disconnected", pid);
        }
        else
        {
          outer_to_inner_.erase (i);
        }
      }

      void disconnect_inner_from_outer (const petri_net::port_id_type& port)
      {
        inner_to_outer_t::iterator i (inner_to_outer_.find (port));

        if (i == inner_to_outer_.end())
        {
          throw exception::not_connected<petri_net::port_id_type> ("port already disconnected", port);
        }
        else
        {
          inner_to_outer_.erase (i);
        }
      }

      void re_connect_inner_to_outer ( const petri_net::port_id_type& port
                                     , const petri_net::place_id_type& pid
                                     , const we::type::property::type & prop
                                     )
      {
        inner_to_outer_.erase (port);

        connect_inner_to_outer (port, pid, prop);
      }

      void re_connect_outer_to_inner ( const petri_net::place_id_type& pid_old
                                     , const petri_net::place_id_type& pid_new
                                     , const petri_net::port_id_type& port
                                     , const we::type::property::type & prop
                                     )
      {
        outer_to_inner_.erase (pid_old);

        connect_outer_to_inner (pid_new, port, prop);
      }

      const petri_net::port_id_type& outer_to_inner (const petri_net::place_id_type& pid) const
      {
        try
        {
          return outer_to_inner_.at (pid).first;
        }
        catch (const std::out_of_range&)
        {
          throw exception::not_connected<petri_net::place_id_type> ("trans: " + name() + ": place not connected: " + fhg::util::show (pid), pid);
        }
      }

      const petri_net::place_id_type& inner_to_outer (const petri_net::port_id_type& port) const
      {
        try
        {
          return inner_to_outer_.at (port).first;
        }
        catch (const std::out_of_range &)
        {
          throw exception::not_connected<petri_net::port_id_type> ("trans: " + name() + ": port not connected: " + fhg::util::show (port), port);
        }
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

      void add_connection ( const petri_net::place_id_type& pid
                          , const std::string& name
                          , const we::type::property::type& prop
                          = we::type::property::type()
                          )
      {
        connect_outer_to_inner (pid, input_port_by_name (name), prop);
      }

      void add_connection ( const std::string& name
                          , const petri_net::place_id_type& pid
                          , const we::type::property::type& prop
                          = we::type::property::type()
                          )
      {
        connect_inner_to_outer (output_port_by_name (name), pid, prop);
      }

      // UNSAFE: does not check for already existing port, use with care
      petri_net::port_id_type UNSAFE_add_port (const port_t & port)
      {
        petri_net::port_id_type port_id = port_id_counter_++;

        ports_.insert (std::make_pair (port_id, port));

        return port_id;
      }

      void erase_port (const petri_net::port_id_type & port_id)
      {
        ports_.erase (port_id);
        inner_to_outer_.erase (port_id);
      }

      void add_port ( const std::string & name
                    , signature::type const & sig
                    , const we::type::PortDirection& direction
                    , const we::type::property::type & prop
                      = we::type::property::type()
                    )
      {
        switch (direction)
          {
          case PORT_IN: this->add_input_port (name, sig, prop); break;
          case PORT_OUT: this->add_output_port (name, sig, prop); break;
          case PORT_TUNNEL: this->add_tunnel (name, sig, prop); break;
          default: throw std::runtime_error ("STRANGE: unknown port direction");
          }
      }

      void add_port ( const std::string & name
                    , signature::type const & sig
                    , const we::type::PortDirection& direction
                    , const petri_net::place_id_type& pid
                    , const we::type::property::type & prop
                      = we::type::property::type()
                    )
      {
        switch (direction)
          {
          case PORT_IN: this->add_input_port (name, sig, pid, prop); break;
          case PORT_OUT: this->add_output_port (name, sig, pid, prop); break;
          case PORT_TUNNEL: this->add_tunnel (name, sig, pid, prop); break;
          default: throw std::runtime_error ("STRANGE: unknown port direction");
          }
      }

      void add_input_port ( const std::string & port_name
                          , const signature::type & signature
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
        const port_t port (port_name, PORT_IN, signature, prop);
        const petri_net::port_id_type port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
      }

      void add_input_port ( const std::string & port_name
                          , const signature::type & signature
                          , const petri_net::place_id_type& associated_place
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
        const port_t port (port_name, PORT_IN, signature, associated_place, prop);
        const petri_net::port_id_type port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
      }

      void add_output_port ( const std::string & port_name
                           , const signature::type & signature
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
        const port_t port (port_name, PORT_OUT, signature, prop);
        const petri_net::port_id_type port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
      }

      void add_tunnel ( const std::string & port_name
                      , const signature::type & signature
                      , const petri_net::place_id_type& associated_place
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
        const petri_net::port_id_type port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
      }

      void add_tunnel ( const std::string & port_name
                      , const signature::type & signature
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
        const petri_net::port_id_type port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
      }

      void add_output_port ( const std::string & port_name
                           , const signature::type & signature
                           , const petri_net::place_id_type& associated_place
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
        const port_t port (port_name, PORT_OUT, signature, associated_place, prop);
        const petri_net::port_id_type port_id (port_id_counter_++);

        ports_.insert (std::make_pair (port_id, port));
      }

      petri_net::port_id_type input_port_by_name (const std::string & port_name) const
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

      const petri_net::port_id_type& output_port_by_name (const std::string & port_name) const
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

      const port_id_with_prop_t& input_port_by_pid (const petri_net::place_id_type & pid) const
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

        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": input port not connected by pid: "+ fhg::util::show (pid), pid);
      }

      const petri_net::place_id_type& input_pid_by_port_id (const petri_net::port_id_type & port_id) const
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

        throw exception::not_connected<petri_net::port_id_type>("trans: "+name()+": pid not connected by port_id: "+ fhg::util::show (port_id), port_id);
      }

      port_id_with_prop_t output_port_by_pid (const petri_net::place_id_type & pid) const
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

        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": output port not connected by pid: "+ fhg::util::show (pid), pid);
      }

      const std::string& name_of_port (const petri_net::port_id_type& port) const
      {
        return get_port (port).name();
      }

      const std::string& name_of_place (const petri_net::place_id_type& pid)
      {
        return get_port (outer_to_inner (pid)).name();
      }

      const port_t& get_port (const petri_net::port_id_type& port_id) const
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

      port_t& get_port (const petri_net::port_id_type& port_id)
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

      const port_t & get_port_by_associated_pid (const petri_net::place_id_type & pid) const
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
        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": port not associated with:"+fhg::util::show(pid), pid);
      }

      // UNSAFE: does not check for multiple connections! Use with care!
      void UNSAFE_re_associate_port ( const petri_net::place_id_type & pid_old
                                    , const petri_net::place_id_type & pid_new
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
        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": during re_connect port not associated with:"+fhg::util::show(pid_old), pid_old);
      }

      // TODO implement port accessor iterator
      const_iterator ports_begin() const { return ports_.begin(); }
      const_iterator ports_end() const { return ports_.end(); }

      port_iterator ports_begin() { return ports_.begin(); }
      port_iterator ports_end() { return ports_.end(); }

      const we::type::property::type & prop (void) const { return prop_; }

      const port_map_t & ports () const { return ports_; }

      port_names_t port_names (const we::type::PortDirection & d) const
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
        _requirements.push_back (r);
      }

      void del_requirement ( requirement_t const & r )
      {
        _requirements.remove (r);
      }

    private:
      std::string name_;
      data_type data_;
      bool internal_;
      condition::type condition_;

      outer_to_inner_t outer_to_inner_;
      inner_to_outer_t inner_to_outer_;
      port_map_t ports_;
      petri_net::port_id_type port_id_counter_;

      we::type::property::type prop_;

      requirements_t _requirements;

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
        ar & BOOST_SERIALIZATION_NVP(_requirements);
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
                                     , boost::bind (&transition_t::name_of_place, this, _1)
                                     );
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);

        if (version > 0)
        {
          ar & BOOST_SERIALIZATION_NVP(_requirements);
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

        std::string operator () ( const petri_net::net& net
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

        kind operator () (const petri_net::net &) const
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

BOOST_CLASS_VERSION(we::type::transition_t, 1);

#endif
