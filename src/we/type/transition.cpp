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
    boost::optional<const we::net&> transition_t::net() const
    {
      return fhg::util::boost::get_or_none<const we::net&> (data());
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

    we::port_id_type transition_t::add_port (port_t const& port)
    {
      we::port_id_type const port_id (port_id_counter_++);

      ports_.insert (std::make_pair (port_id, port));

      return port_id;
    }

    we::port_id_type transition_t::input_port_by_name
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

    const we::port_id_type& transition_t::output_port_by_name
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

    const we::type::property::type& transition_t::prop() const
    {
      return prop_;
    }

    const transition_t::port_map_t& transition_t::ports() const
    {
      return ports_;
    }

    void transition_t::add_requirement (we::type::requirement_t const& r)
    {
      _requirements.push_back (r);
    }
  }
}
