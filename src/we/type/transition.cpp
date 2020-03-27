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
    transition_t::transition_t()
      : name_ ("<<transition unknown>>")
      , data_ (expression_t())
      , condition_ (boost::none)
      , _ports_input()
      , _ports_output()
      , _ports_tunnel()
      , port_id_counter_ (0)
      , prop_()
      , _requirements()
      , _preferences()
      , _priority()
      , eureka_id_ (boost::none)
    {}

    boost::optional<const expression_t&> transition_t::expression() const
    {
      return fhg::util::boost::get_or_none<expression_t> (data());
    }
    boost::optional<const we::type::net_type&> transition_t::net() const
    {
      return fhg::util::boost::get_or_none<we::type::net_type> (data());
    }
    boost::optional<const module_call_t&> transition_t::module_call() const
    {
      return fhg::util::boost::get_or_none<module_call_t> (data());
    }

    boost::optional<expression_t> const& transition_t::condition() const
    {
      return condition_;
    }

    boost::optional<eureka_id_type> const& transition_t::eureka_id() const
    {
      return eureka_id_;
    }

    const std::string& transition_t::name() const
    {
      return name_;
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

    std::list<we::type::preference_t> const& transition_t::preferences() const
    {
      return _preferences;
    }

    we::port_id_type transition_t::add_port (port_t const& port)
    {
      we::port_id_type const port_id (port_id_counter_++);

      switch (port.direction())
      {
      case PORT_IN:
        _ports_input.emplace (port_id, port);
        break;

      case PORT_OUT:
        if (net() && !port.associated_place())
        {
          throw std::runtime_error
            ( ( boost::format ("Error when adding output port '%1%' to net '%2%':"
                              " Not associated with any place"
                              ) % port.name() % name()
              ).str()
            );
        }
        _ports_output.emplace (port_id, port);
        break;

      case PORT_TUNNEL:
        _ports_tunnel.emplace (port_id, port);
        break;
      }

      return port_id;
    }

    we::port_id_type transition_t::input_port_by_name
      (const std::string& port_name) const
    {
      for (port_map_t::value_type const& p : _ports_input)
      {
        if (p.second.name() == port_name)
        {
          return p.first;
        }
      }
      throw pnet::exception::port::unknown (name(), port_name);
    }

    const we::port_id_type& transition_t::output_port_by_name
      (const std::string& port_name) const
    {
      for (port_map_t::value_type const& p : _ports_output)
      {
        if (p.second.name() == port_name)
        {
          return p.first;
        }
      }
      throw pnet::exception::port::unknown (name(), port_name);
    }

    const we::type::property::type& transition_t::prop() const
    {
      return prop_;
    }

    const transition_t::port_map_t& transition_t::ports_input() const
    {
      return _ports_input;
    }
    const transition_t::port_map_t& transition_t::ports_output() const
    {
      return _ports_output;
    }
    const transition_t::port_map_t& transition_t::ports_tunnel() const
    {
      return _ports_tunnel;
    }

    void transition_t::add_requirement (we::type::requirement_t const& r)
    {
      _requirements.push_back (r);
    }

    we::priority_type transition_t::priority() const
    {
      return _priority;
    }
  }
}
