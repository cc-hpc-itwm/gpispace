// {petry, rahn}@itwm.fhg.de

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

#include <we/exception.hpp>

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
#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>

namespace we { namespace type {
    namespace exception {
      template <typename From>
      struct not_connected : std::runtime_error
      {
        typedef From from_type;

        explicit not_connected(const std::string& msg, const from_type from_)
          : std::runtime_error (msg)
          , from(from_)
        {}

        ~not_connected () throw ()
        {}

        const from_type from;
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
    }

    struct transition_t
    {
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

      transition_t ()
        : name_ ("<<transition unknown>>")
        , data_ (expression_t())
        , internal_ (true)
        , condition_("true")
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      {}

      template <typename Type>
      transition_t ( const std::string& name
                   , Type const& typ
                   , const std::string& _condition = "true"
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (detail::is_internal<Type>::value)
        , condition_(_condition)
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      { }

      template <typename Type>
      transition_t ( const std::string& name
                   , Type const& typ
                   , condition::type const& _condition
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (detail::is_internal<Type>::value)
        , condition_ (_condition)
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      { }

      template <typename Type>
      transition_t ( const std::string& name
                   , Type const& typ
                   , const std::string& _condition
                   , bool intern
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_(_condition)
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      { }

      template <typename Type>
      transition_t ( const std::string& name
                   , Type const& typ
                   , condition::type const& _condition
                   , bool intern
                   , const we::type::property::type& prop
                   = we::type::property::type()
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_ (_condition)
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_(prop)
        , _requirements()
      { }

      boost::optional<const expression_t&> expression() const;

      const condition::type& condition() const
      {
        return condition_;
      }

      const std::string& name (void) const
      {
        return name_;
      }

      bool is_internal (void) const
      {
        return internal_;
      }

      void set_internal(bool x)
      {
        internal_ = x;
      }

      const data_type& data (void) const
      {
        return data_;
      }

      data_type& data (void)
      {
        return data_;
      }

      std::list<we::type::requirement_t> const& requirements (void) const
      {
        return _requirements;
      }

      void connect_outer_to_inner ( const petri_net::place_id_type& pid
                                  , const petri_net::port_id_type& port
                                  , const we::type::property::type& prop
                                  )
      {
        outer_to_inner_.insert
          (outer_to_inner_t::value_type (pid, std::make_pair(port, prop)));
      }

      void connect_inner_to_outer ( const petri_net::port_id_type& port
                                  , const petri_net::place_id_type& pid
                                  , const we::type::property::type& prop
                                  )
      {
        inner_to_outer_.insert
          (inner_to_outer_t::value_type (port, std::make_pair(pid, prop)));
      }

      void re_connect_inner_to_outer ( const petri_net::port_id_type& port
                                     , const petri_net::place_id_type& pid
                                     , const we::type::property::type& prop
                                     )
      {
        inner_to_outer_.erase (port);

        connect_inner_to_outer (port, pid, prop);
      }

      void re_connect_outer_to_inner ( const petri_net::place_id_type& pid_old
                                     , const petri_net::place_id_type& pid_new
                                     , const petri_net::port_id_type& port
                                     , const we::type::property::type& prop
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

      inner_to_outer_t const& inner_to_outer() const
      {
        return inner_to_outer_;
      }
      outer_to_inner_t const& outer_to_inner() const
      {
        return outer_to_inner_;
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

      void add_port (port_t const& port)
      {
        ports_.insert (std::make_pair (port_id_counter_++, port));
      }

      void erase_port (const petri_net::port_id_type& port_id)
      {
        ports_.erase (port_id);
        inner_to_outer_.erase (port_id);
      }

      petri_net::port_id_type input_port_by_name (const std::string& port_name) const
      {
        BOOST_FOREACH (port_map_t::value_type const& p, ports_)
        {
          if ((p.second.is_input()) && p.second.name() == port_name)
          {
            return p.first;
          }
        }
        throw pnet::exception::port::unknown (name(), port_name);
      }

      const petri_net::port_id_type& output_port_by_name (const std::string& port_name) const
      {
        BOOST_FOREACH (port_map_t::value_type const& p, ports_)
        {
          if ((p.second.is_output()) && p.second.name() == port_name)
          {
            return p.first;
          }
        }
        throw pnet::exception::port::unknown (name(), port_name);
      }

      const port_id_with_prop_t& input_port_by_pid (const petri_net::place_id_type& pid) const
      {
        BOOST_FOREACH (outer_to_inner_t::value_type const& p, outer_to_inner_)
        {
          if (p.first == pid)
          {
            return p.second;
          }
        }

        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": input port not connected by pid: "+ fhg::util::show (pid), pid);
      }

      const petri_net::place_id_type& input_pid_by_port_id (const petri_net::port_id_type& port_id) const
      {
        BOOST_FOREACH (outer_to_inner_t::value_type const& p, outer_to_inner_)
        {
          if (p.second.first == port_id)
          {
            return p.first;
          }
        }

        throw exception::not_connected<petri_net::port_id_type>("trans: "+name()+": pid not connected by port_id: "+ fhg::util::show (port_id), port_id);
      }

      port_id_with_prop_t output_port_by_pid (const petri_net::place_id_type& pid) const
      {
        BOOST_FOREACH (inner_to_outer_t::value_type const& p, inner_to_outer_)
        {
          if (p.second.first == pid)
          {
            return std::make_pair (p.first, p.second.second);
          }
        }

        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": output port not connected by pid: "+ fhg::util::show (pid), pid);
      }

      const std::string& name_of_port (const petri_net::port_id_type& port) const
      {
        return get_port (port).name();
      }

      const std::string& name_of_place (const petri_net::place_id_type& pid) const
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
          throw pnet::exception::port::unknown (name(), port_name);
        }
      }

      port_t& get_port (const petri_net::port_id_type& port_id)
      {
        return ports_[port_id];
      }

      const port_t& get_port_by_associated_pid (const petri_net::place_id_type& pid) const
      {
        BOOST_FOREACH ( we::type::port_t const& port
                      , ports_ | boost::adaptors::map_values
                      )
        {
          if (port.associated_place() == pid)
          {
            return port;
          }
        }

        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": port not associated with:"+fhg::util::show(pid), pid);
      }

      // UNSAFE: does not check for multiple connections! Use with care!
      void UNSAFE_re_associate_port ( const petri_net::place_id_type& pid_old
                                    , const petri_net::place_id_type& pid_new
                                    )
      {
        BOOST_FOREACH ( we::type::port_t& port
                      , ports_ | boost::adaptors::map_values
                      )
          {
            if (port.associated_place() == pid_old)
            {
              port.associated_place() = pid_new;

              return;
            }
          }

        throw exception::not_connected<petri_net::place_id_type>("trans: "+name()+": during re_connect port not associated with:"+fhg::util::show(pid_old), pid_old);
      }

      const we::type::property::type& prop (void) const { return prop_; }

      const port_map_t& ports() const { return ports_; }
      port_map_t& ports() { return ports_; }

      boost::unordered_set<std::string> port_names
        (const we::type::PortDirection& d) const
      {
        boost::unordered_set<std::string> names;

        BOOST_FOREACH ( we::type::port_t const& port
                      , ports_ | boost::adaptors::map_values
                      )
        {
          if (d == port.direction())
          {
            names.insert (port.name());
          }
        }

        return names;
      }

      void add_requirement (we::type::requirement_t const& r)
      {
        _requirements.push_back (r);
      }

      void del_requirement (we::type::requirement_t const& r)
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

      std::list<we::type::requirement_t> _requirements;

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void save(Archive& ar, const unsigned int) const
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        ar & BOOST_SERIALIZATION_NVP(condition_);
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
        ar & BOOST_SERIALIZATION_NVP(condition_);
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

    inline bool operator==(const transition_t& a, const transition_t& b)
    {
      return a.name() == b.name();
    }
    inline std::size_t hash_value(transition_t const& t)
    {
      boost::hash<std::string> hasher;
      return hasher(t.name());
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

      inline bool is_expression (const transition_t& t)
      {
        return boost::apply_visitor (visitor(), t.data()) == expression;
      }

      inline bool is_subnet (const transition_t& t)
      {
        return boost::apply_visitor (visitor(), t.data()) == subnet;
      }
    }
  }
}

BOOST_CLASS_VERSION(we::type::transition_t, 1)

#endif
