// {mirko.rahn}@itwm.fraunhofer.de

#include <we/type/transition.hpp>

#include <we/exception.hpp>
#include <we/type/net.hpp>

#include <fhg/util/boost/variant.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>

namespace we
{
  namespace type
  {
    boost::optional<const expression_t&> transition_t::expression() const
    {
      return fhg::util::boost::get_or_none<const expression_t&> (data());
    }
    boost::optional<const petri_net::net&> transition_t::net() const
    {
      return fhg::util::boost::get_or_none<const petri_net::net&> (data());
    }
    boost::optional<const module_call_t&> transition_t::module_call() const
    {
      return fhg::util::boost::get_or_none<const module_call_t&> (data());
    }

    const condition::type& transition_t::condition() const
    {
      return condition_;
    }

    const std::string& transition_t::name() const
    {
      return name_;
    }

    bool transition_t::is_internal() const
    {
      return internal_;
    }

    void transition_t::set_internal (bool x)
    {
      internal_ = x;
    }

    const transition_t::data_type& transition_t::data() const
    {
      return data_;
    }

    transition_t::data_type& transition_t::data()
    {
      return data_;
    }

    std::list<we::type::requirement_t> const& transition_t::requirements() const
    {
      return _requirements;
    }

    void transition_t::connect_outer_to_inner
      ( const petri_net::place_id_type& pid
      , const petri_net::port_id_type& port
      , const we::type::property::type& prop
      )
    {
      outer_to_inner_.insert
        (outer_to_inner_t::value_type (pid, std::make_pair (port, prop)));
    }

    void transition_t::connect_inner_to_outer
      ( const petri_net::port_id_type& port
      , const petri_net::place_id_type& pid
      , const we::type::property::type& prop
      )
    {
      inner_to_outer_.insert
        (inner_to_outer_t::value_type (port, std::make_pair (pid, prop)));
    }

    void transition_t::add_connection ( petri_net::place_id_type const& place_id
                                      , petri_net::port_id_type const& port_id
                                      , we::type::property::type const& property
                                      )
    {
      connect_outer_to_inner (place_id, port_id, property);
    }

    void transition_t::add_connection ( petri_net::port_id_type const& port_id
                                      , petri_net::place_id_type const& place_id
                                      , we::type::property::type const& property
                                      )
    {
      connect_inner_to_outer (port_id, place_id, property);
    }

    void transition_t::re_connect_inner_to_outer
      ( const petri_net::port_id_type& port_id
      , const petri_net::place_id_type& place_id
      , const we::type::property::type& property
      )
    {
      remove_connection_out (port_id);

      connect_inner_to_outer (port_id, place_id, property);
    }

    void transition_t::re_connect_outer_to_inner
      ( const petri_net::place_id_type& pid_old
      , const petri_net::place_id_type& pid_new
      , const petri_net::port_id_type& port
      , const we::type::property::type& prop
      )
    {
      remove_connection_in (pid_old);

      connect_outer_to_inner (pid_new, port, prop);
    }

    transition_t::inner_to_outer_t const& transition_t::inner_to_outer() const
    {
      return inner_to_outer_;
    }
    transition_t::outer_to_inner_t const& transition_t::outer_to_inner() const
    {
      return outer_to_inner_;
    }

    void transition_t::remove_connection_in
      (const petri_net::place_id_type& place_id)
    {
      outer_to_inner_.erase (place_id);
    }
    void transition_t::remove_connection_out
      (const petri_net::port_id_type& port_id)
    {
      inner_to_outer_.erase (port_id);
    }

    petri_net::port_id_type transition_t::add_port (port_t const& port)
    {
      petri_net::port_id_type const port_id (port_id_counter_++);

      ports_.insert (std::make_pair (port_id, port));

      return port_id;
    }

    void transition_t::erase_port (const petri_net::port_id_type& port_id)
    {
      ports_.erase (port_id);
      remove_connection_out (port_id);
    }

    petri_net::port_id_type transition_t::input_port_by_name
      (const std::string& port_name) const
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

    const petri_net::port_id_type& transition_t::output_port_by_name
      (const std::string& port_name) const
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

    const port_t& transition_t::get_port
      (const petri_net::port_id_type& port_id) const
    {
      try
      {
        return ports_.at (port_id);
      }
      catch (const std::out_of_range &)
      {
        throw pnet::exception::port::unknown
          (name(), boost::lexical_cast<std::string> (port_id));
      }
    }

    port_t& transition_t::get_port (const petri_net::port_id_type& port_id)
    {
      return ports_[port_id];
    }

    void transition_t::UNSAFE_re_associate_port
      ( const petri_net::place_id_type& pid_old
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

      throw std::runtime_error
        ( ( boost::format ("called UNSAFE_re_associate and it failed."
                          " trans '%1%', pid_old '%2%'"
                          )
          % name()
          % pid_old
          )
        . str()
        );
    }

    const we::type::property::type& transition_t::prop() const
    {
      return prop_;
    }

    const transition_t::port_map_t& transition_t::ports() const
    {
      return ports_;
    }

    transition_t::port_map_t& transition_t::ports()
    {
      return ports_;
    }

    void transition_t::add_requirement (we::type::requirement_t const& r)
    {
      _requirements.push_back (r);
    }

    // ********************************************************************* //

    boost::optional<transition_t::port_id_with_prop_t>
    output_port_by_pid ( transition_t const& trans
                       , const petri_net::place_id_type& pid
                       )
    {
      BOOST_FOREACH ( transition_t::inner_to_outer_t::value_type const& p
                    , trans.inner_to_outer()
                    )
      {
        if (p.second.first == pid)
        {
          return std::make_pair (p.first, p.second.second);
        }
      }

      return boost::none;
    }

    boost::optional<transition_t::port_id_with_prop_t const&>
    input_port_by_pid ( transition_t const& trans
                      , const petri_net::place_id_type& pid
                      )
    {
      transition_t::outer_to_inner_t::const_iterator const pos
        (trans.outer_to_inner().find (pid));

      if (pos != trans.outer_to_inner().end())
      {
        return pos->second;
      }

      return boost::none;
    }

    boost::unordered_set<std::string>
    port_names (transition_t const& trans, const we::type::PortDirection& d)
    {
      boost::unordered_set<std::string> names;

      BOOST_FOREACH ( we::type::port_t const& port
                    , trans.ports() | boost::adaptors::map_values
                    )
      {
        if (d == port.direction())
        {
          names.insert (port.name());
        }
      }

      return names;
    }

    boost::optional<const port_t&>
    get_port_by_associated_pid ( transition_t const& trans
                               , const petri_net::place_id_type& pid
                               )
    {
      BOOST_FOREACH ( we::type::port_t const& port
                    , trans.ports() | boost::adaptors::map_values
                    )
      {
        if (port.associated_place() == pid)
        {
          return port;
        }
      }

      return boost::none;
    }

    // ********************************************************************* //
  }
}
